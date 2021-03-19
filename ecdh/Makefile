.PHONY: all tests benchmarks test benchmark clean

LDLIBS += -l crypto

source := lib/dh.c
headers := lib/api.h lib/dh.h lib/params.h

benchmark_source := $(source) benchmark.c ../utilities/benchmark.c ../utilities/benchmark-sequential.c ../utilities/benchmark-parallel.c ../utilities/time.c ../utilities/cpu.c ../utilities/pool.c
benchmark_headers := $(headers) ../utilities/benchmark.h ../utilities/time.h ../utilities/cpu.h ../utilities/pool.h

test_source := $(source) test.c
test_headers := $(headers)

all: tests benchmarks

tests: build/ecdh_p256_test build/ecdh_25519_test

benchmarks: build/ecdh_p256_plain_optimized build/ecdh_25519_plain_optimized

build/ecdh_p256_plain_optimized: $(benchmark_source) $(benchmark_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 -D BENCHMARK_SUBJECT_NAME='"ECDHE P-256"' -D BENCHMARK_KEYPAIR -D BENCHMARK_EXCHANGE $(benchmark_source) $(LDLIBS) -lpthread

build/ecdh_25519_plain_optimized: $(benchmark_source) $(benchmark_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 -D BENCHMARK_SUBJECT_NAME='"ECDHE x25519"' -D BENCHMARK_KEYPAIR -D BENCHMARK_EXCHANGE $(benchmark_source) $(LDLIBS) -lpthread

build/ecdh_p256_test: $(test_source) $(test_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 $(test_source) $(LDLIBS)

build/ecdh_25519_test: $(test_source) $(test_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 $(test_source) $(LDLIBS)

test: tests
	build/ecdh_p256_test
	build/ecdh_25519_test

benchmark: benchmarks
	build/ecdh_p256_plain_optimized --sequential
	build/ecdh_25519_plain_optimized --sequential

clean:
	rm -rf build &> /dev/null || true
