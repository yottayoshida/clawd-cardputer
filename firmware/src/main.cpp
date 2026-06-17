#include <M5Cardputer.h>
#include <cmath>
#include "clawd_sprites.h"

// ── walk config ──

static constexpr int WALK_RANGE = 25;
static constexpr int STEP_PX   = 2;
static constexpr int STEP_MS   = 150;
static constexpr int BOB_PX    = 2;
static constexpr int BREATH_AMP = 1;

// ── sprite buffer ──

static constexpr int SPRITE_W = clawd::TOTAL_W + 2;
static constexpr int SPRITE_H =
    clawd::TOTAL_H + (BOB_PX + BREATH_AMP) * 2 + 2;
static constexpr int SPRITE_BASE_X = (240 - SPRITE_W) / 2;
static constexpr int SPRITE_Y = (135 - SPRITE_H) / 2 - 6;
static int prevCanvasX = SPRITE_BASE_X;

static M5Canvas canvas(&M5Cardputer.Display);

// ── state ──

static clawd::Expression currentExpr = clawd::EXPR_IDLE;

static int  walkX      = 0;
static int  walkDir    = 1;
static int  walkStep   = 0;
static bool walkActive = true;

static unsigned long nextStepMs    = 0;
static unsigned long nextBlinkMs   = 3000;
static unsigned long reactionEndMs = 0;
static bool          inReaction    = false;

// ── sound state ──

static bool muted       = false;
static int  volumeLevel = 1;
static const uint8_t VOLUME_TABLE[] = {32, 64, 128};

static constexpr unsigned long DEBOUNCE_MS = 200;
static unsigned long lastKeyMs = 0;

// ── sleep cycle ──

enum SleepState : uint8_t {
  SLEEP_AWAKE = 0,
  SLEEP_SLEEPING,
};

static SleepState sleepState        = SLEEP_AWAKE;
static unsigned long lastActivityMs = 0;

static constexpr unsigned long SLEEPING_TIMEOUT_MS  = 60UL * 1000;  // 1 min

// ── mini-Clawds (subagent companions) ──

static constexpr int MAX_MINIS = 2;
static constexpr int MINI_QW = 2;
static constexpr int MINI_QH = 3;
static constexpr int MINI_W = clawd::GRID_COLS * MINI_QW;  // 32
static constexpr int MINI_H = clawd::GRID_ROWS * MINI_QH;  // 30

static constexpr int MINI_WALK_RANGE = 2;
static constexpr int MINI_STEP_PX   = 1;
static constexpr int MINI_STEP_MS   = 80;
static constexpr int MINI_BOB_PX    = 3;

static const uint16_t MINI_COLORS[MAX_MINIS] = {
    ((190 >> 3) << 11) | ((60 >> 2) << 5) | (20 >> 3),  // deep rust #BE3C14
    ((190 >> 3) << 11) | ((60 >> 2) << 5) | (20 >> 3),  // same
};

static const int MINI_CANVAS_X[MAX_MINIS] = {2, 198};
static constexpr int MINI_CANVAS_W = 40;
static constexpr int MINI_CANVAS_H = MINI_H + MINI_BOB_PX * 2 + 6;  // 48
static constexpr int MINI_HOME_OFFSET_X = 4;
static constexpr int MINI_HOME_OFFSET_Y = MINI_BOB_PX + 3;  // 9
static constexpr int MINI_CANVAS_Y = 52 - MINI_HOME_OFFSET_Y;  // 43

static M5Canvas miniCanvas[MAX_MINIS] = {
    M5Canvas(&M5Cardputer.Display),
    M5Canvas(&M5Cardputer.Display),
};

struct MiniState {
  int walkX;
  int walkDir;
  int walkStep;
  unsigned long nextStepMs;
};

static int miniCount = 0;
static MiniState minis[MAX_MINIS] = {};
static unsigned long lastMiniEventMs = 0;
static constexpr unsigned long MINI_TIMEOUT_MS = 8UL * 1000;

