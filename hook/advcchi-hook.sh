#!/bin/bash
# PostToolUse hook — classifies tool invocations and sends events
# to Cardputer ADV via USB serial.
# Priority: test runner > git operation > skill > tool_name fallback.
# Only fixed string literals are sent over serial (security invariant).

input=$(cat)

tool_name=$(printf '%s' "$input" | jq -r '(.tool_name // "" | ascii_downcase)' 2>/dev/null)
event="$tool_name"

if [ "$tool_name" = "bash" ]; then
  cmd=$(printf '%s' "$input" | jq -r '.tool_input.command // ""' 2>/dev/null)
  stdout_head=$(printf '%s' "$input" | jq -r '.tool_response.stdout // "" | .[0:4096]' 2>/dev/null)

  # test runner detection (highest priority — check fail before pass)
  if [[ "$cmd" =~ (^|[[:space:]\;\&\|\(])(pytest|cargo[[:space:]]+test|npm[[:space:]]+test|npx[[:space:]]+vitest|pnpm[[:space:]]+test|uv[[:space:]]+run[[:space:]]+pytest|poetry[[:space:]]+run[[:space:]]+pytest|cargo[[:space:]]+nextest[[:space:]]+run) ]]; then
    if [[ "$stdout_head" =~ FAILED|failed|error ]]; then
      event="test_fail"
    elif [[ "$stdout_head" =~ "test result: ok"|passed ]]; then
      event="test_pass"
    else
      event="test_run"
    fi
  # git operation detection
  elif [[ "$cmd" =~ (^|[[:space:]\;\&\|\(])gh[[:space:]]+pr[[:space:]]+create ]]; then
    event="pr_open"
  elif [[ "$cmd" =~ (^|[[:space:]\;\&\|\(])git[[:space:]] ]]; then
    if [[ "$cmd" =~ git[[:space:]]+(checkout|switch)[[:space:]]+-[bc] ]]; then
      event="branch"
    elif [[ "$cmd" =~ git[[:space:]]+status ]]; then
      if [ -n "$stdout_head" ]; then
        event="dirty"
      else
        event="clean"
      fi
    elif [[ "$cmd" =~ git[[:space:]]+(merge|rebase) ]]; then
      if [[ "$stdout_head" =~ CONFLICT|"Merge conflict" ]]; then
        event="conflict"
      fi
    fi
  fi

elif [ "$tool_name" = "skill" ]; then
  skill=$(printf '%s' "$input" | jq -r '.tool_input.skill // ""' 2>/dev/null)
  [ "$skill" = "develop" ] && event="party"
fi

# allowlist guard — only known events pass through
case "$event" in
  dirty|clean|conflict|branch|pr_open|\
  test_pass|test_fail|test_run|\
  bash|edit|read|glob|grep|write|test|search|commit|push|\
  party|skill) ;;
  *) event="" ;;
esac

if [ -n "$event" ]; then
  port="${ADVCCHI_PORT:-$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)}"
  if [ -n "$port" ] && [ -c "$port" ]; then
    python3 -c "
import sys, serial
try:
  s=serial.Serial(sys.argv[1],115200,timeout=0.1,dsrdtr=False,rtscts=False)
  s.write((sys.argv[2]+'\n').encode())
  s.flush(); s.close()
except: pass
" "$port" "$event" >/dev/null 2>&1
  fi
fi

echo "$input"
