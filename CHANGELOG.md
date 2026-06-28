# Changelog

## [0.5.1] — 2026-06-28

**Summary**: Firmware reliability fixes for always-on desk pet usage.

### Fixed
- Event debounce now only suppresses identical events within 200ms; high-priority events (perm_ask, role_*, subagent_*, stop_fail, party) always pass through immediately (#26)
- All 14 timer comparisons replaced with rollover-safe helpers (`timeReached`/`elapsedGe`) to prevent timer lockup after ~49.7 days of continuous operation (#29)
- Mini-Clawds now track active and lingering states separately with per-slot timeout; stopped minis dim to half brightness and free their slot immediately for new spawns (#28)
- Overlay region (zzz, "???", role icon) is cleared each frame to prevent ghost text artifacts on state transitions (#27)
- Warp transition uses transparent sprite pushing so Clawd renders without a black rectangle over the tunnel background (#30)

## [0.5.0] — 2026-06-18

**Summary**: Role system — Clawd's body color changes based on MCP tool usage and prompt keywords.

### Added
- Role system: 7 distinct roles (Detective, Messenger, Scribe, Artist, Explorer, Worker, Nervous) with Okabe-Ito color-blind-safe palette (#9, #3)
- MCP tool detection: hook extracts server name from `mcp__*` tool calls and maps to roles (GitHub→Detective, Slack→Messenger, Notion→Scribe, Figma→Artist, Drive→Explorer)
- UserPromptSubmit keyword classification: word-boundary matching maps prompt keywords to roles (review→Detective, write→Scribe, build→Worker, test→Nervous, etc.)
- Warp transition effect: ~5-second hyperspace tunnel (expanding rings from center + ascending sweep tone 300→1500Hz) on role change, with Clawd visible throughout, landing on the target role color
- Role icon overlay: 2-character text icon displayed at top-right (e.g. "?!" for Detective, "//" for Scribe)
- Role label in status bar for triple encoding (color + icon + text)
- Big Job reaction: prompts over 500 characters trigger EXCITED expression with fanfare
- `r` / `R` keyboard shortcut to toggle role display on/off
- 10-second auto-timeout for role state, refreshed on re-trigger
- Sleep entry clears active role; wake-from-sleep processes the waking role event (no event loss)

### Changed
- `drawClawdToCanvas()` now accepts a color parameter (default: original rust color) for role-based body coloring
- Hook script extended with MCP detection branch and UserPromptSubmit branch
- `install-hook.sh` now manages 8 event types (added UserPromptSubmit)
- Hook allowlist extended with `role_*` and `mode_bigjob` events

### Fixed
- README: `stop` expression corrected from HAPPY to SURPRISED (matched actual code)

### Security
- Prompt content never leaves the hook script — only fixed string literals sent over serial
- UserPromptSubmit output suppressed (`skip_echo=true`) to prevent context injection
- jq failure falls back to empty string, never echoes raw input
- Keyword matching uses bash `[[ =~ ]]` with word boundaries, no shell expansion risk

## [0.4.0] — 2026-06-14

**Summary**: Canvas shrink, mini-Clawd polish, lifecycle event sprites.

### Added
- Unique sprites for lifecycle events: DISAPPOINTED (tool_fail), PANICKING (stop_fail), CONFUSED (perm_ask) with "???" overlay
- Lifecycle hook reactions: PostToolUseFailure, Stop, StopFailure, PermissionRequest (#1, #2, #4)
- `install-hook.sh` now manages all 7 hook event types atomically
- Sleep clears mini Clawds from display

### Changed
- Parent canvas shrunk from 148px to 98px with position-based walking and strip-clearing
- Mini walk: random flip before move + hard clamp to prevent canvas overflow
- Mini colors changed to deep rust (#BE3C14), both minis use the same color
- Minis linger until 8-second timeout instead of despawning immediately on subagent_stop
- Mini animation tuned: 80ms steps, 1px stride, 3px bounce for subtler movement

## [0.3.0] — 2026-06-13

**Summary**: Mini-Clawds spawn as subagent companions.

### Added
- Mini-Clawd system: up to 2 small Clawds appear when Claude Code subagents start, despawn when they stop (#10)
- Per-mini M5Canvas sprite buffers for artifact-free rendering (40×48px each)
- Active walk+bob animation: 50ms steps, 3px stride, 6px bounce, random direction changes
- "Piropiropiin" fanfare sound on spawn, parent shows HAPPY expression
- 5-minute auto-despawn timeout for stale subagent sessions
- Hook argument mode: `clawd-hook.sh` accepts event name as `$1` for SubagentStart/SubagentStop hooks
- `install-hook.sh` manages PostToolUse + SubagentStart + SubagentStop atomically

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
