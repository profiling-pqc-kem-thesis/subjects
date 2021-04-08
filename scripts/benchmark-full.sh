#!/usr/bin/env bash

# === ENVIRONMENT VARIABLES ===

DIRECTORY="$(dirname "$(realpath -s "$0")")"

# Steps to skip, comma separated
SKIP_STEPS="${SKIP_STEPS:-}"

# Maximum number of seconds to a allow a single benchmark to run - defaults to 1h
TIMEOUT="${TIMEOUT:-3600}"

# Number of iterations to run in sequential benchmarks
SEQUENTIAL_ITERATIONS="${SEQUENTIAL_ITERATIONS:-1000}"

# Number of iteartions to run in sequential benchmarks under heaptrack
HEAPTRACK_SEQUENTIAL_ITERATIONS="${HEAPTRACK_SEQUENTIAL_ITERATIONS:-10}"

# Number of iterations to run in parallel benchmarks (per thread)
PARALLEL_ITERATIONS="${PARALLEL_ITERATIONS:-1000}"

# Number of seconds to sleep between tests - defaults to 1m
SLEEP_BETWEEN_TESTS="${SLEEP_BETWEEN_TESTS:-60}"

# The comma separated events to use with perf / perforator
PERF_EVENTS="${PERF_EVENTS:-cpu-cycles,instructions,cache-misses,page-faults,task-clock}"

KEM_REGIONS="crypto_kem_keypair crypto_kem_enc crypto_kem_dec"
NTRU_REGIONS="$KEM_REGIONS poly_Rq_mul poly_S3_inv randombytes poly_Rq_inv poly_R2_inv poly_R2_inv_to_Rq_inv poly_Sq_mul"
MCELIECE_REGIONS="$KEM_REGIONS pk_gen gen_e syndrome syndrome_asm same_mask gf_mul root eval gf_add synd"

# == PARSE ARGUMENTS ==

# Set SKIP_STEP_i for each specified i
while read -r step; do
    eval "SKIP_STEP_$step=1"
done <<<"$(echo "$SKIP_STEPS" | tr ',' '\n')"

environment_name="$1"

# Require parameters to be set, or die with usage
if [[ -z "$environment_name" ]]; then
  echo "usage: $0 <environment name>"
  exit 1
fi

# === FUNCTION DEFINITIONS ===

# Cleanup on CTRL+C
function cleanup() {
  ARG="$?"
  echo "=== Exiting benchmark ==="
  echo -n "Benchmark ended on: "
  LANG=en_US date | tee "$output_directory/end.txt"
  end_time="$(date '+%s')"
  duration="$((end_time-start_time))"
  echo -n "Duration: "
  printf '%dh %dm %ds\n' $((duration/3600)) $((duration%3600/60)) $((duration%60)) | tee "$output_directory/duration.txt"
  exit "$ARG"
}
trap cleanup EXIT

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

# Ensure that each specified command exists or die
function assert_commands() {
  failures=0
  for name in "$@"; do
    if ! which "$name" &> /dev/null; then
    echo "Expected command '$name' to be available"
    failures=$((failures+1))
  fi
  done
  if [[ $failures -gt 0 ]]; then
    exit 1
  fi
}

function micro_benchmark_kem() {
  binary="$1"

  if ! which "perforator" &> /dev/null; then
    echo "warning: skipping micro_benchmark_kem '$binary' - perforator missing" | tee "$output_directory/micro/$(basename "$binary").keypair.txt"
    echo "warning: skipping micro_benchmark_kem '$binary' - perforator missing" | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"
    echo "warning: skipping micro_benchmark_kem '$binary' - perforator missing" | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
    return
  fi

  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping micro_benchmark_kem '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").keypair.txt"
    echo "warning: skipping micro_benchmark_kem '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"
    echo "warning: skipping micro_benchmark_kem '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
    return
  fi

  keypair_methods="$(echo "$2" | sed 's/^\| / -r /g')"
  wait_for_cooldown
  run_wrapped perforator --ignore-missing-regions --no-sort --summary --csv -e "$PERF_EVENTS" $keypair_methods -- "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").keypair.txt"

  encrypt_methods="$(echo "$3" | sed 's/^\| / -r /g')"
  wait_for_cooldown
  run_wrapped perforator --ignore-missing-regions --no-sort --summary --csv -e "$PERF_EVENTS" $encrypt_methods -- "$binary" sequential --encrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"

  decrypt_methods="$(echo "$4" | sed 's/^\| / -r /g')"
  wait_for_cooldown
  run_wrapped perforator --ignore-missing-regions --no-sort --summary --csv -e "$PERF_EVENTS" $decrypt_methods -- "$binary" sequential --decrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
}

