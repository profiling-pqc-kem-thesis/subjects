#!/usr/bin/env bash

object_file="$1"
dump="$(objdump -d "$object_file")"

function get_stack_size() {
  function_name="$1"
  hex="$(echo "$dump" | grep -A5 "<$function_name>:" | grep -io 'sub[ \t]\+$0x[0-9abcdef]\+,%rsp' | head -1 | sed 's/sub[0 \t]\+$0x\([0-9abcdefABCDEF]\+\),%rsp/\1/')"
  if [[ -z "$hex" ]]; then
    echo "N/A"
  else
    printf "%d\n" "0x$hex"
  fi
}

# get_stack_size "$object_file" "_perform_keypair"

counts="$(nm -S --size-sort -t d "$object_file" | awk '{print $2,$4;}')"

printf "%-50s %10s %10s\n" "symbol" "function size" "stack allocation"
while read -r count; do
  symbol="$(echo "$count" | awk '{print $2;}')"
  stack_size="$(echo "$count" | awk '{print $1;}' | sed 's/^0\+//')"
  stack_allocation="$(get_stack_size "$symbol")"
  printf "%-50s %10s %10s\n" "$symbol" "$stack_size" "$stack_allocation"
done <<<"$counts"
