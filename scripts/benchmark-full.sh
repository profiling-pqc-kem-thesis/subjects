#!/usr/bin/env bash

DIRECTORY=$(dirname $(realpath -s $0))

SKIP_STEPS="${SKIP_STEPS:-}"

TIMEOUT="${TIMEOUT:-3600}"
SEQUENTIAL_ITERATIONS="${SEQUENTIAL_ITEARTIONS:-1000}"
PARALLEL_ITERATIONS="${SEQUENTIAL_ITEARTIONS:-100000}"

SLEEP_BETWEEN_TESTS=${SLEEP_BETWEEN_TESTS:-1}

while read -r step; do
    eval "SKIP_STEP_$step=1"
done <<<"$(echo "$SKIP_STEPS" | tr ',' '\n')"

environment_name="$1"

if [[ -z "$environment_name" ]]; then
  echo "usage: $0 <environment name>"
  exit 1
fi

function wait_for_cooldown() {
  echo "Waiting ${SLEEP_BETWEEN_TESTS}s for cooldown"
  sleep "$SLEEP_BETWEEN_TESTS"
}

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
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").keypair.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
    return
  fi

  keypair_methods="$(echo "$2" | sed 's/^\| / -r /g')"
  perforator --summary --csv -e cpu-cycles,instructions $keypair_methods -- "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").keypair.txt"
  wait_for_cooldown

  encrypt_methods="$(echo "$3" | sed 's/^\| / -r /g')"
  perforator --summary --csv -e cpu-cycles,instructions $encrypt_methods -- "$binary" sequential --encrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").encrypt.txt"
  wait_for_cooldown

  decrypt_methods="$(echo "$4" | sed 's/^\| / -r /g')"
  perforator --summary --csv -e cpu-cycles,instructions $decrypt_methods -- "$binary" sequential --decrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/micro/$(basename "$binary").decrypt.txt"
  wait_for_cooldown
}

function sequential_benchmark_kex() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").exchange.txt"
    return
  fi

  "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
  wait_for_cooldown

  "$binary" sequential --exchange --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").exchange.txt"
  wait_for_cooldown
}

function sequential_benchmark_kem() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").encrypt.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/sequential/$(basename "$binary").decrypt.txt"
    return
  fi

  "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").keypair.txt"
  wait_for_cooldown

  "$binary" sequential --encrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").encrypt.txt"
  wait_for_cooldown

  "$binary" sequential --decrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/sequential/$(basename "$binary").decrypt.txt"
  wait_for_cooldown
}

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
      echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").exchange.$thread_count.txt"
      continue
    else
      "$binary" parallel --keypair --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      wait_for_cooldown

      "$binary" parallel --exchange --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").exchange.$thread_count.txt"
      wait_for_cooldown
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
      echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").encrypt.$thread_count.txt"
      echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/parallel/$(basename "$binary").decrypt.$thread_count.txt"
      continue
    else
      "$binary" parallel --keypair --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").keypair.$thread_count.txt"
      wait_for_cooldown

      "$binary" parallel --encrypt --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").encrypt.$thread_count.txt"
      wait_for_cooldown

      "$binary" parallel --decrypt --iterations "$PARALLEL_ITERATIONS" --timeout "$TIMEOUT" --thread-count "$thread_count" 2>&1 | tee "$output_directory/parallel/$(basename "$binary").decrypt.$thread_count.txt"
      wait_for_cooldown
    fi

    thread_count=$((thread_count*2))
    if [[ $thread_count -gt $max_thread_count ]]; then
      break
    fi
  done
}

function heap_benchmark_kex() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").exchange.txt"
    return
  fi

  heaptrack "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
  wait_for_cooldown

  heaptrack "$binary" sequential --exchange --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").exchange.txt"
  wait_for_cooldown

  mv heaptrack.* "$output_directory/heap"
}

function heap_benchmark_kem() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"
    return
  fi

  heaptrack "$binary" sequential --keypair --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").keypair.txt"
  wait_for_cooldown

  heaptrack "$binary" sequential --encrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").encrypt.txt"
  wait_for_cooldown

  heaptrack "$binary" sequential --decrypt --iterations "$SEQUENTIAL_ITERATIONS" --timeout "$TIMEOUT" 2>&1 | tee "$output_directory/heap/$(basename "$binary").decrypt.txt"
  wait_for_cooldown

  mv heaptrack.* "$output_directory/heap"
}

