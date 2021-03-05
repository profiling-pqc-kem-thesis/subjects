.PHONY: all tests benchmarks test benchmark clean

all: tests benchmarks

tests: build/dh_test

benchmarks: build/dh_plain_optimized

build/dh_plain_optimized: benchmark.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ benchmark.c lib/dh.c -l crypto

build/dh_test: test.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ test.c lib/dh.c -l crypto

test:
	build/dh_test

benchmark: benchmarks
	build/dh_plain_optimized

clean:
	rm -rf build &> /dev/null || true
