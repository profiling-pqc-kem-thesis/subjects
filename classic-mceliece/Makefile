.PHONY: all tests benchmarks test target clean

# The build type, 'none', 'test' or 'benchmark'
TARGET ?= none

# The group to build, '8192128' or '6960119'
GROUP ?=

# The parameters, such as 'f'
PARAMETERS ?=

# Whether or not (true/false) to use AVX2
USE_AVX2 ?= false

# Whether or not (true/false) to use optimization flags
USE_OPTIMIZATION_FLAGS ?= false

LDLIBS += -lcrypto -lkeccak

# If the target is not none, define all required parameters for building a target
ifneq ($(TARGET),none)
	# The name of the parameter set, such as hrss701
	parameter_set := $(GROUP)

	# Add common source and headers
	source := $(shell find * -maxdepth 1 -type f -iname "*.c" -not -iname "test.c" -not -iname "benchmark.c")
	headers := $(shell find * -maxdepth 1 -type f -iname "*.h" -not -iname "crypto_kem_mceliece8192128.h" -not -iname "crypto_kem_mceliece6960119.h")
	headers += subroutines/crypto_int32.h
	# Add common poly and sample source
	#source += $(shell find poly/common -type f -iname "*.c")
	#source += $(shell find sample/common -type f -iname "*.c")

	ifeq ($(shell echo $(PARAMETERS) | grep -q 'f' && echo "true" || echo "false"),true)
		CPPFLAGS += -DUSE_F
	endif

	ifeq ($(USE_AVX2),true)
		CPPFLAGS += -DUSE_KECCAK -I ../xkcp/build/AVX2/keccak
		LDFLAGS += -L ../xkcp/build/AVX2/keccak
	else
		CPPFLAGS += -DUSE_KECCAK -I ../xkcp/build/plain-64bitsua/keccak
		LDFLAGS += -L ../xkcp/build/plain-64bitsua/keccak
	endif

	ifeq ($(USE_AVX2),true)
		# Add the target-specific AVX2 poly implementation
		source += $(shell find poly/avx2 -maxdepth 1 -type f -iname "*.c" -or -iname "*.s")
		source += $(shell find poly/avx2/$(PARAMETERS) -type f -iname "*.c" -or -iname "*.s")
		headers += poly/avx2/poly_r2_inv.h
		# Add the target-specific AVX2 sample implementation
		source += sample/avx2/$(PARAMETERS)/vec32_sample_iid.s
		CFLAGS += -mavx2
		CPPFLAGS += -DUSE_AVX2
		features += avx2
	else
		# Add the reference poly implementation
		#source += $(shell find poly/ref -type f -iname "*.c")
	endif

	# Add source only used by the HPS family
	#ifeq ($(GROUP),hps)
	#	source += crypto_sort_int32.c
	#	headers += crypto_sort_int32.h
	#endif

	ifeq ($(USE_OPTIMIZATION_FLAGS),true)
		CFLAGS += -march=native
		features += optimized
	endif

	# If no features are enabled, use "ref" for "reference"
	ifeq ($(features),)
		features := ref
	endif

	# Define the target name like so:
	# build/ntru_hrss701_keccak-avx2-optimized
	target_name := build/mceliece_$(PARAMETERS)_$(shell echo "$(features)" | tr ' ' '-')

	# Set the main target to either test or benchmark
	ifeq ($(TARGET),benchmark)
		source += benchmark.c
		target_name := $(target_name)_test
	else ifeq ($(TARGET),test)
		source += test.c
		target_name := $(target_name)_test
	endif

	# Format the constant for controlling the implementation. Will look like
	# NTRU_HPS_2048677. Possible values are defined in params.h
	MCELIECE_PARAMETER_SET := MCELIECE_$(PARAMETERS)
	CPPFLAGS += -D MCELIECE_PARAMETER_SET=$(MCELIECE_PARAMETER_SET)
endif

# This macro creates a build target for a configuration of NTUR
define create-target
$(strip $(1)):
	TARGET=$(strip $(2)) PARAMETERS=$(strip $(3)) USE_AVX2=$(strip $(4)) USE_OPTIMIZATION_FLAGS=$(strip $(5)) $$(MAKE) target
endef

all: tests benchmarks

# Build all tests
# first line is HRS701 targets, the second HPS4096821
#tests: mceliece_8192128_ref_test mceliece_8192128_avx2n_test \
#	mceliece_8192128f_ref_test mceliece_8192128f_avx2_test