// perm_ask "???" overlay (non-blocking, expression unchanged)
static unsigned long permAskEndMs = 0;

// fanfare sound (non-blocking, driven from loop)
static int fanfarePhase = -1;
static unsigned long fanfareNextMs = 0;
struct FanfareNote { int freq; int durMs; int gapMs; };
static const FanfareNote FANFARE_NOTES[] = {
    {1000, 40, 20},   // ぴ
    {1400, 40, 20},   // ろ
    {1100, 40, 20},   // ぴ
    {1500, 40, 20},   // ろ
    {2000, 350, 0},   // ぴ〜〜ん
};
static constexpr int FANFARE_COUNT = 5;

// ── party mode ──

static bool partyActive              = false;
static unsigned long partyEndMs      = 0;
static int  partyNotePos             = 0;
static unsigned long partyNextNoteMs = 0;

static const uint16_t PARTY_COLORS[] = {
    0xF800, 0xFD20, 0xFFE0, 0x07E0, 0x001F, 0x780F, 0xF81F, 0x07FF,
};
static constexpr int NUM_PARTY_COLORS = 8;

static const int16_t PARTY_MELODY[] = {
    330,  392,  440,  523,    // buildup low
    523,  659,  784,  880,    // buildup high
    262,  0,    1047, 0,      // drop 1
    262,  0,    1319, 0,
    523,  659,  784,  1047,   // groove
    880,  784,  659,  523,
    262,  0,    1568, 0,      // drop 2
    262,  0,    1047, 1319,
};
static constexpr int PARTY_MELODY_LEN = 32;
static constexpr unsigned long PARTY_DURATION_MS = 10000;

// ── role system ──

static clawd::Role currentRole   = clawd::ROLE_NONE;
static unsigned long roleEndMs   = 0;
static bool roleEnabled          = true;
static constexpr unsigned long ROLE_TIMEOUT_MS = 10000;

// warp transition — full-screen tunnel rush + ascending sweep tone
static bool warpActive             = false;
static unsigned long warpStartMs   = 0;
static constexpr unsigned long WARP_FRAME_MS  = 30;
static constexpr int WARP_TOTAL_FRAMES        = 166;  // ~5 seconds
static constexpr int WARP_TUNNEL_FRAMES       = 140;
static constexpr int WARP_FLASH_FRAMES        = 153;

// ── drawing ──

static void drawClawdToCanvas(clawd::Expression expr,
                              int offsetX, int offsetY,
                              uint16_t color = clawd::COLOR_MAIN) {
  canvas.fillSprite(TFT_BLACK);

  const int ox = 1 + offsetX;
  const int oy = BOB_PX + BREATH_AMP + 1 + offsetY;

  const auto *shape = clawd::shapeFor(expr);

  for (int r = 0; r < clawd::GRID_ROWS; r++) {
    for (int c = 0; c < clawd::GRID_COLS; c++) {
      if (shape[r][c]) {
        canvas.fillRect(ox + c * clawd::QW, oy + r * clawd::QH,
                        clawd::QW, clawd::QH, color);
      }
    }
  }
}

static void pushCanvas(int canvasX) {
  if (canvasX != prevCanvasX) {
    if (canvasX > prevCanvasX) {
      M5Cardputer.Display.fillRect(prevCanvasX, SPRITE_Y,
                                   canvasX - prevCanvasX, SPRITE_H, TFT_BLACK);
    } else {
      M5Cardputer.Display.fillRect(canvasX + SPRITE_W, SPRITE_Y,
                                   (prevCanvasX + SPRITE_W) - (canvasX + SPRITE_W),
                                   SPRITE_H, TFT_BLACK);
    }
    prevCanvasX = canvasX;
  }
  canvas.pushSprite(canvasX, SPRITE_Y);
}

