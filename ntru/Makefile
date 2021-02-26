.PHONY: all tests clean

common_source := $(shell find . -type f -iname "*.c" -not -iname "crypto_sort_int32.c" -not -iname "test.c")
common_headers := $(shell find . -type f -iname "*.h" -not -iname "crypto_sort_int32.h")

all: tests benchmarks

tests: build/ntru_hrss701_test build/ntru_hps4096821_test

benchmarks:

build/ntru_hrss701_test: test.c $(common_source) $(common_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HRSS_701 test.c $(common_source) -lcrypto

build/ntru_hps4096821_test: test.c $(common_source) crypto_sort_int32.c $(common_headers) crypto_sort_int32.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HPS_4096821 test.c $(common_source) crypto_sort_int32.c -lcrypto

clean:
	rm -rf build &> /dev/null || true
