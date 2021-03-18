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

	# Add common headers
	headers := lib/api.h lib/params.h

	ifeq ($(shell echo $(PARAMETERS) | grep -q 'f' && echo "true" || echo "false"),true)
		CPPFLAGS += -DUSE_F
	endif

	ifeq ($(USE_AVX2),true)
		CPPFLAGS += -I ../xkcp/build/AVX2/keccak
		LDFLAGS += -L ../xkcp/build/AVX2/keccak
	else
		CPPFLAGS += -I ../xkcp/build/plain-64bitsua/keccak
		LDFLAGS += -L ../xkcp/build/plain-64bitsua/keccak
	endif

	ifeq ($(USE_AVX2),true)
		source := $(shell find lib/avx -type f -iname "*.c")
		source += $(shell find lib/avx -maxdepth 1 -type f -iname "*.S")
		headers += $(shell find lib/avx -type f -iname "*.h")

		ifeq ($(PARAMETERS),$(filter $(PARAMETERS),8192128 8192128f))
			source += lib/avx/syndrome_asm_8192128/syndrome_asm.S
		else ifeq ($(PARAMETERS),$(filter $(PARAMETERS),6960119 6960119f))
			source += lib/avx/syndrome_asm_6960119/syndrome_asm.S
		endif

		# Add the AVX2 flags
		CFLAGS += -mavx2
		CPPFLAGS += -DUSE_AVX2
		features += avx2
	else
		# Add the reference implementation
		source := $(shell find lib/reference -type f -iname "*.c")
		headers += $(shell find lib/reference -type f -iname "*.h")
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
	target_name := build/mceliece_$(PARAMETERS)_$(shell echo "$(strip $(features))" | tr ' ' '_')

	# Set the main target to either test or benchmark
	ifeq ($(TARGET),benchmark-sequential)
		source += benchmark-sequential.c
		target_name := $(target_name)_benchmark_sequential
	else ifeq ($(TARGET),benchmark-parallel)
		source += benchmark-parallel.c ../utilities/time.c ../utilities/pool.c
		target_name := $(target_name)_benchmark_parallel
		LDLIBS += -lpthread
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
tests:  mceliece_8192128_ref_test mceliece_8192128f_ref_test mceliece_6960119_ref_test mceliece_6960119f_ref_test \
		mceliece_8192128_avx2_test mceliece_8192128f_avx2_test mceliece_6960119_avx2_test mceliece_6960119f_avx2_test

# Build all benchmarks
benchmarks: mceliece_8192128_ref_benchmark_sequential mceliece_8192128_ref_optimized_benchmark_sequential	mceliece_8192128f_ref_benchmark_sequential mceliece_8192128f_ref_optimized_benchmark_sequential \
			mceliece_6960119_ref_benchmark_sequential mceliece_6960119_ref_optimized_benchmark_sequential	mceliece_6960119f_ref_benchmark_sequential mceliece_6960119f_ref_optimized_benchmark_sequential \
			mceliece_8192128_avx2_benchmark_sequential mceliece_8192128_avx2_optimized_benchmark_sequential	mceliece_8192128f_avx2_benchmark_sequential mceliece_8192128f_avx2_optimized_benchmark_sequential \
			mceliece_6960119_avx2_benchmark_sequential mceliece_6960119_avx2_optimized_benchmark_sequential	mceliece_6960119f_avx2_benchmark_sequential mceliece_6960119f_avx2_optimized_benchmark_sequential \
			mceliece_8192128_avx2_optimized_benchmark_parallel mceliece_8192128f_avx2_optimized_benchmark_parallel mceliece_6960119_avx2_optimized_benchmark_parallel mceliece_6960119f_avx2_optimized_benchmark_parallel

# Define all mceliece8192128 targets
#                            rule name                                             target                parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_8192128_ref_benchmark_sequential,            benchmark-sequential, 8192128,       false, false))
$(eval $(call create-target, mceliece_8192128_avx2_benchmark_sequential,           benchmark-sequential, 8192128,       true,  false))
$(eval $(call create-target, mceliece_8192128_ref_optimized_benchmark_sequential,  benchmark-sequential, 8192128,       false, true))
$(eval $(call create-target, mceliece_8192128_avx2_optimized_benchmark_sequential, benchmark-sequential, 8192128,       true,  true))
$(eval $(call create-target, mceliece_8192128_avx2_optimized_benchmark_parallel,   benchmark-parallel,   8192128,       true,  true))
$(eval $(call create-target, mceliece_8192128_ref_test,                            test,                 8192128,       false, false))
$(eval $(call create-target, mceliece_8192128_avx2_test,                           test,                 8192128,       true,  false))