static void showStatus(clawd::Expression expr) {
  static const char *labels[] = {
      "idle", "blink", "happy", "surprised", "sleepy", "excited", "sleeping",
      "disappointed", "panicking", "confused"};
  static_assert(sizeof(labels) / sizeof(labels[0]) == clawd::EXPR_COUNT,
                "labels[] must match Expression enum count");
  int ty = 135 - 14;
  M5Cardputer.Display.fillRect(0, ty, 240, 14, TFT_BLACK);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(0x7BEF);
  M5Cardputer.Display.setCursor(4, ty + 3);

  const char *roleLbl = "";
  if (roleEnabled && currentRole != clawd::ROLE_NONE) {
    roleLbl = clawd::roleDefFor(currentRole).label;
  }

  if (muted) {
    M5Cardputer.Display.printf("%s  %s  MUTE  heap:%u",
                               labels[expr], roleLbl, ESP.getFreeHeap());
  } else {
    M5Cardputer.Display.printf("%s  %s  vol:%d  heap:%u",
                               labels[expr], roleLbl, volumeLevel, ESP.getFreeHeap());
  }
}

// ── forward declarations ──

static void triggerExpression(clawd::Expression expr,
                              unsigned long durationMs, int toneHz,
                              bool stopWalk = true);

// ── mini-Clawd drawing ──

static void drawMiniClawds(unsigned long now) {
  if (miniCount <= 0) return;
  if (sleepState == SLEEP_SLEEPING) return;

  if (now - lastMiniEventMs > MINI_TIMEOUT_MS) {
    for (int i = 0; i < MAX_MINIS; i++) {
      miniCanvas[i].fillSprite(TFT_BLACK);
      miniCanvas[i].pushSprite(MINI_CANVAS_X[i], MINI_CANVAS_Y);
    }
    miniCount = 0;
    return;
  }

  for (int i = 0; i < miniCount && i < MAX_MINIS; i++) {
    MiniState &m = minis[i];

    if (now >= m.nextStepMs) {
      if ((esp_random() % 4) == 0) m.walkDir = -m.walkDir;
      m.walkX += m.walkDir * MINI_STEP_PX;
      if (m.walkX > MINI_WALK_RANGE) { m.walkX = MINI_WALK_RANGE; m.walkDir = -1; }
      else if (m.walkX < -MINI_WALK_RANGE) { m.walkX = -MINI_WALK_RANGE; m.walkDir = 1; }
      m.walkStep++;
      m.nextStepMs = now + MINI_STEP_MS;
    }

    int bobY = (m.walkStep % 3 == 0) ? -MINI_BOB_PX
             : (m.walkStep % 3 == 1) ?  0
             :                          -MINI_BOB_PX / 2;

    float phase = (float)((now + (unsigned long)i * 1100) % 2000) / 2000.0f * 6.2831853f;
    int breathY = (int)(sinf(phase) * 1.5f);

    int ox = MINI_HOME_OFFSET_X + m.walkX;
    int oy = MINI_HOME_OFFSET_Y + bobY + breathY;

    miniCanvas[i].fillSprite(TFT_BLACK);

    const auto *shape = clawd::shapeFor(clawd::EXPR_HAPPY);
    for (int r = 0; r < clawd::GRID_ROWS; r++) {
      for (int c = 0; c < clawd::GRID_COLS; c++) {
        if (shape[r][c]) {
          miniCanvas[i].fillRect(ox + c * MINI_QW, oy + r * MINI_QH,
                                  MINI_QW, MINI_QH, MINI_COLORS[i]);
        }
      }
    }

    miniCanvas[i].pushSprite(MINI_CANVAS_X[i], MINI_CANVAS_Y);
  }
}

static void spawnMiniClawd() {
  if (miniCount >= MAX_MINIS) return;
  int idx = miniCount;
  minis[idx] = {0, (idx == 0) ? 1 : -1, 0, millis()};
  miniCount++;
  lastMiniEventMs = millis();
  fanfarePhase = 0;
  fanfareNextMs = millis();
  triggerExpression(clawd::EXPR_HAPPY, 800, 0);
  Serial.printf("[clawd] mini spawned (%d active)\n", miniCount);
}

static void despawnMiniClawd() {
  if (miniCount <= 0) return;
  lastMiniEventMs = millis();
  if (!muted) M5Cardputer.Speaker.tone(400, 80);
  Serial.printf("[clawd] mini stop received (%d active, linger until timeout)\n", miniCount);
}

