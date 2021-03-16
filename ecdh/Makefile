.PHONY: all tests benchmarks test benchmark clean

all: tests benchmarks

tests: build/ecdh_p256_test build/ecdh_25519_test

benchmarks: build/ecdh_p256_plain_optimized build/ecdh_25519_plain_optimized

build/ecdh_p256_plain_optimized: benchmark.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 benchmark.c lib/dh.c -l crypto

build/ecdh_25519_plain_optimized: benchmark.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 benchmark.c lib/dh.c -l crypto

build/ecdh_p256_test: test.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 test.c lib/dh.c -l crypto

build/ecdh_25519_test: test.c lib/dh.c lib/dh.h lib/params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 test.c lib/dh.c -l crypto

test: tests
	build/ecdh_p256_test
	build/ecdh_25519_test

benchmark: benchmarks
	build/ecdh_p256_plain_optimized
	build/ecdh_25519_plain_optimized

clean:
	rm -rf build &> /dev/null || true
