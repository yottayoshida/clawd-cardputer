# ADVcchi

A developer tamagotchi running on M5Stack Cardputer ADV. It reacts to your Claude Code activity via a PostToolUse hook — every `bash`, `edit`, `write`, and `commit` feeds the creature on your desk.

## What it does

A pixel-art character ("Clawd") lives on the Cardputer's 240x135 display. It walks, bobs, breathes, and blinks on its own. When Claude Code invokes a tool, the hook script sends the event over USB serial, and Clawd reacts:

| Event | Expression | Sound |
|-------|-----------|-------|
| `bash` `read` `glob` `grep` | Blink | — |
| `edit` | Blink | — |
| `write` `search` | Happy | tone |
| `test` | Surprised | tone |
| `commit` `push` | Excited | tone |
| `/develop` | **Party mode** | melody |

Keyboard shortcuts on the Cardputer itself:

| Key | Action |
|-----|--------|
| `p` | Party mode |
| `m` | Mute / unmute |
| `1` `2` `3` | Volume level |
| `Enter` | Pet (excited + sound) |
| Any other | Random reaction |

## Hardware

- [M5Stack Cardputer ADV](https://shop.m5stack.com/products/m5stack-cardputer-adv) (ESP32-S3, 240x135 display, keyboard, speaker)
- USB-C cable (for flashing and serial communication)

## Setup

### 1. Flash the firmware

Requires [PlatformIO](https://platformio.org/).

```bash
cd firmware
pio run -t upload
```

> If you're behind a corporate proxy with custom CA certificates:
> ```bash
> REQUESTS_CA_BUNDLE=$HOME/.your-ca.pem pio run -t upload
> ```

### 2. Install the Claude Code hook

```bash
./hook/install-hook.sh
```

This adds a `PostToolUse` entry to `~/.claude/settings.json`. The script is safe to re-run (idempotent) and creates a backup before modifying settings.

Options:
- `--dry-run` — show what would change without modifying anything
- `--uninstall` — remove the hook entry

### 3. Restart Claude Code

The hook takes effect on the next Claude Code session.

## Requirements

- Python 3 with [pyserial](https://pypi.org/project/pyserial/) (`pip install pyserial`)
- [jq](https://jqlang.github.io/jq/) (for JSON parsing in the hook script)
- macOS (tested on macOS 15 Sequoia; Linux should work but is untested)

## Project structure

```
firmware/
  platformio.ini        # PlatformIO build config
  src/main.cpp          # Application logic (animations, events, serial, party mode)
  src/clawd_sprites.h   # Pixel-art character data (16x10 grid, 6 expressions)
  assets/source/        # Design notes (not redistributed)
hook/
  advcchi-hook.sh       # PostToolUse hook script
  install-hook.sh       # Hook installer for settings.json
```

## How the hook works

1. Claude Code fires `PostToolUse` after every tool invocation
2. `advcchi-hook.sh` reads the JSON event from stdin, extracts `tool_name`
3. The tool name is sent over USB serial (115200 baud) via pyserial
4. The Cardputer's firmware matches it against the event table and triggers the appropriate reaction

The hook is non-blocking — if the Cardputer is disconnected, it silently does nothing.

## License

[MIT](LICENSE)