tests:  mceliece_8192128_ref_test mceliece_8192128f_ref_test mceliece_6960119_ref_test mceliece_6960119f_ref_test

# Build all benchmarks
# first line is HRS701 targets, the second HPS4096821
#benchmarks: mceliece_8192128_ref mceliece_8192128_avx2 mceliece_8192128_ref_optimized mceliece_8192128_avx2_optimized \
#	mceliece_8192128f_ref mceliece_8192128f_avx2 mceliece_8192128f_ref_optimized mceliece_8192128f_avx2_optimized

benchmarks: mceliece_8192128_ref mceliece_8192128_ref_optimized	mceliece_8192128f_ref mceliece_8192128f_ref_optimized

# Define all HRSS701 targets
#                            rule name                         target     parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_8192128_ref,             benchmark, 8192128,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2,           benchmark, 8192128,       true,  false))
$(eval $(call create-target, mceliece_8192128_ref_optimized,   benchmark, 8192128,       false, true))
#$(eval $(call create-target, mceliece_8192128_avx2_optimized, benchmark, 8192128,       true,  true))
$(eval $(call create-target, mceliece_8192128_ref_test,        test,      8192128,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2_test,      test,      8192128,       true,  false))

# Define all HPS4096821 targets
#                            rule name                          target     parameter-set avx2   optimize
$(eval $(call create-target, mceliece_8192128f_ref,             benchmark, 8192128f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2,           benchmark, 8192128f,     true,  false))
$(eval $(call create-target, mceliece_8192128f_ref_optimized,   benchmark, 8192128f,     false, true))
#$(eval $(call create-target, mceliece_8192128f_avx2_optimized, benchmark, 8192128f,     true,  true))
$(eval $(call create-target, mceliece_8192128f_ref_test,        test,      8192128f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2_test,      test,      8192128f,     true,  false))

# Define all HRSS701 targets
#                            rule name                         target     parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_6960119_ref,             benchmark, 6960119,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2,           benchmark, 8192128,       true,  false))
$(eval $(call create-target, mceliece_6960119_ref_optimized,   benchmark, 6960119,       false, true))
#$(eval $(call create-target, mceliece_8192128_avx2_optimized, benchmark, 8192128,       true,  true))
$(eval $(call create-target, mceliece_6960119_ref_test,        test,      6960119,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2_test,      test,      8192128,       true,  false))

# Define all HPS4096821 targets
#                            rule name                          target     parameter-set avx2   optimize
$(eval $(call create-target, mceliece_6960119f_ref,             benchmark, 6960119f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2,           benchmark, 8192128f,     true,  false))
$(eval $(call create-target, mceliece_6960119f_ref_optimized,   benchmark, 6960119f,     false, true))
#$(eval $(call create-target, mceliece_8192128f_avx2_optimized, benchmark, 8192128f,     true,  true))
$(eval $(call create-target, mceliece_6960119f_ref_test,        test,      6960119f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2_test,      test,      8192128f,     true,  false))

# Run all tests
test: tests
	# Test the HRSS701 implementation
	build/mceliece_8192128_ref_test
	build/mceliece_8192128f_ref_test
#	bash -c 'diff <(build/mceliece_8192128_ref_test) <(build/mceliece_8192128_avx2_test)'

	# Test the HPS4086821 implementation

	build/mceliece_6960119_ref_test
	build/mceliece_6960119f_ref_test
#	bash -c 'diff <(build/mceliece_8192128f_ref_test) <(build/mceliece_8192128f_avx2_test)'

# Run all benchmarks
benchmark: benchmarks
	# Benchmark the HRSS701 implementation
	build/mceliece_8192128_ref
#	build/mceliece_8192128_avx2
	build/mceliece_8192128_ref_optimized
#	build/mceliece_8192128_avx2_optimized

	# Benchmark the HPS4096821 implementation
	build/mceliece_8192128f_ref
#	build/mceliece_8192128f_avx2
	build/mceliece_8192128f_ref_optimized
#	build/mceliece_8192128f_avx2_optimized

clean:
	rm -rf build &> /dev/null || true

# This target will simply evaluate to build the target as specified by the setup
ifneq ($(TARGET),none)
target: $(target_name)
$(target_name): $(source) $(headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(source) $(LDLIBS)
else
target:
	@echo "Cannot use target directly"
endif