static void fanfareTick(unsigned long now) {
  if (fanfarePhase < 0 || fanfarePhase >= FANFARE_COUNT) return;
  if (now < fanfareNextMs) return;
  const auto &n = FANFARE_NOTES[fanfarePhase];
  if (!muted) M5Cardputer.Speaker.tone(n.freq, n.durMs);
  fanfareNextMs = now + n.durMs + n.gapMs;
  fanfarePhase++;
  if (fanfarePhase >= FANFARE_COUNT) fanfarePhase = -1;
}

// ── animation ──

static void sleepTransition(unsigned long now) {
  if (partyActive || inReaction) return;

  unsigned long idle = now - lastActivityMs;
  SleepState prev = sleepState;

  sleepState = (idle >= SLEEPING_TIMEOUT_MS) ? SLEEP_SLEEPING : SLEEP_AWAKE;

  if (sleepState != prev && sleepState == SLEEP_SLEEPING) {
    currentExpr = clawd::EXPR_SLEEPING;
    walkActive = false;
    currentRole = clawd::ROLE_NONE;
    warpActive = false;
    for (int i = 0; i < miniCount && i < MAX_MINIS; i++) {
      miniCanvas[i].fillSprite(TFT_BLACK);
      miniCanvas[i].pushSprite(MINI_CANVAS_X[i], MINI_CANVAS_Y);
    }
    showStatus(currentExpr);
  }
}

static void wakeUp() {
  if (sleepState == SLEEP_SLEEPING) {
    triggerExpression(clawd::EXPR_SURPRISED, 800, 600);
  }
  sleepState = SLEEP_AWAKE;
}

static void recordActivity() {
  lastActivityMs = millis();
}

