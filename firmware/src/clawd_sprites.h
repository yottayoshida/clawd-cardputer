#pragma once
#include <stdint.h>

namespace clawd {

constexpr uint16_t COLOR_MAIN =
    ((217 >> 3) << 11) | ((119 >> 2) << 5) | (87 >> 3);  // #D97757

// 16×10 quadrant grid extracted from terminal screenshot pixel data.
// Each cell = QW×QH pixels on the LCD.
constexpr int GRID_COLS = 16;
constexpr int GRID_ROWS = 10;
constexpr int QW = 6;
constexpr int QH = 7;
constexpr int TOTAL_W = GRID_COLS * QW;  // 96
constexpr int TOTAL_H = GRID_ROWS * QH;  // 70

// IDLE shape — row-major, 1=filled 0=empty
constexpr uint8_t SHAPE_IDLE[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // stalks
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},  // head + eyes (tall rect holes)
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // head + ears (full width)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // body
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},  // legs
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// BLINK — eyes closed (fill in the eye holes)
constexpr uint8_t SHAPE_BLINK[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // eyes filled
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// HAPPY — legs spread outward
constexpr uint8_t SHAPE_HAPPY[GRID_ROWS][GRID_COLS] = {
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},  // legs wider
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
};

// SURPRISED — wider stalks + bigger eyes
constexpr uint8_t SHAPE_SURPRISED[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // stalks full width
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},  // wider eyes (2 quad cols)
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// SLEEPY — droopy stalks + thin eyes
constexpr uint8_t SHAPE_SLEEPY[GRID_ROWS][GRID_COLS] = {
    {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},  // shorter stalks
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // eyes closed (thin line)
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},  // only bottom row has eye holes
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// EXCITED — stalks + ears both full, legs wide
constexpr uint8_t SHAPE_EXCITED[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // stalks full
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},  // legs wider
    {0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
};

// SLEEPING — squished flat, melted down
constexpr uint8_t SHAPE_SLEEPING[GRID_ROWS][GRID_COLS] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},  // small bump
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // spreading
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},  // wider
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // full width flat
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},  // tiny feet poking out
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// DISAPPOINTED — droopy stalks, eyes shifted down, limp legs
constexpr uint8_t SHAPE_DISAPPOINTED[GRID_ROWS][GRID_COLS] = {
    {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},  // short droopy stalks
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},  // eyes closed on top
    {0,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0},  // eyes only open at bottom
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},  // legs droop inward
    {0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
};

// PANICKING — asymmetric stalks, big eyes, legs splayed wide
constexpr uint8_t SHAPE_PANICKING[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // stalks max width
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // left stalk slightly shorter
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},  // big eyes (2-col holes)
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},  // legs max splay
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
};

// CONFUSED — same shape as SURPRISED, with "???" overlay drawn in code
constexpr uint8_t SHAPE_CONFUSED[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},
    {0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0},
};

// ── Role system (orthogonal to Expression) ──
// Okabe-Ito color-blind-safe palette, RGB565

enum Role : uint8_t {
  ROLE_NONE = 0,
  ROLE_DETECTIVE,
  ROLE_MESSENGER,
  ROLE_SCRIBE,
  ROLE_ARTIST,
  ROLE_EXPLORER,
  ROLE_WORKER,
  ROLE_NERVOUS,
  ROLE_COUNT,
};

struct RoleDef {
  uint16_t color;
  char icon[3];
  char label[12];
};

constexpr uint16_t CLR_DETECTIVE = ((0x56>>3)<<11)|((0xB4>>2)<<5)|(0xE9>>3); // #56B4E9
constexpr uint16_t CLR_MESSENGER = ((0xF0>>3)<<11)|((0xE4>>2)<<5)|(0x42>>3); // #F0E442
constexpr uint16_t CLR_SCRIBE    = ((0x00>>3)<<11)|((0x9E>>2)<<5)|(0x73>>3); // #009E73
constexpr uint16_t CLR_ARTIST    = ((0xCC>>3)<<11)|((0x79>>2)<<5)|(0xA7>>3); // #CC79A7
constexpr uint16_t CLR_EXPLORER  = ((0xE6>>3)<<11)|((0x9F>>2)<<5)|(0x00>>3); // #E69F00
constexpr uint16_t CLR_WORKER    = ((0xD5>>3)<<11)|((0x5E>>2)<<5)|(0x00>>3); // #D55E00
constexpr uint16_t CLR_NERVOUS   = ((0x00>>3)<<11)|((0x72>>2)<<5)|(0xB2>>3); // #0072B2

constexpr RoleDef ROLE_TABLE[] = {
    {COLOR_MAIN,    "",   ""},          // ROLE_NONE
    {CLR_DETECTIVE, "?!", "detective"}, // ROLE_DETECTIVE
    {CLR_MESSENGER, ">>", "messenger"},
    {CLR_SCRIBE,    "//", "scribe"},
    {CLR_ARTIST,    "<>", "artist"},
    {CLR_EXPLORER,  "[]", "explorer"},
    {CLR_WORKER,    "{}",  "worker"},
    {CLR_NERVOUS,   "!!", "nervous"},
};
static_assert(sizeof(ROLE_TABLE) / sizeof(ROLE_TABLE[0]) == ROLE_COUNT,
              "ROLE_TABLE size must match ROLE_COUNT");

inline const RoleDef& roleDefFor(Role r) {
  return ROLE_TABLE[(r < ROLE_COUNT) ? r : 0];
}

enum Expression : uint8_t {
  EXPR_IDLE = 0,
  EXPR_BLINK,
  EXPR_HAPPY,
  EXPR_SURPRISED,
  EXPR_SLEEPY,
  EXPR_EXCITED,
  EXPR_SLEEPING,
  EXPR_DISAPPOINTED,
  EXPR_PANICKING,
  EXPR_CONFUSED,
  EXPR_COUNT,
};

inline const uint8_t (*shapeFor(Expression e))[GRID_COLS] {
  switch (e) {
    case EXPR_BLINK:        return SHAPE_BLINK;
    case EXPR_HAPPY:        return SHAPE_HAPPY;
    case EXPR_SURPRISED:    return SHAPE_SURPRISED;
    case EXPR_SLEEPY:       return SHAPE_SLEEPY;
    case EXPR_EXCITED:      return SHAPE_EXCITED;
    case EXPR_SLEEPING:     return SHAPE_SLEEPING;
    case EXPR_DISAPPOINTED: return SHAPE_DISAPPOINTED;
    case EXPR_PANICKING:    return SHAPE_PANICKING;
    case EXPR_CONFUSED:     return SHAPE_CONFUSED;
    default:                return SHAPE_IDLE;
  }
}

}  // namespace clawd
