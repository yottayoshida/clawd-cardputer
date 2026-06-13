#!/bin/bash
# PostToolUse hook — sends tool name to Cardputer ADV via serial.
# Exits in <50ms even when device is disconnected.

input=$(cat)

event=$(printf '%s' "$input" | jq -r '.tool_name // empty' 2>/dev/null | tr '[:upper:]' '[:lower:]')

# /develop invocation → party mode
if [ "$event" = "skill" ]; then
  skill=$(printf '%s' "$input" | jq -r '.tool_input.skill // empty' 2>/dev/null)
  [ "$skill" = "develop" ] && event="party"
fi

if [ -n "$event" ]; then
  port="${ADVCCHI_PORT:-$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)}"
  if [ -n "$port" ] && [ -c "$port" ]; then
    python3 -c "
import serial
try:
  s=serial.Serial('$port',115200,timeout=0.1,dsrdtr=False,rtscts=False)
  s.write(b'$event\n')
  s.flush()
  s.close()
except: pass
" >/dev/null 2>&1
  fi
fi

echo "$input"