static void animTick(unsigned long now) {
  sleepTransition(now);

  // sleep guard: don't override sleep display with idle blink
  if (sleepState == SLEEP_SLEEPING && !inReaction) {
    float breathPhase = (float)(now % 3000) / 3000.0f * 6.2831853f;
    int breathY = (int)(sinf(breathPhase) * BREATH_AMP * 0.5f);
    drawClawdToCanvas(currentExpr, 0, breathY);
    pushCanvas(SPRITE_BASE_X);

    int zCount = (int)((now / 800) % 3) + 1;
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(0x4A49);
    M5Cardputer.Display.setCursor(155, 28);
    for (int i = 0; i < zCount; i++) M5Cardputer.Display.print("z");
    drawMiniClawds(now);
    return;
  }

  if (walkActive && now >= nextStepMs) {
    walkX += walkDir * STEP_PX;
    if (walkX >= WALK_RANGE || walkX <= -WALK_RANGE) walkDir = -walkDir;
    walkStep++;
    nextStepMs = now + STEP_MS;
  }

  int bobY = (walkActive && (walkStep & 1)) ? -BOB_PX : 0;

  float breathPhase = (float)(now % 3000) / 3000.0f * 6.2831853f;
  int breathY = (int)(sinf(breathPhase) * (float)BREATH_AMP);

  if (!inReaction && currentExpr == clawd::EXPR_IDLE) {
    if (now > nextBlinkMs) {
      currentExpr = clawd::EXPR_BLINK;
      reactionEndMs = now + 150;
      inReaction = true;
      nextBlinkMs = now + 3000 + (esp_random() % 5000);
    }
  }

  if (inReaction && now > reactionEndMs) {
    currentExpr = clawd::EXPR_IDLE;
    inReaction = false;
    walkActive = true;
  }

  // Role timeout (silent clear)
  if (currentRole != clawd::ROLE_NONE && now >= roleEndMs) {
    currentRole = clawd::ROLE_NONE;
    showStatus(currentExpr);
  }

  // Warp draws tunnel background, then falls through to normal Clawd drawing
  if (warpActive) {
    unsigned long elapsed = now - warpStartMs;
    int frame = (int)(elapsed / WARP_FRAME_MS);

    if (frame >= WARP_TOTAL_FRAMES) {
      warpActive = false;
      M5Cardputer.Display.fillScreen(TFT_BLACK);
      showStatus(currentExpr);
    } else {
      float progress = (float)frame / WARP_TOTAL_FRAMES;

      // Ascending sweep tone: 300Hz → 1500Hz
      if (!muted) {
        int freq = 300 + (int)(progress * 1200);
        M5Cardputer.Speaker.tone(freq, WARP_FRAME_MS + 15);
      }

      if (frame < WARP_TUNNEL_FRAMES) {
        // Tunnel phase: rings continuously spawn from center and expand outward
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        int cx = 120, cy = 67;
        constexpr float RING_CYCLE = 18.0f;
        for (int r = 0; r < 8; r++) {
          float raw = (float)frame - r * 2.5f;
          if (raw < 0.0f) continue;
          float age = fmodf(raw, RING_CYCLE);
          float sz = age * age * 3.0f;
          int hw = (int)sz;
          int hh = (int)(sz * 0.56f);
          if (hw > 130) continue;
          uint16_t col = PARTY_COLORS[(frame + r) % NUM_PARTY_COLORS];
          M5Cardputer.Display.drawRect(cx - hw, cy - hh, hw * 2, hh * 2, col);
          if (hw > 4)
            M5Cardputer.Display.drawRect(cx - hw + 1, cy - hh + 1,
                                         hw * 2 - 2, hh * 2 - 2, col);
        }
      } else if (frame < WARP_FLASH_FRAMES) {
        // Flash phase: rapid full-screen color bursts
        uint16_t col = PARTY_COLORS[(frame * 3) % NUM_PARTY_COLORS];
        M5Cardputer.Display.fillScreen(col);
      } else {
        // Landing: role color flash
        uint16_t roleCol = clawd::roleDefFor(currentRole).color;
        M5Cardputer.Display.fillScreen(roleCol);
      }
      // Fall through to draw Clawd on top of the tunnel background
    }
  }

  // Normal drawing: determine body color
  uint16_t drawColor = clawd::COLOR_MAIN;
  if (roleEnabled && currentRole != clawd::ROLE_NONE) {
    drawColor = clawd::roleDefFor(currentRole).color;
  }

  drawClawdToCanvas(currentExpr, 0, bobY + breathY, drawColor);
  pushCanvas(SPRITE_BASE_X + walkX);

  // Overlay priority: permAsk > role icon
  bool overlayDrawn = false;
  if (permAskEndMs > 0) {
    if (now < permAskEndMs) {
      int qCount = (int)((now / 300) % 3) + 1;
      M5Cardputer.Display.setTextSize(2);
      M5Cardputer.Display.setTextColor(TFT_RED);
      M5Cardputer.Display.setCursor(155, 28);
      for (int i = 0; i < qCount; i++) M5Cardputer.Display.print("?");
      overlayDrawn = true;
    } else {
      permAskEndMs = 0;
      walkActive = true;
    }
  }

  if (!overlayDrawn && roleEnabled && currentRole != clawd::ROLE_NONE) {
    const auto &rd = clawd::roleDefFor(currentRole);
    if (rd.icon[0] != '\0') {
      M5Cardputer.Display.setTextSize(2);
      M5Cardputer.Display.setTextColor(rd.color);
      M5Cardputer.Display.setCursor(155, 28);
      M5Cardputer.Display.print(rd.icon);
    }
  }

  drawMiniClawds(now);
}

// ── party mode ──

static void startParty() {
  partyActive = true;
  partyEndMs = millis() + PARTY_DURATION_MS;
  partyNotePos = 0;
  partyNextNoteMs = 0;
  inReaction = false;
  walkActive = false;
  currentRole = clawd::ROLE_NONE;
  warpActive = false;
  sleepState = SLEEP_AWAKE;
  recordActivity();
  Serial.println("[clawd] party mode!");
}

