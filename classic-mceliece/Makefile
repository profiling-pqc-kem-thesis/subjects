.PHONY: all tests benchmarks test target clean

# The build type, 'none', 'test' or 'benchmark'
TARGET ?= none

# The parameters, such as '8192128' or '6960119f'
PARAMETERS ?=

# Whether or not (true/false) to use AVX2
USE_AVX2 ?= false

# Whether or not (true/false) to use optimization flags
USE_OPTIMIZATION_FLAGS ?= false

# Liberties to load
LDLIBS += -lcrypto -lkeccak

# If the target is not none, define all required parameters for building a target
ifneq ($(TARGET),none)

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

	ifeq ($(USE_OPTIMIZATION_FLAGS),true)
		CFLAGS += -march=native
		features += optimized
	endif

	# If no features are enabled, use "ref" for "reference"
	ifeq ($(features),)
		features := ref
	endif

	# Define the target name like so:
	# build/mceliece_8192128-avx2-optimized
	target_name := build/mceliece_$(PARAMETERS)_$(shell echo "$(strip $(features))" | tr ' ' '-')

	# Set the main target to either test or benchmark
	ifeq ($(TARGET),benchmark)
		source += benchmark.c
		target_name := $(target_name)
	else ifeq ($(TARGET),test)
		source += test.c
		target_name := $(target_name)_test
		CFLAGS += -g
	endif

	# Format the constant for controlling the implementation. Will look like
	# MCELIECE_8192128. Possible values are defined in params.h
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
#tests: mceliece_8192128_ref_test mceliece_8192128_avx2n_test \
#	mceliece_8192128f_ref_test mceliece_8192128f_avx2_test

tests:  mceliece_8192128_ref_test mceliece_8192128f_ref_test mceliece_6960119_ref_test mceliece_6960119f_ref_test

# Build all benchmarks
#benchmarks: mceliece_8192128_ref mceliece_8192128_avx2 mceliece_8192128_ref_optimized mceliece_8192128_avx2_optimized \
#	mceliece_8192128f_ref mceliece_8192128f_avx2 mceliece_8192128f_ref_optimized mceliece_8192128f_avx2_optimized

benchmarks: mceliece_8192128_ref mceliece_8192128_ref_optimized	mceliece_8192128f_ref mceliece_8192128f_ref_optimized \
						mceliece_6960119_ref mceliece_6960119_ref_optimized	mceliece_6960119f_ref mceliece_6960119f_ref_optimized

# Define all mceliece8192128 targets
#                            rule name                         target     parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_8192128_ref,             benchmark, 8192128,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2,           benchmark, 8192128,       true,  false))
$(eval $(call create-target, mceliece_8192128_ref_optimized,   benchmark, 8192128,       false, true))
#$(eval $(call create-target, mceliece_8192128_avx2_optimized, benchmark, 8192128,       true,  true))
$(eval $(call create-target, mceliece_8192128_ref_test,        test,      8192128,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2_test,      test,      8192128,       true,  false))

# Define all mceliece8192128f targets
#                            rule name                          target     parameter-set avx2   optimize
$(eval $(call create-target, mceliece_8192128f_ref,             benchmark, 8192128f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2,           benchmark, 8192128f,     true,  false))
$(eval $(call create-target, mceliece_8192128f_ref_optimized,   benchmark, 8192128f,     false, true))
#$(eval $(call create-target, mceliece_8192128f_avx2_optimized, benchmark, 8192128f,     true,  true))
$(eval $(call create-target, mceliece_8192128f_ref_test,        test,      8192128f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2_test,      test,      8192128f,     true,  false))

# Define all mceliece6960119 targets
#                            rule name                         target     parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_6960119_ref,             benchmark, 6960119,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2,           benchmark, 8192128,       true,  false))
$(eval $(call create-target, mceliece_6960119_ref_optimized,   benchmark, 6960119,       false, true))
#$(eval $(call create-target, mceliece_8192128_avx2_optimized, benchmark, 8192128,       true,  true))
$(eval $(call create-target, mceliece_6960119_ref_test,        test,      6960119,       false, false))
#$(eval $(call create-target, mceliece_8192128_avx2_test,      test,      8192128,       true,  false))

# Define all mceliece6960119f targets
#                            rule name                          target     parameter-set avx2   optimize
$(eval $(call create-target, mceliece_6960119f_ref,             benchmark, 6960119f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2,           benchmark, 8192128f,     true,  false))
$(eval $(call create-target, mceliece_6960119f_ref_optimized,   benchmark, 6960119f,     false, true))
#$(eval $(call create-target, mceliece_8192128f_avx2_optimized, benchmark, 8192128f,     true,  true))
$(eval $(call create-target, mceliece_6960119f_ref_test,        test,      6960119f,     false, false))
#$(eval $(call create-target, mceliece_8192128f_avx2_test,      test,      8192128f,     true,  false))

# Run all tests
test: tests
	# Test the mceliece8192128 implementations
	build/mceliece_8192128_ref_test
	build/mceliece_8192128f_ref_test
#	bash -c 'diff <(build/mceliece_8192128_ref_test) <(build/mceliece_8192128_avx2_test)'

	# Test the mceliece6960119 implementations
	build/mceliece_6960119_ref_test
	build/mceliece_6960119f_ref_test
#	bash -c 'diff <(build/mceliece_8192128f_ref_test) <(build/mceliece_8192128f_avx2_test)'

# Run all benchmarks
benchmark: benchmarks
	# Benchmark the mceliece8192128 implementations
	build/mceliece_8192128_ref
#	build/mceliece_8192128_avx2
	build/mceliece_8192128_optimized
#	build/mceliece_8192128_avx2_optimized

	# Benchmark the mceliece8192128f implementations
	build/mceliece_8192128f_ref
#	build/mceliece_8192128f_avx2
	build/mceliece_8192128f_optimized
#	build/mceliece_8192128f_avx2_optimized

	# Benchmark the mceliece6960119 implementations
	build/mceliece_6960119_ref
#	build/mceliece_6960119_avx2
	build/mceliece_6960119_optimized
#	build/mceliece_6960119_avx2_optimized

	# Benchmark the mceliece6960119f implementations
	build/mceliece_6960119f_ref
#	build/mceliece_6960119f_avx2
	build/mceliece_6960119f_optimized
#	build/mceliece_6960119f_avx2_optimized

# Analyze the hot paths of the algorithm
hotpaths: mceliece_8192128_ref_test mceliece_8192128f_ref_test mceliece_6960119_ref_test mceliece_6960119f_ref_test
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_8192128_ref_test.profile build/mceliece_8192128_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_8192128f_ref_test.profile build/mceliece_8192128f_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_6960119_ref_test.profile build/mceliece_6960119_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_6960119f_ref_test.profile build/mceliece_6960119f_ref_test > /dev/null

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
