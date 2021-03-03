.PHONY: all tests benchmarks test clean

common_source := $(shell find . -maxdepth 1 -type f -iname "*.c" -not -iname "crypto_sort_int32.c" -not -iname "test.c")
common_headers := $(shell find . -maxdepth 1 -type f -iname "*.h" -not -iname "crypto_sort_int32.h")

# Source code of the reference implementation of the polynomal library
reference_poly_source := $(shell find poly/ref -type f -iname "*.c") $(shell find poly/common -type f -iname "*.c")

# Base source code of the AVX2 implementation of the polynomial library
avx2_poly_source := $(shell find poly/avx2 -maxdepth 1 -type f -iname "*.c" -or -iname "*.s") $(shell find poly/common -type f -iname "*.c")

# Source code of the AVX2 implementation of the polynomial libraries for the parameter sets
avx2_poly_source_hps701 := $(avx2_poly_source) $(shell find poly/avx2/701 -type f -iname "*.c" -or -iname "*.s")
avx2_poly_source_hrss4096821 := $(avx2_poly_source) $(shell find poly/avx2/4096821 -type f -iname "*.c" -or -iname "*.s")

# Source code of the reference implementation of the sampling library
reference_sample_source := $(shell find sample/common -type f -iname "*.c")

# Source code for the AVX2 implementation of the sample library for the parameter sets
avx2_sample_source_hps701 := $(shell find sample/common -type f -iname "*.c") sample/avx2/701/vec32_sample_iid.s
avx2_sample_source_hrss4096821 := $(shell find sample/common -type f -iname "*.c") sample/avx2/4096821/vec32_sample_iid.s

all: tests benchmarks test

tests: build/ntru_hrss701_ref_test build/ntru_hps4096821_ref_test build/ntru_hrss701_avx_test build/ntru_hps4096821_avx_test

benchmarks:

build/ntru_hrss701_ref_test: test.c $(reference_sample_source) $(reference_poly_source) $(common_source) $(common_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HRSS_701 test.c $(reference_sample_source) $(reference_poly_source) $(common_source) -lcrypto

build/ntru_hrss701_avx_test: test.c $(avx2_sample_source_hps701) $(avx2_poly_source_hps701) $(common_source) $(common_headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HRSS_701 -D USE_AVX2 -mavx2 test.c $(avx2_sample_source_hps701) $(avx2_poly_source_hps701) $(common_source) -lcrypto

build/ntru_hps4096821_ref_test: test.c $(reference_sample_source) $(reference_poly_source) $(common_source) $(common_headers) crypto_sort_int32.c crypto_sort_int32.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HPS_4096821 test.c $(reference_sample_source) $(reference_poly_source) $(common_source) $(common_headers) crypto_sort_int32.c -lcrypto

build/ntru_hps4096821_avx_test: test.c $(avx2_sample_source_hrss4096821) $(avx2_poly_source_hrss4096821) $(common_source) $(common_headers) crypto_sort_int32.c crypto_sort_int32.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ -D NTRU_PARAMETER_SET=NTRU_HPS_4096821 -D USE_AVX2 -mavx2 test.c $(avx2_sample_source_hrss4096821) $(avx2_poly_source_hrss4096821) $(common_source)  crypto_sort_int32.c -lcrypto

test: tests
	build/ntru_hrss701_ref_test
	build/ntru_hps4096821_ref_test
	bash -c 'diff <(build/ntru_hrss701_ref_test) <(build/ntru_hrss701_avx_test)'
	bash -c 'diff <(build/ntru_hps4096821_ref_test) <(build/ntru_hps4096821_avx_test)'

clean:
	rm -rf build &> /dev/null || true