# Define all mceliece8192128f targets
#                            rule name                                              target     parameter-set avx2   optimize
$(eval $(call create-target, mceliece_8192128f_ref_benchmark_sequential,            benchmark-sequential, 8192128f,     false, false))
$(eval $(call create-target, mceliece_8192128f_avx2_benchmark_sequential,           benchmark-sequential, 8192128f,     true,  false))
$(eval $(call create-target, mceliece_8192128f_ref_optimized_benchmark_sequential,  benchmark-sequential, 8192128f,     false, true))
$(eval $(call create-target, mceliece_8192128f_avx2_optimized_benchmark_sequential, benchmark-sequential, 8192128f,     true,  true))
$(eval $(call create-target, mceliece_8192128f_avx2_optimized_benchmark_parallel,   benchmark-parallel,   8192128f,     true,  true))
$(eval $(call create-target, mceliece_8192128f_ref_test,                            test,                 8192128f,     false, false))
$(eval $(call create-target, mceliece_8192128f_avx2_test,                           test,                 8192128f,     true,  false))

# Define all mceliece6960119 targets
#                            rule name                                             target                parameter-set  avx2   optimize
$(eval $(call create-target, mceliece_6960119_ref_benchmark_sequential,            benchmark-sequential, 6960119,       false, false))
$(eval $(call create-target, mceliece_6960119_avx2_benchmark_sequential,           benchmark-sequential, 6960119,       true,  false))
$(eval $(call create-target, mceliece_6960119_ref_optimized_benchmark_sequential,  benchmark-sequential, 6960119,       false, true))
$(eval $(call create-target, mceliece_6960119_avx2_optimized_benchmark_sequential, benchmark-sequential, 6960119,       true,  true))
$(eval $(call create-target, mceliece_6960119_avx2_optimized_benchmark_parallel,   benchmark-parallel,   6960119,       true,  true))
$(eval $(call create-target, mceliece_6960119_ref_test,                            test,                 6960119,       false, false))
$(eval $(call create-target, mceliece_6960119_avx2_test,                           test,                 6960119,       true,  false))

# Define all mceliece6960119f targets
#                            rule name                                              target                parameter-set avx2   optimize
$(eval $(call create-target, mceliece_6960119f_ref_benchmark_sequential,            benchmark-sequential, 6960119f,     false, false))
$(eval $(call create-target, mceliece_6960119f_avx2_benchmark_sequential,           benchmark-sequential, 6960119f,     true,  false))
$(eval $(call create-target, mceliece_6960119f_ref_optimized_benchmark_sequential,  benchmark-sequential, 6960119f,     false, true))
$(eval $(call create-target, mceliece_6960119f_avx2_optimized_benchmark_sequential, benchmark-sequential, 6960119f,     true,  true))
$(eval $(call create-target, mceliece_6960119f_avx2_optimized_benchmark_parallel,   benchmark-parallel,   6960119f,     true,  true))
$(eval $(call create-target, mceliece_6960119f_ref_test,                            test,                 6960119f,     false, false))
$(eval $(call create-target, mceliece_6960119f_avx2_test,                           test,                 6960119f,     true,  false))

# Run all tests
test: tests
	# Test the mceliece8192128 implementations
	build/mceliece_8192128_ref_test
	build/mceliece_8192128_avx2_test
	build/mceliece_8192128f_ref_test
	build/mceliece_8192128f_avx2_test
#	bash -c 'diff <(build/mceliece_8192128_ref_test) <(build/mceliece_8192128_avx2_test)'

	# Test the mceliece6960119 implementations
	build/mceliece_6960119_ref_test
	build/mceliece_6960119_avx2_test
	build/mceliece_6960119f_ref_test
	build/mceliece_6960119f_avx2_test
#	bash -c 'diff <(build/mceliece_8192128f_ref_test) <(build/mceliece_8192128f_avx2_test)'

# Run all benchmarks
benchmark: benchmarks
	# Benchmark the mceliece8192128 implementations
	build/mceliece_8192128_ref_benchmark_sequential
	build/mceliece_8192128_avx2_benchmark_sequential
	build/mceliece_8192128_optimized_benchmark_sequential
	build/mceliece_8192128_avx2_optimized_benchmark_sequential
	build/mceliece_8192128_avx2_optimized_benchmark_parallel

	# Benchmark the mceliece8192128f implementations
	build/mceliece_8192128f_ref_benchmark_sequential
	build/mceliece_8192128f_avx2_benchmark_sequential
	build/mceliece_8192128f_optimized_benchmark_sequential
	build/mceliece_8192128f_avx2_optimized_benchmark_sequential
	build/mceliece_8192128f_avx2_optimized_benchmark_parallel

	# Benchmark the mceliece6960119 implementations
	build/mceliece_6960119_ref_benchmark_sequential
	build/mceliece_6960119_avx2_benchmark_sequential
	build/mceliece_6960119_optimized_benchmark_sequential
	build/mceliece_6960119_avx2_optimized_benchmark_sequential
	build/mceliece_6960119_avx2_optimized_benchmark_parallel

	# Benchmark the mceliece6960119f implementations
	build/mceliece_6960119f_ref_benchmark_sequential
	build/mceliece_6960119f_avx2_benchmark_sequential
	build/mceliece_6960119f_optimized_benchmark_sequential
	build/mceliece_6960119f_avx2_optimized_benchmark_sequential
	build/mceliece_6960119f_avx2_optimized_benchmark_parallel

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
