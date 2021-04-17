#!/usr/bin/env bash

# Number of seconds to sleep between tests - defaults to 1m
SLEEP_BETWEEN_TESTS="${SLEEP_BETWEEN_TESTS:-60}"

function extract_runs_to_complement() {
  path="$1"
  matches="$(grep -r "received non SIGTRAP: stopped" "$path/micro" | sed 's/^\([^:]\+\).*$/\1/' | sed -e "s!^$path/micro/!!" -e 's/\.txt$//' | tr '.' '\t')"
  #./data/low-end-laptop//micro/mceliece_6960119_gcc_ref-optimized.keypair.txt:wait: received non SIGTRAP: stopped (signal)
  echo "$matches"
}

function extract_benchmark_command() {
  path="$1"
  binary="$2"
  stage="$3"

  command="$(head -n 3 "$path/micro/$binary.$stage.txt" | tail -1 | grep '^perforator')"
  if [[ -z "$command" ]]; then
    echo ""
  else
    echo "$command" | grep -o -e '-- .\+' | cut -c4-
  fi
}

function extract_events() {
  path="$1"
  binary="$2"
  stage="$3"

  command="$(head -n 3 "$path/micro/$binary.$stage.txt" | tail -1 | grep '^perforator')"
  if [[ -z "$command" ]]; then
    echo ""
  else
    echo "$command" | grep -o -e '-e [0-9A-Za-z,_-]\+' | cut -c4- | tr ',' ' '
  fi
}

function extract_regions() {
  path="$1"
  binary="$2"
  stage="$3"

  command="$(head -n 3 "$path/micro/$binary.$stage.txt" | tail -1 | grep '^perforator')"
  if [[ -z "$command" ]]; then
    echo ""
  else
    echo "$command" | grep -o -e '-r [a-zA-Z0-9_-]\+' | cut -c4- | paste -sd' '
  fi
}

# Wait SLEEP_BETWEEN_TEST seconds before continuing
function wait_for_cooldown() {
  echo "Waiting ${SLEEP_BETWEEN_TESTS}s for cooldown"
  sleep "$SLEEP_BETWEEN_TESTS"
}

# Run a command and wrap it with time started, time ended
function run_wrapped() (
  echo "=== $(date '+%Y-%m-%d %H:%M:%S') Running command ==="
  echo "command:"
  echo "$@"
  echo "output:"
  "$@"
  echo "=== $(date '+%Y-%m-%d %H:%M:%S') Command ended with exit code $? ==="
)

function micro_benchmark_kem() {
  command="$1"
  stage="$2"
  events="$(echo "$3" | tr ' ' ',')"
  regions="$(echo "$4" | sed 's/^\| / -r /g')"
  output_directory="$5"

  binary="$(echo "$command" | cut -d' ' -f1)"

  if ! which "perforator" &> /dev/null; then
    echo "warning: skipping micro_benchmark_kem '$binary' - perforator missing" | tee "$output_directory/micro/$(basename "$binary").$stage.txt"
    return
  fi

  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping micro_benchmark_kem '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").$stage.txt"
    return
  fi

  #wait_for_cooldown
  echo run_wrapped perforator -V --ignore-missing-regions --no-sort --summary --csv -e "$events" $regions -- $command 2>&1 | tee "$output_directory/micro/$(basename "$binary").$stage.txt"
}

function print_usage() {
  echo "usage: $0 <command> [options]"
  echo "commands:"
  echo -e "\textract <path to previous output>"
  echo -e "\trun <path to previous output> <new environment name>"
}

command="$1"
if [[ -z "$command" ]]; then
  print_usage
elif [[ "$command" = "extract" ]]; then
  previous_output_path="$2"
  if [[ -z "$previous_output_path" ]]; then
    print_usage
  else
    echo "algorithm,stage"
    extract_runs_to_complement "$previous_output_path" | tr '\t' ','
  fi
elif [[ "$command" = "run" ]]; then
  previous_output_path="$2"
  environment_name="$3"
  if [[ -z "$previous_output_path" ]] || [[ -z "$environment_name" ]]; then
    print_usage
  else
    output_directory="data/benchmarks/$environment_name"

    # If the directory already exists, prompt the user
    if [[ -d "$output_directory" ]]; then
      echo -n "Continuing may overwrite contents of '$output_directory/micro'. Press any key to continue: "
      read -rn 1
      echo ""
      echo -n "Are you sure? Press any key to continue: "
      read -rn 1
      echo ""
    fi

    # Create output directories
    mkdir -p "$output_directory/micro"

    runs="$(extract_runs_to_complement "$previous_output_path")"
    echo "=== Runs to complement ==="
    echo "$runs" | tr '\t' ','
    while read -r run; do
      binary="$(echo "$run" | awk '{print $1;}')"
      stage="$(echo "$run" | awk '{print $2;}')"
      echo "=== Complementing $binary - $stage ==="
      echo "benchmark command:"
      benchmark_command="$(extract_benchmark_command "$previous_output_path" "$binary" "$stage")"
      echo "$benchmark_command"
      echo "events:"
      events="$(extract_events "$previous_output_path" "$binary" "$stage")"
      echo "$events"
      echo "regions:"
      regions="$(extract_regions "$previous_output_path" "$binary" "$stage")"
      echo "$regions"
      micro_benchmark_kem "$benchmark_command" "$stage" "$events" "$regions" "$output_directory"
    done <<<"$runs"
  fi
else
  print_usage
fi