function sequential_benchmark_kex() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping sequential_benchmark_kex '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
    echo "warning: skipping sequential_benchmark_kex '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").exchange.txt"
    return
  fi

  wait_for_cooldown
  run_wrapped "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"

  wait_for_cooldown
  run_wrapped "$binary" sequential --exchange --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").exchange.txt"
}

function sequential_benchmark_kem() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping sequential_benchmark_kem '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
    echo "warning: skipping sequential_benchmark_kem '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").encrypt.txt"
    echo "warning: skipping sequential_benchmark_kem '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").decrypt.txt"
    return
  fi

  wait_for_cooldown
  run_wrapped "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"

  wait_for_cooldown
  run_wrapped "$binary" sequential --encrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").encrypt.txt"

  wait_for_cooldown
  run_wrapped "$binary" sequential --decrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").decrypt.txt"
}

# Returns smt * cores * 4 on Linux and macOS
function get_max_thread_count() {
  if [[ "$(uname)" = "Darwin" ]]; then
    # Cores are logical cores - physical cores * smt
    cores="$(sysctl machdep.cpu.thread_count | cut -d ':' -f2)"
    echo "$((cores*4))"
  else
    # Cores are logical cores - physical cores * smt
    cores="$(lscpu | grep '^CPU(s):' | cut -d':' -f2 | sed 's/[ \t]//g')"
    echo "$((cores*4))"
  fi
}

function parallel_benchmark_kex() {
  binary="$1"
  max_thread_count="$(get_max_thread_count)"
  thread_count=1
  while true; do
    if [[ ! -f "$binary" ]]; then
      echo "warning: skipping parallel_benchmark_kex '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      echo "warning: skipping parallel_benchmark_kex '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").exchange.$thread_count.txt"
    else
      wait_for_cooldown
      run_wrapped "$binary" parallel --keypair --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"

      wait_for_cooldown
      run_wrapped "$binary" parallel --exchange --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").exchange.$thread_count.txt"
    fi

    thread_count=$((thread_count*2))
    if [[ $thread_count -gt $max_thread_count ]]; then
      break
    fi
  done
}

function parallel_benchmark_kem() {
  binary="$1"
  max_thread_count="$(get_max_thread_count)"
  thread_count=1
  while true; do
    if [[ ! -f "$binary" ]]; then
      echo "warning: skipping parallel_benchmark_kem '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      echo "warning: skipping parallel_benchmark_kem '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").encrypt.$thread_count.txt"
      echo "warning: skipping parallel_benchmark_kem '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").decrypt.$thread_count.txt"
    else
      wait_for_cooldown
      run_wrapped "$binary" parallel --keypair --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"

      wait_for_cooldown
      run_wrapped "$binary" parallel --encrypt --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").encrypt.$thread_count.txt"

      wait_for_cooldown
      run_wrapped "$binary" parallel --decrypt --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").decrypt.$thread_count.txt"
    fi

    thread_count=$((thread_count*2))
    if [[ $thread_count -gt $max_thread_count ]]; then
      break
    fi
  done
}

