#!/usr/bin/env bash

path="$1"

if [[ -z "$path" ]]; then
  echo "usage: $0 <path to data collection directory>"
fi

function grep() {
  command grep --color=always "$@"
}

echo "=== Warnings ==="
grep -r "warning" "$path" | grep -v "echo" | grep -v "pool.c" | grep -v "warnings generated"

echo "=== Errors ==="
grep -r "received non SIGTRAP: stopped" "$path"

grep -r "perf_event_open" "$path"
