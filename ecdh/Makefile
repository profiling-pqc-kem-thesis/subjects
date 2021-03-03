.PHONY: all tests benchmarks test benchmark clean

all: tests benchmarks test

tests: build/ecdh_p256_test build/ecdh_25519_test

benchmarks: build/ecdh_p256_plain_optimized build/ecdh_25519_plain_optimized

build/ecdh_p256_plain_optimized: benchmark.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 benchmark.c dh.c -l crypto

build/ecdh_25519_plain_optimized: benchmark.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 benchmark.c dh.c -l crypto

build/ecdh_p256_test: test.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 test.c dh.c -l crypto

build/ecdh_25519_test: test.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 test.c dh.c -l crypto

test:
	build/ecdh_p256_test
	build/ecdh_25519_test

benchmark: benchmarks
	build/ecdh_p256_plain_optimized
	build/ecdh_25519_plain_optimized

clean:
	rm -rf build &> /dev/null || true