function heap_benchmark_kex() {
  binary="$1"
  if ! which "heaptrack" &> /dev/null; then
    echo "warning: skipping heap_benchmark_kex '$binary' - heaptrack missing" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping heap_benchmark_kex '$binary' - heaptrack missing" | tee "$output_directory/heap/$(basename "$binary").exchange.txt"
    return
  fi

  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping heap_benchmark_kex '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping heap_benchmark_kex '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").exchange.txt"
    return
  fi

  run_wrapped heaptrack "$binary" sequential --keypair --iterations "$HEAPTRACK_SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"

  run_wrapped heaptrack "$binary" sequential --exchange --iterations "$HEAPTRACK_SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").exchange.txt"

  mv heaptrack.* "$output_directory/heap"
}

function heap_benchmark_kem() {
  binary="$1"

  if ! which "heaptrack" &> /dev/null; then
    echo "warning: skipping heap_benchmark_kem '$binary' - heaptrack missing" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping heap_benchmark_kem '$binary' - heaptrack missing" | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"
    echo "warning: skipping heap_benchmark_kem '$binary' - heaptrack missing" | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"
   return
  fi

  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping heap_benchmark_kem '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping heap_benchmark_kem '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"
    echo "warning: skipping heap_benchmark_kem '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"
    return
  fi

  run_wrapped heaptrack "$binary" sequential --keypair --iterations "$HEAPTRACK_SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"

  run_wrapped heaptrack "$binary" sequential --encrypt --iterations "$HEAPTRACK_SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"

  run_wrapped heaptrack "$binary" sequential --decrypt --iterations "$HEAPTRACK_SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"

  mv heaptrack.* "$output_directory/heap"
}

function benchmark_stack() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark_stack '$binary' - no such file" | tee "$output_directory/stack/$(basename "$binary").txt"
    return
  fi

  run_wrapped bash "$DIRECTORY/calculate-stack-usage-of-object.sh" "$binary" | tee "$output_directory/stack/$(basename "$binary").txt"
}

function plan() {
  command="$1"
  shift
  arguments="$*"

  line="$(grep "$output_directory/progress.txt" -e "$command $arguments")"
  if [[ "$line" = "" ]]; then
    echo "planned $command $arguments" >> "$output_directory/progress.txt"
  fi
}

function run_jobs() {
  commands_to_run="$*"
  for command_to_run in $commands_to_run; do
    jobs="$(grep "$output_directory/progress.txt" -e "$command_to_run")"
    while read -r job; do
      status="$(echo "$job" | cut -d ' ' -f1)"
      command="$(echo "$job" | cut -d ' ' -f2-)"
      if [[ "$status" = "completed" ]]; then
        echo "Skipping completed job: $command"
        continue
      elif [[ "$status" = "started" ]]; then
        echo "Rerunning incomplete job: $command"
      elif [[ "$status" = "planned" ]]; then
        echo "Running planned job: $command"
      fi
      sed -i "$output_directory/progress.txt" -e "s!^$status $command\$!started $command!"
      eval "$command"
      sed -i "$output_directory/progress.txt" -e "s!^started $command\$!completed $command!"
    done <<<"$jobs"
  done
}

## === START OF BENCHMARKS ===

output_directory="data/benchmarks/$environment_name"

# If the directory already exists, prompt the user
if [[ -d "$output_directory" ]]; then
  echo -n "Continuing will continue where the benchmark left off, but may overwrite contents of '$output_directory'. Press any key to continue: "
  read -rn 1
  echo ""
  echo -n "Are you sure? Press any key to continue: "
  read -rn 1
  echo ""
fi

# Create output directories
mkdir -p "$output_directory"
mkdir -p "$output_directory/micro"
mkdir -p "$output_directory/heap"
mkdir -p "$output_directory/sequential"
mkdir -p "$output_directory/parallel"
mkdir -p "$output_directory/stack"
mkdir -p "$output_directory/versions"

# Copy the benchmark script for easy version control
cat "$0" > "$output_directory/benchmark-full.sh"

# Ensure the progress file is created
touch "$output_directory/progress.txt"

