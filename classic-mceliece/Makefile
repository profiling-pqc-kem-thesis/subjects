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
		CPPFLAGS += -I ../xkcp/build/plain-64bitslcu6/keccak
		LDFLAGS += -L ../xkcp/build/plain-64bitslcu6/keccak
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

	# If no features are enabled, use "ref" for "reference"
	ifeq ($(features),)
		features := ref
	endif

	ifeq ($(USE_OPTIMIZATION_FLAGS),true)
		CFLAGS += -O3 -march=native
		features += optimized
	endif

	# Define the target name like so:
	# build/mceliece_8192128-avx2-optimized
	target_name := build/mceliece_$(PARAMETERS)_$(CC)_$(shell echo "$(strip $(features))" | tr ' ' '-')

	# Set the main target to either test or benchmark
	ifeq ($(TARGET),benchmark)
		source += benchmark.c ../utilities/benchmark.c ../utilities/benchmark-sequential.c ../utilities/benchmark-parallel.c ../utilities/cpu.c ../utilities/pool.c ../utilities/time.c
		target_name := $(target_name)
		LDLIBS += -lpthread
		CPPFLAGS += -D BENCHMARK_SUBJECT_NAME='"$(target_name)"' -D BENCHMARK_KEYPAIR -D BENCHMARK_ENCRYPT -D BENCHMARK_DECRYPT
	else ifeq ($(TARGET),test)
		source += test.c
		target_name := $(target_name)_test
		CFLAGS += -g
	endif

	# Format the constant for controlling the implementation. Will look like
	# MCELIECE_8192128. Possible values are defined in params.h
	CPPFLAGS += -D MCELIECE_PARAMETER_SET=MCELIECE_$(PARAMETERS)
endif

# This macro creates a build target for a configuration of NTUR
define create-target
$(strip $(1)):
ifeq ($$(strip $(5)),true)
ifeq ($$(shell uname),Darwin)
ifeq ($$(shell sysctl machdep.cpu.leaf7_features | cut -d ':' -f2 | grep -o AVX2),)
	@echo "Warning: skipping unsupported AVX2 build"
else
	TARGET=$(strip $(2)) PARAMETERS=$(strip $(3)) CC=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) $$(MAKE) target
endif
else
ifeq ($$(shell lscpu | grep '^Flags' | cut -d ':' -f2 | grep -o avx2),)
	@echo "Warning: skipping unsupported AVX2 build"
else
	TARGET=$(strip $(2)) PARAMETERS=$(strip $(3)) CC=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) $$(MAKE) target
endif
endif
else
	TARGET=$(strip $(2)) PARAMETERS=$(strip $(3)) CC=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) $$(MAKE) target
endif
endef

all: tests benchmarks

# Build all tests
tests:  mceliece_8192128_gcc_ref_test mceliece_8192128f_gcc_ref_test mceliece_6960119_gcc_ref_test mceliece_6960119f_gcc_ref_test \
		mceliece_8192128_gcc_avx2_test mceliece_8192128f_gcc_avx2_test mceliece_6960119_gcc_avx2_test mceliece_6960119f_gcc_avx2_test

# Build all benchmarks
benchmarks: mceliece_8192128_gcc_ref mceliece_8192128_gcc_ref-optimized	mceliece_8192128_clang_ref-optimized mceliece_8192128f_gcc_ref mceliece_8192128f_gcc_ref-optimized mceliece_8192128f_clang_ref-optimized \
			mceliece_6960119_gcc_ref mceliece_6960119_gcc_ref-optimized mceliece_6960119_clang_ref-optimized	mceliece_6960119f_gcc_ref mceliece_6960119f_gcc_ref-optimized mceliece_6960119f_clang_ref-optimized \
			mceliece_8192128_gcc_avx2 mceliece_8192128_gcc_avx2-optimized mceliece_8192128_clang_avx2-optimized	mceliece_8192128f_gcc_avx2 mceliece_8192128f_gcc_avx2-optimized mceliece_8192128f_clang_avx2-optimized \
			mceliece_6960119_gcc_avx2 mceliece_6960119_gcc_avx2-optimized	mceliece_6960119_clang_avx2-optimized mceliece_6960119f_gcc_avx2 mceliece_6960119f_gcc_avx2-optimized mceliece_6960119f_clang_avx2-optimized

# Define all mceliece8192128 targets
#                            rule name                              target     parameter-set  compiler  avx2   optimize
$(eval $(call create-target, mceliece_8192128_gcc_ref,              benchmark, 8192128,       gcc,      false, false))
$(eval $(call create-target, mceliece_8192128_gcc_avx2,             benchmark, 8192128,       gcc,      true,  false))
$(eval $(call create-target, mceliece_8192128_gcc_ref-optimized,    benchmark, 8192128,       gcc,      false, true))
$(eval $(call create-target, mceliece_8192128_clang_ref-optimized,  benchmark, 8192128,       clang,    false, true))
$(eval $(call create-target, mceliece_8192128_gcc_avx2-optimized,   benchmark, 8192128,       gcc,      true,  true))
$(eval $(call create-target, mceliece_8192128_clang_avx2-optimized, benchmark, 8192128,       clang,    true,  true))
$(eval $(call create-target, mceliece_8192128_gcc_ref_test,         test,      8192128,       gcc,      false, false))
$(eval $(call create-target, mceliece_8192128_gcc_avx2_test,        test,      8192128,       gcc,      true,  false))

