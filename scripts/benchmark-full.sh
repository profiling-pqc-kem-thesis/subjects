#!/usr/bin/env bash

set -e

DIRECTORY=$(dirname $(realpath -s $0))

SKIP_STEPS="${SKIP_STEPS:-}"

while read -r step; do
    eval "SKIP_STEP_$step=1"
done <<<"$(echo "$SKIP_STEPS" | tr ',' '\n')"

environment_name="$1"

if [[ -z "$environment_name" ]]; then
  echo "usage: $0 <environment name>"
  exit 1
fi

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
  shift
  methods="$(echo "$@" | sed 's/ / -r /g')"
  perforator --summary --csv -e cpu-cycles,instructions $methods -- "$binary" --sequential --keypair --iterations 1000 2>&1 | tee "$output_directory/micro/$(basename "$binary").keypair.txt"
  perforator --summary --csv -e cpu-cycles,instructions $methods -- "$binary" --sequential --encrypt --iterations 1000 2>&1 | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"
  perforator --summary --csv -e cpu-cycles,instructions $methods -- "$binary" --sequential --decrypt --iterations 1000 2>&1 | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
}

function sequential_benchmark_kex() {
  binary="$1"
  "$binary" --sequential --keypair --iterations 1000 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
  "$binary" --sequential --exchange --iterations 1000 2>&1 | tee "$output_directory/sequential/$(basename "$binary").exchange.txt"
}

function sequential_benchmark_kem() {
  binary="$1"
  "$binary" --sequential --keypair --iterations 1000 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
  "$binary" --sequential --encrypt --iterations 1000 2>&1 | tee "$output_directory/sequential/$(basename "$binary").encrypt.txt"
  "$binary" --sequential --decrypt --iterations 1000 2>&1 | tee "$output_directory/sequential/$(basename "$binary").decrypt§.txt"
}

function get_max_thread_count() {
  if [[ "$(uname)" = "Darwin" ]]; then
    cores="$(sysctl machdep.cpu.core_count | cut -d ':' -f2)"
    threads="$(sysctl machdep.cpu.thread_count | cut -d ':' -f2)"
    smt="$((threads/cores))"
    echo "$((cores*smt*4))"
  else
    cores="$(lscpu | grep '^CPU(s):' | cut -d':' -f2 | sed 's/[ \t]//g')"
    smt="$(lscpu | grep '^Thread(s)' | cut -d':' -f2 | sed 's/[ \t]//g')"
    echo "$((cores*smt*4))"
  fi
}

function parallel_benchmark_kex() {
  binary="$1"
  max_thread_count="$(get_max_thread_count)"
  thread_count=1
  while true; do
    "$binary" --parallel --keypair --duration 5 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
    "$binary" --parallel --exchange --duration 5 2>&1 | tee "$output_directory/parallel/$(basename "$binary").exchange.$thread_count.txt"
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
    "$binary" --parallel --keypair --duration 5 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
    "$binary" --parallel --encrypt --duration 5 2>&1 | tee "$output_directory/parallel/$(basename "$binary").encrypt.$thread_count.txt"
    "$binary" --parallel --decrypt --duration 5 2>&1 | tee "$output_directory/parallel/$(basename "$binary").decrypt.$thread_count.txt"
    thread_count=$((thread_count*2))
    if [[ $thread_count -gt $max_thread_count ]]; then
      break
    fi
  done
}

function heap_benchmark_kex() {
  binary="$1"
  heaptrack "$binary" --sequential --keypair --iterations 1000 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
  heaptrack "$binary" --sequential --exchange --iterations 1000 2>&1 | tee "$output_directory/heap/$(basename "$binary").exchange.txt"
}

function heap_benchmark_kem() {
  binary="$1"
  heaptrack "$binary" --sequential --keypair --iterations 1000 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
  heaptrack "$binary" --sequential --encrypt --iterations 1000 2>&1 | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"
  heaptrack "$binary" --sequential --decrypt --iterations 1000 2>&1 | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"
}

