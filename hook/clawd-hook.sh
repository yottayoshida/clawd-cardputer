#!/bin/bash
# PostToolUse / SubagentStart / SubagentStop / UserPromptSubmit hook
# Classifies events and sends them to Cardputer ADV via USB serial.
# Priority: test runner > git operation > skill > MCP server > tool_name fallback.
# Only fixed string literals are sent over serial (security invariant).
# Prompt content NEVER leaves this script (not passed as argv, not sent to serial).

input=$(cat)
skip_echo=false

# UserPromptSubmit: keyword classification (prompt_submit arg)
if [ $# -ge 1 ] && [ "$1" = "prompt_submit" ]; then
  skip_echo=true
  prompt=$(printf '%s' "$input" | jq -r '.prompt // "" | .[0:4096]' 2>/dev/null)
  prompt="${prompt:-}"
  prompt_lower=$(printf '%s' "$prompt" | tr '[:upper:]' '[:lower:]')
  prompt_len=${#prompt}

  event=""
  if [ "$prompt_len" -gt 500 ]; then
    event="mode_bigjob"
  elif [[ "$prompt_lower" =~ (^|[^[:alnum:]_])(review|audit|check|inspect)([^[:alnum:]_]|$) ]]; then
    event="role_detective"
  elif [[ "$prompt_lower" =~ (^|[^[:alnum:]_])(write|docs|document|note)([^[:alnum:]_]|$) ]]; then
    event="role_scribe"
  elif [[ "$prompt_lower" =~ (^|[^[:alnum:]_])(build|deploy|implement|ship)([^[:alnum:]_]|$) ]]; then
    event="role_worker"
  elif [[ "$prompt_lower" =~ (^|[^[:alnum:]_])(test|verify|validate)([^[:alnum:]_]|$) ]]; then
    event="role_nervous"
  fi

# SubagentStart/SubagentStop/Stop etc.: argument mode
elif [ $# -ge 1 ]; then
  event="$1"

# PostToolUse: stdin JSON classification
else
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

  elif [[ "$tool_name" == mcp__* ]]; then
    server="${tool_name#mcp__}"
    server="${server%%__*}"
    server=$(printf '%s' "$server" | tr '[:upper:]' '[:lower:]')

    case "$server" in
      *github*)  event="role_detective" ;;
      *slack*)   event="role_messenger" ;;
      *notion*)  event="role_scribe" ;;
      *figma*)   event="role_artist" ;;
      *drive*)   event="role_explorer" ;;
      *)         event="" ;;
    esac
  fi
fi

# allowlist guard — only known events pass through
case "$event" in
  dirty|clean|conflict|branch|pr_open|\
  test_pass|test_fail|test_run|\
  bash|edit|read|glob|grep|write|test|search|commit|push|\
  party|skill|\
  subagent_start|subagent_stop|\
  tool_fail|stop|stop_fail|perm_ask|\
  role_detective|role_messenger|role_scribe|role_artist|role_explorer|role_worker|role_nervous|\
  mode_bigjob) ;;
  *) event="" ;;
esac

if [ -n "$event" ]; then
  port="${CLAWD_PORT:-$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)}"
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

if ! $skip_echo; then
  echo "$input"
fi
