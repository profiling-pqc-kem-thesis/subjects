.PHONY: all tests benchmarks test benchmark clean

all: tests benchmarks test

tests: build/dh_test

benchmarks: build/dh_plain_optimized

build/dh_plain_optimized: benchmark.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ benchmark.c dh.c -l crypto

build/dh_test: test.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ test.c dh.c -l crypto

test:
	build/dh_test

benchmark: benchmarks
	build/dh_plain_optimized

clean:
	rm -rf build &> /dev/null || true
