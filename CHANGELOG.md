# Changelog

## [0.2.1] — 2026-06-13

**Summary**: Renamed project from ADVcchi to clawd-cardputer.

### Changed
- Repository renamed from `advcchi` to `clawd-cardputer`
- Hook script renamed from `advcchi-hook.sh` to `clawd-hook.sh`
- Environment variable renamed from `ADVCCHI_PORT` to `CLAWD_PORT`

## [0.2.0] — 2026-06-13

**Summary**: Clawd now reacts to git operations and test results, and falls asleep when idle.

### Added
- Git event detection: hook parses `tool_input.command` to classify `dirty`, `clean`, `conflict`, `branch`, and `pr_open` events (#6)
- Test result detection: hook detects test runners (pytest, cargo test, npm test, and wrappers) and sends `test_pass`, `test_fail`, or `test_run` based on stdout patterns (#7)
- Sleep cycle: after 1 minute of inactivity, Clawd squishes flat with animated "zzz" (z → zz → zzz). Any event or key press wakes it up with a surprised reaction (#8)
- New SLEEPING sprite (7th expression): flattened "melted" shape
- `static_assert` to catch labels[]/enum mismatch at compile time

### Changed
- Hook script rewritten for command parsing (30 → 74 lines), with `sys.argv` for serial communication (injection prevention) and case-statement allowlist guard
- EVENT_TABLE expanded from 10 to 18 entries

## [0.1.0] — 2026-06-13

Initial release. Clawd walks, bobs, breathes, blinks, and reacts to Claude Code tool invocations via PostToolUse hook over USB serial. Party mode, keyboard shortcuts, 6 expressions, sound effects.
