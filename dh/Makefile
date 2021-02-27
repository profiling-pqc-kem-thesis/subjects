.PHONY: build clean

OPENSSL ?= /usr/local/Cellar/openssl@1.1/1.1.1i

CCFLAGS += -I $(OPENSSL)/include -L $(OPENSSL)/lib

build: build/dh

build/dh: main.c dh.c dh.h params.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -g3 -o $@ main.c dh.c -l crypto # TODO: remove -g3 flag

clean:
	rm -rf build &> /dev/null || true