static void partyTick(unsigned long now) {
  if (now > partyEndMs) {
    partyActive = false;
    currentExpr = clawd::EXPR_IDLE;
    inReaction = false;
    walkActive = true;
    walkX = 0;
    prevCanvasX = SPRITE_BASE_X;
    recordActivity();
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    showStatus(clawd::EXPR_IDLE);
    return;
  }

  int bgIdx = (int)(now / 150) % NUM_PARTY_COLORS;
  uint16_t bgColor   = PARTY_COLORS[bgIdx];
  uint16_t charColor = PARTY_COLORS[(bgIdx + 3) % NUM_PARTY_COLORS];

  M5Cardputer.Display.fillScreen(bgColor);

  float phaseX = (float)(now % 400) / 400.0f * 6.2831853f;
  float phaseY = (float)(now % 250) / 250.0f * 6.2831853f;
  int wildX = (int)(sinf(phaseX) * 30.0f);
  int wildY = (int)(fabsf(sinf(phaseY)) * -12.0f);

  static const clawd::Expression PARTY_EXPRS[] = {
      clawd::EXPR_HAPPY, clawd::EXPR_EXCITED,
      clawd::EXPR_SURPRISED, clawd::EXPR_EXCITED,
  };
  clawd::Expression partyExpr = PARTY_EXPRS[(now / 200) % 4];

  canvas.fillSprite(bgColor);

  for (int i = 0; i < 20; i++) {
    canvas.fillRect(esp_random() % SPRITE_W, esp_random() % SPRITE_H,
                    3, 3, TFT_WHITE);
  }

  const int ox = 1 + wildX;
  const int oy = BOB_PX + BREATH_AMP + 1 + wildY;
  const auto *shape = clawd::shapeFor(partyExpr);
  for (int r = 0; r < clawd::GRID_ROWS; r++) {
    for (int c = 0; c < clawd::GRID_COLS; c++) {
      if (shape[r][c]) {
        canvas.fillRect(ox + c * clawd::QW, oy + r * clawd::QH,
                        clawd::QW, clawd::QH, charColor);
      }
    }
  }

  canvas.pushSprite(SPRITE_BASE_X, SPRITE_Y);
  prevCanvasX = SPRITE_BASE_X;

  int remaining = (int)((partyEndMs - now) / 1000) + 1;
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(TFT_WHITE);
  M5Cardputer.Display.setCursor(70, 135 - 22);
  M5Cardputer.Display.printf("PARTY! %d", remaining);

  if (!muted && now >= partyNextNoteMs) {
    int freq = PARTY_MELODY[partyNotePos % PARTY_MELODY_LEN];
    if (freq > 0) {
      M5Cardputer.Speaker.tone(freq, 100);
    }
    partyNotePos++;
    partyNextNoteMs = now + 120;
  }
}

// ── reactions ──

static void triggerExpression(clawd::Expression expr,
                              unsigned long durationMs, int toneHz,
                              bool stopWalk) {
  currentExpr = expr;
  reactionEndMs = millis() + durationMs;
  inReaction = true;
  if (stopWalk) walkActive = false;

  if (toneHz > 0 && !muted) {
    M5Cardputer.Speaker.tone(toneHz, 80);
  }
  showStatus(expr);
}

// ── serial events ──

struct EventDef {
  const char *name;
  clawd::Expression expr;
  int toneHz;
  unsigned long durationMs;
  bool stopWalk;
};

