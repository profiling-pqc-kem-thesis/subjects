.PHONY: build clean

OPENSSL ?= /usr/local/Cellar/openssl@1.1/1.1.1i

CCFLAGS += -I $(OPENSSL)/include -L $(OPENSSL)/lib

build: build/ecdh

build/ecdh: main.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ main.c dh.c -l crypto

clean:
	rm -rf build &> /dev/null || true
