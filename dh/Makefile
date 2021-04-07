.PHONY: all tests benchmarks test benchmark clean

LDLIBS += -l crypto

source := lib/dh.c
headers := lib/api.h lib/dh.h lib/params.h

benchmark_source := $(source) benchmark.c ../utilities/benchmark.c ../utilities/benchmark-sequential.c ../utilities/benchmark-parallel.c ../utilities/time.c ../utilities/cpu.c ../utilities/pool.c
benchmark_headers := $(headers) ../utilities/benchmark.h ../utilities/time.h ../utilities/cpu.h ../utilities/pool.h

test_source := $(source) test.c
test_headers := $(headers)

all: tests benchmarks

tests: build/dh_test

benchmarks: build/dh_gcc_plain-optimized build/dh_clang_plain-optimized

build/dh_gcc_plain-optimized: $(benchmark_source) $(benchmark_headers)
	mkdir -p $(dir $@)
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -mtune=native -o $@ -D BENCHMARK_SUBJECT_NAME='"DH"' -D BENCHMARK_KEYPAIR -D BENCHMARK_EXCHANGE $(benchmark_source) $(LDLIBS) -lpthread

build/dh_clang_plain-optimized: $(benchmark_source) $(benchmark_headers)
	mkdir -p $(dir $@)
	clang $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -mtune=native -o $@ -D BENCHMARK_SUBJECT_NAME='"DH"' -D BENCHMARK_KEYPAIR -D BENCHMARK_EXCHANGE $(benchmark_source) $(LDLIBS) -lpthread

build/dh_test: $(test_source) $(test_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -mtune=native -o $@ $(test_source) $(LDLIBS)

test: tests
	build/dh_test

clean:
	rm -rf build &> /dev/null || true