static const EventDef EVENT_TABLE[] = {
    {"bash",      clawd::EXPR_BLINK,     0,    300,  false},
    {"edit",      clawd::EXPR_BLINK,     0,    300,  false},
    {"read",      clawd::EXPR_BLINK,     0,    200,  false},
    {"glob",      clawd::EXPR_BLINK,     0,    200,  false},
    {"grep",      clawd::EXPR_BLINK,     0,    200,  false},
    {"write",     clawd::EXPR_HAPPY,     800,  600,  true},
    {"test",      clawd::EXPR_SURPRISED, 600,  600,  true},
    {"search",    clawd::EXPR_HAPPY,     700,  500,  true},
    {"commit",    clawd::EXPR_EXCITED,   1200, 1500, true},
    {"push",      clawd::EXPR_EXCITED,   1500, 2000, true},
    {"dirty",     clawd::EXPR_SLEEPY,    400,  500,  false},
    {"clean",     clawd::EXPR_HAPPY,     900,  800,  true},
    {"conflict",  clawd::EXPR_SURPRISED, 300,  1000, true},
    {"branch",    clawd::EXPR_EXCITED,   1000, 800,  true},
    {"pr_open",   clawd::EXPR_EXCITED,   1400, 1500, true},
    {"test_pass", clawd::EXPR_HAPPY,     1100, 800,  true},
    {"test_fail", clawd::EXPR_SURPRISED, 200,  1000, true},
    {"test_run",  clawd::EXPR_BLINK,     0,    400,  false},
    {"tool_fail", clawd::EXPR_DISAPPOINTED, 250,  1000, true},
    {"stop",      clawd::EXPR_SURPRISED,    900,  1200, true},
    {"stop_fail", clawd::EXPR_PANICKING,    200,  1000, true},
};

static unsigned long lastEventMs = 0;
static char serialBuf[32];
static int  serialPos = 0;

static void handleSerialEvent(const char *event) {
  unsigned long now = millis();
  if (now - lastEventMs < DEBOUNCE_MS) return;
  lastEventMs = now;
  recordActivity();

  if (sleepState == SLEEP_SLEEPING) {
    wakeUp();
    if (strncmp(event, "role_", 5) != 0 && strcmp(event, "mode_bigjob") != 0) {
      return;
    }
  }

  if (strcmp(event, "party") == 0) {
    startParty();
    return;
  }

  if (strcmp(event, "subagent_start") == 0) {
    spawnMiniClawd();
    return;
  }
  if (strcmp(event, "subagent_stop") == 0) {
    despawnMiniClawd();
    return;
  }

  if (strcmp(event, "perm_ask") == 0) {
    permAskEndMs = now + 3000;
    walkActive = false;
    walkX = 0;
    if (!muted) M5Cardputer.Speaker.tone(500, 150);
    Serial.printf("[clawd] event: %s\n", event);
    return;
  }

  // role_* events: set role color with warp transition
  if (strncmp(event, "role_", 5) == 0) {
    if (!roleEnabled) return;
    clawd::Role newRole = clawd::ROLE_NONE;
    const char *rn = event + 5;
    if      (strcmp(rn, "detective") == 0) newRole = clawd::ROLE_DETECTIVE;
    else if (strcmp(rn, "messenger") == 0) newRole = clawd::ROLE_MESSENGER;
    else if (strcmp(rn, "scribe")    == 0) newRole = clawd::ROLE_SCRIBE;
    else if (strcmp(rn, "artist")    == 0) newRole = clawd::ROLE_ARTIST;
    else if (strcmp(rn, "explorer")  == 0) newRole = clawd::ROLE_EXPLORER;
    else if (strcmp(rn, "worker")    == 0) newRole = clawd::ROLE_WORKER;
    else if (strcmp(rn, "nervous")   == 0) newRole = clawd::ROLE_NERVOUS;

    if (newRole != clawd::ROLE_NONE) {
      if (newRole != currentRole) {
        warpActive = true;
        warpStartMs = now;
      }
      currentRole = newRole;
      roleEndMs = now + ROLE_TIMEOUT_MS;
      showStatus(currentExpr);
      Serial.printf("[clawd] role: %s\n", event);
    }
    return;
  }

  // mode_bigjob: EXCITED reaction with fanfare (not a persistent role)
  if (strcmp(event, "mode_bigjob") == 0) {
    fanfarePhase = 0;
    fanfareNextMs = now;
    triggerExpression(clawd::EXPR_EXCITED, 2000, 0);
    Serial.printf("[clawd] event: %s\n", event);
    return;
  }

  for (const auto &e : EVENT_TABLE) {
    if (strcmp(event, e.name) == 0) {
      triggerExpression(e.expr, e.durationMs, e.toneHz, e.stopWalk);
      Serial.printf("[clawd] event: %s\n", event);
      return;
    }
  }
  triggerExpression(clawd::EXPR_BLINK, 200, 0, false);
  Serial.printf("[clawd] event: %s (unknown)\n", event);
}