function benchmark_stack() {
  binary="$1"
  if [[ ! -f "$binary" ]]; then
    echo "warning: skipping benchmark '$binary' - no such file" | tee "$output_directory/stack/$(basename "$binary").txt"
    return
  fi

  bash "$DIRECTORY/calculate-stack-usage-of-object.sh" "$binary" | tee "$output_directory/stack/$(basename "$binary").txt"
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
mkdir -p "$output_directory/stack"
cat "$0" > "$output_directory/benchmark-full.sh"

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
  if [[ -z "$SKIP_ECDH" ]]; then
    heap_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"
    heap_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    heap_benchmark_kex "./dh/build/dh_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    heap_benchmark_kem "./ntru/build/ntru_hrss701_ref"
    heap_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized"
    heap_benchmark_kem "./ntru/build/ntru_hrss701_avx2"
    heap_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

    heap_benchmark_kem "./ntru/build/ntru_hps4096821_ref"
    heap_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized"
    heap_benchmark_kem "./ntru/build/ntru_hps4096821_avx2"
    heap_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref-optimized"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2-optimized"

    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref-optimized"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2-optimized"

    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized"

    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2"
    heap_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  fi
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 5 - Micro Benchmarks ==="
if [[ -z "$SKIP_STEP_5" ]]; then
  if [[ -z "$SKIP_NTRU" ]]; then
    micro_benchmark_kem "./ntru/build/ntru_hrss701_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hrss701_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"

    micro_benchmark_kem "./ntru/build/ntru_hps4096821_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hps4096821_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"

    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2_optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"

    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"

    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
    micro_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized" "crypto_kem_keypair" "crypto_kem_enc" "crypto_kem_dec"
  fi

  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 6 - Sequential Benchmarks ==="
if [[ -z "$SKIP_STEP_6" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    benchmark_stack "./ecdh/build/ecdh_25519_plain-optimized"

    benchmark_stack "./ecdh/build/ecdh_p256_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    benchmark_stack "./dh/build/dh_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    benchmark_stack "./ntru/build/ntru_hrss701_ref"
    benchmark_stack "./ntru/build/ntru_hrss701_ref-optimized"
    benchmark_stack "./ntru/build/ntru_hrss701_avx2"
    benchmark_stack "./ntru/build/ntru_hrss701_avx2-optimized"

    benchmark_stack "./ntru/build/ntru_hps4096821_ref"
    benchmark_stack "./ntru/build/ntru_hps4096821_ref-optimized"
    benchmark_stack "./ntru/build/ntru_hps4096821_avx2"
    benchmark_stack "./ntru/build/ntru_hps4096821_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    benchmark_stack "./classic-mceliece/build/mceliece_6960119_ref"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119_ref-optimized"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119_avx2"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119_avx2-optimized"

    benchmark_stack "./classic-mceliece/build/mceliece_6960119f_ref"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119f_ref-optimized"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119f_avx2"
    benchmark_stack "./classic-mceliece/build/mceliece_6960119f_avx2-optimized"

    benchmark_stack "./classic-mceliece/build/mceliece_8192128_ref"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128_ref-optimized"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128_avx2"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128_avx2-optimized"

    benchmark_stack "./classic-mceliece/build/mceliece_8192128f_ref"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128f_ref-optimized"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128f_avx2"
    benchmark_stack "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  fi
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 7 - Parallel Benchmarks ==="
if [[ -z "$SKIP_STEP_7" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    parallel_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"

    parallel_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    parallel_benchmark_kex "./dh/build/dh_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    # TODO: Select best implementation
    parallel_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

    # TODO: Select best implementation
    parallel_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    # TODO: Select best implementation
    parallel_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2-optimized"

    # TODO: Select best implementation
    parallel_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  fi
  echo "=== done ==="
else
  echo "=== skipped ==="
fi
echo ""

echo "=== STEP 8 - Calculate Stack Usage ==="
if [[ -z "$SKIP_STEP_8" ]]; then
  if [[ -z "$SKIP_ECDH" ]]; then
    sequential_benchmark_kex "./ecdh/build/ecdh_25519_plain-optimized"

    sequential_benchmark_kex "./ecdh/build/ecdh_p256_plain-optimized"
  fi

  if [[ -z "$SKIP_DH" ]]; then
    sequential_benchmark_kex "./dh/build/dh_plain-optimized"
  fi

  if [[ -z "$SKIP_NTRU" ]]; then
    sequential_benchmark_kem "./ntru/build/ntru_hrss701_ref"
    sequential_benchmark_kem "./ntru/build/ntru_hrss701_ref-optimized"
    sequential_benchmark_kem "./ntru/build/ntru_hrss701_avx2"
    sequential_benchmark_kem "./ntru/build/ntru_hrss701_avx2-optimized"

    sequential_benchmark_kem "./ntru/build/ntru_hps4096821_ref"
    sequential_benchmark_kem "./ntru/build/ntru_hps4096821_ref-optimized"
    sequential_benchmark_kem "./ntru/build/ntru_hps4096821_avx2"
    sequential_benchmark_kem "./ntru/build/ntru_hps4096821_avx2-optimized"
  fi

  if [[ -z "$SKIP_MCELIECE" ]]; then
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_ref-optimized"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119_avx2-optimized"

    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_ref-optimized"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_6960119f_avx2-optimized"

    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_ref-optimized"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128_avx2-optimized"

    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_ref-optimized"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2"
    sequential_benchmark_kem "./classic-mceliece/build/mceliece_8192128f_avx2-optimized"
  fi
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