# Define all mceliece8192128f targets
#                            rule name                              target     parameter-set compiler  avx2   optimize
$(eval $(call create-target, mceliece_8192128f_gcc_ref,              benchmark, 8192128f,     gcc,      false, false))
$(eval $(call create-target, mceliece_8192128f_gcc_avx2,             benchmark, 8192128f,     gcc,      true,  false))
$(eval $(call create-target, mceliece_8192128f_gcc_ref-optimized,    benchmark, 8192128f,     gcc,      false, true))
$(eval $(call create-target, mceliece_8192128f_clang_ref-optimized,  benchmark, 8192128f,     clang,    false, true))
$(eval $(call create-target, mceliece_8192128f_gcc_avx2-optimized,   benchmark, 8192128f,     gcc,      true,  true))
$(eval $(call create-target, mceliece_8192128f_clang_avx2-optimized, benchmark, 8192128f,     clang,    true,  true))
$(eval $(call create-target, mceliece_8192128f_gcc_ref_test,         test,      8192128f,     gcc,      false, false))
$(eval $(call create-target, mceliece_8192128f_gcc_avx2_test,        test,      8192128f,     gcc,      true,  false))

# Define all mceliece6960119 targets
#                            rule name                              target     parameter-set  compiler  avx2   optimize
$(eval $(call create-target, mceliece_6960119_gcc_ref,              benchmark, 6960119,       gcc,      false, false))
$(eval $(call create-target, mceliece_6960119_gcc_avx2,             benchmark, 6960119,       gcc,      true,  false))
$(eval $(call create-target, mceliece_6960119_gcc_ref-optimized,    benchmark, 6960119,       gcc,      false, true))
$(eval $(call create-target, mceliece_6960119_clang_ref-optimized,  benchmark, 6960119,       clang,    false, true))
$(eval $(call create-target, mceliece_6960119_gcc_avx2-optimized,   benchmark, 6960119,       gcc,      true,  true))
$(eval $(call create-target, mceliece_6960119_clang_avx2-optimized, benchmark, 6960119,       clang,    true,  true))
$(eval $(call create-target, mceliece_6960119_gcc_ref_test,         test,      6960119,       gcc,      false, false))
$(eval $(call create-target, mceliece_6960119_gcc_avx2_test,        test,      6960119,       gcc,      true,  false))

# Define all mceliece6960119f targets
#                            rule name                               target     parameter-set compiler  avx2   optimize
$(eval $(call create-target, mceliece_6960119f_gcc_ref,              benchmark, 6960119f,     gcc,      false, false))
$(eval $(call create-target, mceliece_6960119f_gcc_avx2,             benchmark, 6960119f,     gcc,      true,  false))
$(eval $(call create-target, mceliece_6960119f_gcc_ref-optimized,    benchmark, 6960119f,     gcc,      false, true))
$(eval $(call create-target, mceliece_6960119f_clang_ref-optimized,  benchmark, 6960119f,     clang,    false, true))
$(eval $(call create-target, mceliece_6960119f_gcc_avx2-optimized,   benchmark, 6960119f,     gcc,      true,  true))
$(eval $(call create-target, mceliece_6960119f_clang_avx2-optimized, benchmark, 6960119f,     clang,    true,  true))
$(eval $(call create-target, mceliece_6960119f_gcc_ref_test,         test,      6960119f,     gcc,      false, false))
$(eval $(call create-target, mceliece_6960119f_gcc_avx2_test,        test,      6960119f,     gcc,      true,  false))

# Run all tests
test: tests
	# Test the mceliece8192128 implementations
	build/mceliece_8192128_gcc_ref_test
	build/mceliece_8192128_gcc_avx2_test
	build/mceliece_8192128f_gcc_ref_test
	build/mceliece_8192128f_gcc_avx2_test
#	bash -c 'diff <(build/mceliece_8192128_ref_test) <(build/mceliece_8192128_avx2_test)'

	# Test the mceliece6960119 implementations
	build/mceliece_6960119_gcc_ref_test
	build/mceliece_6960119_gcc_avx2_test
	build/mceliece_6960119f_gcc_ref_test
	build/mceliece_6960119f_gcc_avx2_test
#	bash -c 'diff <(build/mceliece_8192128f_ref_test) <(build/mceliece_8192128f_avx2_test)'

# Analyze the hot paths of the algorithm
hotpaths: mceliece_8192128_gcc_ref_test mceliece_8192128f_gcc_ref_test mceliece_6960119_gcc_ref_test mceliece_6960119f_gcc_ref_test
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_8192128_gcc_ref_test.profile build/mceliece_8192128_gcc_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_8192128f_gcc_ref_test.profile build/mceliece_8192128f_gcc_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_6960119_gcc_ref_test.profile build/mceliece_6960119_gcc_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/mceliece_6960119f_gcc_ref_test.profile build/mceliece_6960119f_gcc_ref_test > /dev/null

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