// ── key handling ──

static void handleKey() {
  unsigned long now = millis();
  if (now - lastKeyMs < DEBOUNCE_MS) return;
  lastKeyMs = now;
  recordActivity();
  if (sleepState == SLEEP_SLEEPING) {
    wakeUp();
    return;
  }

  auto &state = M5Cardputer.Keyboard.keysState();

  if (state.enter) {
    triggerExpression(clawd::EXPR_EXCITED, 1500, 800);
    if (!muted) {
      delay(90);
      M5Cardputer.Speaker.tone(1200, 80);
    }
    Serial.println("[clawd] pet!");
    return;
  }

  if (!state.word.empty()) {
    char c = state.word[0];

    if (c == 'p' || c == 'P') {
      startParty();
      return;
    }

    if (c == 'm' || c == 'M') {
      muted = !muted;
      if (!muted) M5Cardputer.Speaker.tone(1000, 30);
      showStatus(currentExpr);
      Serial.printf("[clawd] mute=%d\n", muted);
      return;
    }

    if (c == 'r' || c == 'R') {
      roleEnabled = !roleEnabled;
      if (!roleEnabled) {
        currentRole = clawd::ROLE_NONE;
        warpActive = false;
      }
      showStatus(currentExpr);
      Serial.printf("[clawd] role display=%d\n", roleEnabled);
      return;
    }

    if (c >= '1' && c <= '3') {
      volumeLevel = c - '0';
      M5Cardputer.Speaker.setVolume(VOLUME_TABLE[volumeLevel - 1]);
      if (!muted) M5Cardputer.Speaker.tone(800 + volumeLevel * 200, 40);
      showStatus(currentExpr);
      Serial.printf("[clawd] vol=%d\n", volumeLevel);
      return;
    }
  }

  static const clawd::Expression REACT_EXPRS[] = {
      clawd::EXPR_HAPPY, clawd::EXPR_SURPRISED,
      clawd::EXPR_SLEEPY, clawd::EXPR_EXCITED,
  };
  static const int REACT_TONES[] = {1000, 800, 500, 1200};
  int idx = esp_random() % 4;
  triggerExpression(REACT_EXPRS[idx], 800, REACT_TONES[idx]);
  Serial.printf("[clawd] key → expr=%d\n", currentExpr);
}

// ── setup / loop ──

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  Serial.begin(115200);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(TFT_BLACK);

  canvas.setColorDepth(16);
  canvas.createSprite(SPRITE_W, SPRITE_H);

  for (int i = 0; i < MAX_MINIS; i++) {
    miniCanvas[i].setColorDepth(16);
    miniCanvas[i].createSprite(MINI_CANVAS_W, MINI_CANVAS_H);
  }

  M5Cardputer.Speaker.setVolume(VOLUME_TABLE[0]);
  M5Cardputer.Speaker.tone(880, 100);
  delay(120);
  M5Cardputer.Speaker.tone(1100, 80);

  showStatus(clawd::EXPR_IDLE);
  lastActivityMs = millis();

  Serial.printf("[clawd] sprite %dx%d (%dB), heap: %u\n",
                SPRITE_W, SPRITE_H, SPRITE_W * SPRITE_H * 2,
                ESP.getFreeHeap());
}

void loop() {
  M5Cardputer.update();

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialPos > 0) {
        serialBuf[serialPos] = '\0';
        handleSerialEvent(serialBuf);
        serialPos = 0;
      }
    } else if (serialPos < (int)sizeof(serialBuf) - 1) {
      serialBuf[serialPos++] = c;
    }
  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    handleKey();
  }

  unsigned long now = millis();
  fanfareTick(now);
  if (partyActive) {
    partyTick(now);
  } else {
    animTick(now);
  }
  delay(16);
}
