#!/bin/bash
set -euo pipefail

HOOK_DIR="$(cd "$(dirname "$0")" && pwd)"
HOOK_PATH="$HOOK_DIR/advcchi-hook.sh"
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

is_installed() {
  jq -e '.hooks.PostToolUse[]? | select(.hooks[]?.command | contains("advcchi-hook.sh"))' \
    "$SETTINGS" > /dev/null 2>&1
}

case "$ACTION" in
  status)
    require_jq
    if is_installed; then
      echo "advcchi hook is installed."
      jq '.hooks.PostToolUse[] | select(.hooks[]?.command | contains("advcchi-hook.sh"))' "$SETTINGS"
    else
      echo "advcchi hook is NOT installed."
    fi
    exit 0
    ;;

  uninstall)
    require_jq
    if ! is_installed; then
      echo "advcchi hook is not installed. Nothing to do."
      exit 0
    fi
    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"
    tmp=$(mktemp)
    jq '.hooks.PostToolUse |= map(select(.hooks[]?.command | contains("advcchi-hook.sh") | not))' \
      "$SETTINGS" > "$tmp"
    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi
    mv "$tmp" "$SETTINGS"
    echo "advcchi hook removed. Backup saved."
    exit 0
    ;;

  --dry-run)
    require_jq
    if is_installed; then
      echo "advcchi hook is already installed. No changes needed."
      exit 0
    fi
    echo "Would add to $SETTINGS:"
    echo ""
    jq -n --arg cmd "$HOOK_PATH" \
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
      echo "advcchi hook is already installed."
      exit 0
    fi

    cp "$SETTINGS" "$SETTINGS.bak.$(date +%Y%m%d%H%M%S)"

    tmp=$(mktemp)
    jq --arg cmd "$HOOK_PATH" \
      '.hooks.PostToolUse += [{"hooks": [{"command": $cmd, "type": "command"}], "matcher": "*"}]' \
      "$SETTINGS" > "$tmp"

    if ! jq empty "$tmp" 2>/dev/null; then
      echo "Error: generated settings.json is invalid. Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi

    existing_count=$(jq '.hooks.PostToolUse | length' "$SETTINGS")
    new_count=$(jq '.hooks.PostToolUse | length' "$tmp")
    if [ "$new_count" -ne "$((existing_count + 1))" ]; then
      echo "Error: unexpected entry count change ($existing_count -> $new_count). Aborting." >&2
      rm -f "$tmp"
      exit 1
    fi

    mv "$tmp" "$SETTINGS"
    echo "advcchi hook installed."
    echo "  PostToolUse entries: $existing_count -> $new_count"
    echo "  Backup: $SETTINGS.bak.*"
    echo ""
    echo "Restart Claude Code for the hook to take effect."
    ;;

  *)
    usage
    ;;
esac
