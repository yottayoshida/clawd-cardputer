#!/bin/bash
set -euo pipefail

HOOK_DIR="$(cd "$(dirname "$0")" && pwd)"
HOOK_PATH="$HOOK_DIR/clawd-hook.sh"
SETTINGS="$HOME/.claude/settings.json"
ACTION="${1:-install}"

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
  has_hook "PostToolUse"
}

case "$ACTION" in
  status)
    require_jq
    for evt in PostToolUse SubagentStart SubagentStop; do
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
    if ! is_installed; then
      echo "clawd hook is not installed. Nothing to do."
      exit 0
    fi
    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"
    tmp=$(mktemp)
    jq '
      .hooks.PostToolUse |= map(select(.hooks[]?.command | contains("clawd-hook.sh") | not)) |
      .hooks.SubagentStart |= map(select(.hooks[]?.command | contains("clawd-hook.sh") | not)) |
      .hooks.SubagentStop |= map(select(.hooks[]?.command | contains("clawd-hook.sh") | not))
    ' "$SETTINGS" > "$tmp"
    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi
    mv "$tmp" "$SETTINGS"
    echo "clawd hook removed (PostToolUse + SubagentStart + SubagentStop). Backup saved."
    exit 0
    ;;

  --dry-run)
    require_jq
    if is_installed; then
      echo "clawd hook is already installed. No changes needed."
      exit 0
    fi
    echo "Would add to $SETTINGS:"
    echo ""
    echo "PostToolUse:"
    jq -n --arg cmd "$HOOK_PATH" \
      '{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}'
    echo ""
    echo "SubagentStart:"
    jq -n --arg cmd "$HOOK_PATH subagent_start" \
      '{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}'
    echo ""
    echo "SubagentStop:"
    jq -n --arg cmd "$HOOK_PATH subagent_stop" \
      '{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}'
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
    if is_installed; then
      echo "clawd hook is already installed."
      exit 0
    fi

    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"

    tmp=$(mktemp)
    jq --arg cmd "$HOOK_PATH" --arg cmd_start "$HOOK_PATH subagent_start" --arg cmd_stop "$HOOK_PATH subagent_stop" '
      .hooks.PostToolUse = (.hooks.PostToolUse // []) + [{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}] |
      .hooks.SubagentStart = (.hooks.SubagentStart // []) + [{"hooks": [{"command": $cmd_start, "type": "command"}], "matcher": "*"}] |
      .hooks.SubagentStop = (.hooks.SubagentStop // []) + [{"hooks": [{"command": $cmd_stop, "type": "command"}], "matcher": "*"}]
    ' "$SETTINGS" > "$tmp"

    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi

    mv "$tmp" "$SETTINGS"
    echo "clawd hook installed (PostToolUse + SubagentStart + SubagentStop)."
    echo "  Backup: $SETTINGS.bak.*"
    echo ""
    echo "Restart Claude Code for the hook to take effect."
    ;;

  *)
    usage
    ;;
esac