# Print start of script
echo -n "Benchmark started on: "
LANG=en_US date | tee "$output_directory/start.txt"
start_time="$(date '+%s')"
echo ""

echo "=== STEP 1 - Checking Prerequisites ==="
if [[ -z "$SKIP_STEP_1" ]]; then
  assert_commands "sqlite3" "heaptrack" "make" "gcc" "clang" "perforator" "nm" "objdump" | tee "$output_directory/commands.txt"
  heaptrack -v 2>&1 | tee "$output_directory/versions/heaptrack.txt"
  perforator -v 2>&1 | tee "$output_directory/versions/perforator.txt"
  gcc -v 2>&1 | tee "$output_directory/versions/gcc.txt"
  clang -v 2>&1 | tee "$output_directory/versions/clang.txt"
  nm --version 2>&1 | tee "$output_directory/versions/nm.txt"
  objdump --version 2>&1 | tee "$output_directory/versions/objdump.txt"
  make -v 2>&1 | tee "$output_directory/versions/make.txt"
  sqlite3 --version 2>&1 | tee "$output_directory/versions/sqlite3.txt"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 2 - Build Benchmarks ==="
if [[ -z "$SKIP_STEP_2" ]]; then
  VERBOSE=yes run_wrapped make benchmarks 2>&1 | tee "$output_directory/build.txt"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 3 - Collect Environmental Information ==="
if [[ -z "$SKIP_STEP_3" ]]; then
  bash "$DIRECTORY/collect-environment-information.sh" "$output_directory/environment.sqlite" "$environment_name"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 4 - Profile Heap Usage ==="
