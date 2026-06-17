#!/bin/bash
set -euo pipefail

HOOK_DIR="$(cd "$(dirname "$0")" && pwd)"
HOOK_PATH="$HOOK_DIR/clawd-hook.sh"
SETTINGS="$HOME/.claude/settings.json"
ACTION="${1:-install}"

# Single source of truth for all managed hook events.
# Parallel arrays: EVENT_KEYS[i] is the hook name, EVENT_ARGS[i] is the serial arg.
EVENT_KEYS=(PostToolUse SubagentStart SubagentStop PostToolUseFailure Stop StopFailure PermissionRequest UserPromptSubmit)
EVENT_ARGS=(""          subagent_start subagent_stop tool_fail          stop stop_fail  perm_ask         prompt_submit)

usage() {
  echo "Usage: $0 [install|uninstall|status|--dry-run]"
  exit 1
}

require_jq() {
  if ! command -v jq &>/dev/null; then
    echo "Error: jq is required. Install with: brew install jq" >&2
    exit 1
  fi
}

has_hook() {
  jq -e ".hooks.${1}[]? | select(.hooks[]?.command | contains(\"clawd-hook.sh\"))" \
    "$SETTINGS" > /dev/null 2>&1
}

is_installed() {
  for evt in "${EVENT_KEYS[@]}"; do
    if ! has_hook "$evt"; then
      return 1
    fi
  done
  return 0
}

remove_clawd_hooks() {
  jq '
    .hooks.PostToolUse = (.hooks.PostToolUse // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.SubagentStart = (.hooks.SubagentStart // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.SubagentStop = (.hooks.SubagentStop // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.PostToolUseFailure = (.hooks.PostToolUseFailure // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.Stop = (.hooks.Stop // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.StopFailure = (.hooks.StopFailure // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.PermissionRequest = (.hooks.PermissionRequest // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))) |
    .hooks.UserPromptSubmit = (.hooks.UserPromptSubmit // [] | map(select(.hooks[]?.command | contains("clawd-hook.sh") | not)))
  ' "$SETTINGS"
}

case "$ACTION" in
  status)
    require_jq
    for evt in "${EVENT_KEYS[@]}"; do
      if has_hook "$evt"; then
        echo "clawd hook ($evt): installed"
      else
        echo "clawd hook ($evt): NOT installed"
      fi
    done
    exit 0
    ;;

  uninstall)
    require_jq
    has_any=false
    for evt in "${EVENT_KEYS[@]}"; do
      if has_hook "$evt"; then has_any=true; break; fi
    done
    if ! $has_any; then
      echo "clawd hook is not installed. Nothing to do."
      exit 0
    fi
    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"
    tmp=$(mktemp)
    remove_clawd_hooks > "$tmp"
    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi
    mv "$tmp" "$SETTINGS"
    echo "clawd hook removed (all ${#EVENT_KEYS[@]} event types). Backup saved."
    exit 0
    ;;

  --dry-run)
    require_jq
    if is_installed; then
      echo "clawd hook is already installed (all ${#EVENT_KEYS[@]} events). No changes needed."
      exit 0
    fi
    echo "Would add to $SETTINGS:"
    for i in "${!EVENT_KEYS[@]}"; do
      evt="${EVENT_KEYS[$i]}"
      arg="${EVENT_ARGS[$i]}"
      if [ -n "$arg" ]; then
        cmd="$HOOK_PATH $arg"
      else
        cmd="$HOOK_PATH"
      fi
      echo ""
      echo "$evt:"
      jq -n --arg cmd "$cmd" \
        '{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}'
    done
    exit 0
    ;;

  install)
    require_jq
    if [ ! -f "$SETTINGS" ]; then
      echo "Error: $SETTINGS not found" >&2
      exit 1
    fi
    if [ ! -x "$HOOK_PATH" ]; then
      echo "Error: $HOOK_PATH not found or not executable" >&2
      exit 1
    fi

    # Idempotent: remove existing clawd hooks before adding all
    has_any=false
    for evt in "${EVENT_KEYS[@]}"; do
      if has_hook "$evt"; then has_any=true; break; fi
    done
    if $has_any; then
      echo "Existing clawd hooks detected. Removing before reinstall..."
      tmp_clean=$(mktemp)
      remove_clawd_hooks > "$tmp_clean"
      if ! jq empty "$tmp_clean" 2>/dev/null; then
        echo "Error: cleanup produced invalid JSON. Aborting." >&2
        rm -f "$tmp_clean"
        exit 1
      fi
      mv "$tmp_clean" "$SETTINGS"
    fi

    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"

    tmp=$(mktemp)
    jq --arg cmd "$HOOK_PATH" \
       --arg cmd_start "$HOOK_PATH subagent_start" \
       --arg cmd_stop "$HOOK_PATH subagent_stop" \
       --arg cmd_tool_fail "$HOOK_PATH tool_fail" \
       --arg cmd_stop_evt "$HOOK_PATH stop" \
       --arg cmd_stop_fail "$HOOK_PATH stop_fail" \
       --arg cmd_perm_ask "$HOOK_PATH perm_ask" \
       --arg cmd_prompt "$HOOK_PATH prompt_submit" '
      .hooks.PostToolUse = (.hooks.PostToolUse // []) + [{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}] |
      .hooks.SubagentStart = (.hooks.SubagentStart // []) + [{"hooks": [{"command": $cmd_start, "type": "command"}], "matcher": "*"}] |
      .hooks.SubagentStop = (.hooks.SubagentStop // []) + [{"hooks": [{"command": $cmd_stop, "type": "command"}], "matcher": "*"}] |
      .hooks.PostToolUseFailure = (.hooks.PostToolUseFailure // []) + [{"hooks": [{"command": $cmd_tool_fail, "type": "command"}], "matcher": "*"}] |
      .hooks.Stop = (.hooks.Stop // []) + [{"hooks": [{"command": $cmd_stop_evt, "type": "command"}], "matcher": "*"}] |
      .hooks.StopFailure = (.hooks.StopFailure // []) + [{"hooks": [{"command": $cmd_stop_fail, "type": "command"}], "matcher": "*"}] |
      .hooks.PermissionRequest = (.hooks.PermissionRequest // []) + [{"hooks": [{"command": $cmd_perm_ask, "type": "command"}], "matcher": "*"}] |
      .hooks.UserPromptSubmit = (.hooks.UserPromptSubmit // []) + [{"hooks": [{"command": $cmd_prompt, "type": "command"}], "matcher": "*"}]
    ' "$SETTINGS" > "$tmp"

    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi

    mv "$tmp" "$SETTINGS"
    echo "clawd hook installed (all ${#EVENT_KEYS[@]} event types)."
    echo "  Backup: $SETTINGS.bak.*"
    echo ""
    echo "Restart Claude Code for the hook to take effect."
    ;;

  *)
    usage
    ;;
esac