output_directory="data/benchmarks/$environment_name"

if [[ -d "$output_directory" ]]; then
  echo -n "Continuing overwrite contents of '$output_directory'. Press any key to continue: "
  read -rn 1
  echo ""
  echo -n "Are you sure? Press any key to continue: "
  read -rn 1
  echo ""
fi

mkdir -p "$output_directory"
mkdir -p "$output_directory/micro"
mkdir -p "$output_directory/heap"
mkdir -p "$output_directory/sequential"
mkdir -p "$output_directory/parallel"

echo -n "Benchmark started on: "
LANG=en_US date | tee "$output_directory/start.txt"
start_time="$(date '+%s')"
echo ""

echo "=== STEP 1 - Checking Prerequisites ==="
if [[ -z "$SKIP_STEP_1" ]]; then
  assert_commands "sqlite3" "heaptrack" "make" "gcc" "clang" "go" "perforator"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 2 - Build Benchmarks ==="
if [[ -z "$SKIP_STEP_2" ]]; then
  VERBOSE=yes make benchmarks 2>&1 | tee "$output_directory/build.txt"
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
  heap_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"
  heap_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"

  heap_benchmark_kex "./dh/build/dh_plain-optimized"

  heap_benchmark_kem "./ntru/build/ntru_hrss701_ref"
  heap_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized"
  heap_benchmark_kem "./ntru/build/ntru_hrss701_avx2"
  heap_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

  heap_benchmark_kem "./ntru/build/ntru_hps4096821_ref"
  heap_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized"
  heap_benchmark_kem "./ntru/build/ntru_hps4096821_avx2"
  heap_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"

  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref-optimized"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2-optimized"

  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref-optimized"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2-optimized"

  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized"

  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2"
  heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 5 - Micro Benchmarks ==="
if [[ -z "$SKIP_STEP_5" ]]; then
  micro_benchmark_kem "./ntru/build/ntru_hrss701_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hrss701_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  micro_benchmark_kem "./ntru/build/ntru_hps4096821_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hps4096821_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2_optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2" crypto_kem_keypair crypto_kem_enc crypto_kem_dec
  micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized" crypto_kem_keypair crypto_kem_enc crypto_kem_dec

  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 6 - Sequential Benchmarks ==="
if [[ -z "$SKIP_STEP_6" ]]; then
  sequential_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"
  sequential_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"

  sequential_benchmark_kex "./dh/build/dh_plain-optimized
"
  sequential_benchmark_kem "./ntru/build/ntru_hrss701_ref"
  sequential_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized"
  sequential_benchmark_kem "./ntru/build/ntru_hrss701_avx2"
  sequential_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

  sequential_benchmark_kem "./ntru/build/ntru_hps4096821_ref"
  sequential_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized"
  sequential_benchmark_kem "./ntru/build/ntru_hps4096821_avx2"
  sequential_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"

  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119_ref-optimized"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119_avx2-optimized"

  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119f_ref-optimized"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_690119f_avx2-optimized"

  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized"

  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2"
  sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 7 - Parallel Benchmarks ==="
if [[ -z "$SKIP_STEP_7" ]]; then
  parallel_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"

  parallel_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"

  # TODO: Select best implementation
  parallel_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

  # TODO: Select best implementation
  parallel_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"

  # TODO: Select best implementation
  parallel_benchmark_kem "./classic-mceliece/build/mceliece_69601196f_avx2-optimized"

  # TODO: Select best implementation
  parallel_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo -n "Benchmark ended on: "
LANG=en_US date | tee "$output_directory/end.txt"
end_time="$(date '+%s')"
duration="$((end_time-start_time))"
echo -n "Duration: "
printf '%dh %dm %ds\n' $((duration/3600)) $((duration%3600/60)) $((duration%60)) | tee "$output_directory/duration.txt"
