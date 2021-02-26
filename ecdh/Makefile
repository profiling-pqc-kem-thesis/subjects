.PHONY: build clean

OPENSSL ?= /usr/local/Cellar/openssl@1.1/1.1.1i

CCFLAGS += -I $(OPENSSL)/include -L $(OPENSSL)/lib

all: tests benchmarks

tests: build/ecdh_p256_test build/ecdh_25519_test

benchmarks: build/ecdh_p256_plain_optimized build/ecdh_25519_plain_optimized

build/ecdh_p256_plain_optimized: benchmark.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 benchmark.c dh.c -l crypto

build/ecdh_25519_plain_optimized: benchmark.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -O3 -march=native -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 benchmark.c dh.c -l crypto

build/ecdh_p256_test: test.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 test.c dh.c -l crypto

build/ecdh_25519_test: test.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 test.c dh.c -l crypto

clean:
	rm -rf build &> /dev/null || true
