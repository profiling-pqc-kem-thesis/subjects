.PHONY: build clean

OPENSSL ?= /usr/local/Cellar/openssl@1.1/1.1.1i

CCFLAGS += -I $(OPENSSL)/include -L $(OPENSSL)/lib -O3 -march=native -pipe

build: build/ecdh_p256_plain_optimized build/ecdh_25519_plain_optimized

build/ecdh_p256_plain_optimized: main.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_P256 main.c dh.c -l crypto

build/ecdh_25519_plain_optimized: main.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ -D ECDH_CURVE=ECDH_CURVE_25519 main.c dh.c -l crypto

clean:
	rm -rf build &> /dev/null || true