if [[ -z "$SKIP_STEP_4" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    plan heap_benchmark_kex "./ecdh/build/ecdh_25519_gcc_plain-optimized"
    plan heap_benchmark_kex "./ecdh/build/ecdh_25519_clang_plain-optimized"

    plan heap_benchmark_kex "./ecdh/build/ecdh_p256_gcc_plain-optimized"
    plan heap_benchmark_kex "./ecdh/build/ecdh_p256_clang_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    plan heap_benchmark_kex "./dh/build/dh_gcc_plain-optimized"
    plan heap_benchmark_kex "./dh/build/dh_clang_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref"
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_clang_ref-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2"
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hrss701_clang_avx2-optimized"

    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref"
    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_clang_ref-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2"
    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2-optimized"
    plan heap_benchmark_kem "./ntru/build/ntru_hps4096821_clang_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_avx2-optimized"

    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_avx2-optimized"

    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_avx2-optimized"

    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_ref-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2-optimized"
    plan heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_avx2-optimized"
  fi

  run_jobs heap_benchmark_kex heap_benchmark_kem
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 5 - Micro Benchmarks ==="
if [[ -z "$SKIP_STEP_5" ]]; then
  if [[ -z "$SKIP_NTRU" ]]; then
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_clang_ref-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hrss701_clang_avx2-optimized" $NTRU_REGIONS

    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_clang_ref-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2-optimized" $NTRU_REGIONS
    plan micro_benchmark_kem "./ntru/build/ntru_hps4096821_clang_avx2-optimized" $NTRU_REGIONS
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_avx2-optimized" $MCELIECE_REGIONS

    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_avx2-optimized" $MCELIECE_REGIONS

    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_avx2-optimized" $MCELIECE_REGIONS

    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_ref-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2-optimized" $MCELIECE_REGIONS
    plan micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_avx2-optimized" $MCELIECE_REGIONS
  fi

  run_jobs micro_benchmark_kem
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 6 - Sequential Benchmarks ==="
if [[ -z "$SKIP_STEP_6" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    plan sequential_benchmark_kex "./ecdh/build/ecdh_25519_gcc_plain-optimized"

    plan sequential_benchmark_kex "./ecdh/build/ecdh_p256_gcc_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    plan sequential_benchmark_kex "./dh/build/dh_gcc_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref"
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_gcc_ref-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_clang_ref-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2"
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hrss701_clang_avx2-optimized"

    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref"
    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_ref-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_clang_ref-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2"
    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./ntru/build/ntru_hps4096821_clang_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_clang_avx2-optimized"

    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_clang_avx2-optimized"

    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_clang_avx2-optimized"

    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_ref-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2-optimized"
    plan sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_clang_avx2-optimized"
  fi

  run_jobs sequential_benchmark_kex sequential_benchmark_kem
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 7 - Parallel Benchmarks ==="
if [[ -z "$SKIP_STEP_7" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    # TODO: Select best implementation
    plan parallel_benchmark_kex "./ecdh/build/ecdh_25519_gcc_plain-optimized"

    # TODO: Select best implementation
    plan parallel_benchmark_kex "./ecdh/build/ecdh_p256_gcc_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    # TODO: Select best implementation
    plan parallel_benchmark_kex "./dh/build/dh_gcc_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    # TODO: Select best implementation
    plan parallel_benchmark_kem "./ntru/build/ntru_hrss701_gcc_avx2-optimized"

    # TODO: Select best implementation
    plan parallel_benchmark_kem "./ntru/build/ntru_hps4096821_gcc_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    # TODO: Select best implementation
    plan parallel_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_gcc_avx2-optimized"

    # TODO: Select best implementation
    plan parallel_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_gcc_avx2-optimized"
  fi

  run_jobs parallel_benchmark_kex parallel_benchmark_kem
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 8 - Calculate Stack Usage ==="
if [[ -z "$SKIP_STEP_8" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    plan benchmark_stack "./ecdh/build/ecdh_25519_gcc_plain-optimized"
    plan benchmark_stack "./ecdh/build/ecdh_25519_clang_plain-optimized"

    plan benchmark_stack "./ecdh/build/ecdh_p256_gcc_plain-optimized"
    plan benchmark_stack "./ecdh/build/ecdh_p256_clang_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    plan benchmark_stack "./dh/build/dh_gcc_plain-optimized"
    plan benchmark_stack "./dh/build/dh_clang_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    plan benchmark_stack "./ntru/build/ntru_hrss701_gcc_ref"
    plan benchmark_stack "./ntru/build/ntru_hrss701_gcc_ref-optimized"
    plan benchmark_stack "./ntru/build/ntru_hrss701_clang_ref-optimized"
    plan benchmark_stack "./ntru/build/ntru_hrss701_gcc_avx2"
    plan benchmark_stack "./ntru/build/ntru_hrss701_gcc_avx2-optimized"
    plan benchmark_stack "./ntru/build/ntru_hrss701_clang_avx2-optimized"

    plan benchmark_stack "./ntru/build/ntru_hps4096821_gcc_ref"
    plan benchmark_stack "./ntru/build/ntru_hps4096821_gcc_ref-optimized"
    plan benchmark_stack "./ntru/build/ntru_hps4096821_clang_ref-optimized"
    plan benchmark_stack "./ntru/build/ntru_hps4096821_gcc_avx2"
    plan benchmark_stack "./ntru/build/ntru_hps4096821_gcc_avx2-optimized"
    plan benchmark_stack "./ntru/build/ntru_hps4096821_clang_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_gcc_ref"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_gcc_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_clang_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_gcc_avx2"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_gcc_avx2-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119_clang_avx2-optimized"

    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_gcc_ref"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_gcc_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_clang_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_gcc_avx2"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_gcc_avx2-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_6960119f_clang_avx2-optimized"

    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_gcc_ref"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_gcc_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_clang_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_gcc_avx2"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_gcc_avx2-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128_clang_avx2-optimized"

    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_gcc_ref"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_gcc_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_clang_ref-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_gcc_avx2"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_gcc_avx2-optimized"
    plan benchmark_stack "./classic-mceliece/build/mceliece_8192128f_clang_avx2-optimized"
  fi

  run_jobs benchmark_stack
else
  echo "=== skipped ==="
fi
echo ""
