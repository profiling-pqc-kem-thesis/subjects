.PHONY: all tests benchmarks test target clean

# The build type, 'none', 'test', 'benchmark'
TARGET ?= none

# The group to build, 'hrss' or 'hps'
GROUP ?=

# The parameters, such as 701
PARAMETERS ?=

# Whether or not (true/false) to use AVX2
USE_AVX2 ?= false

# Whether or not (true/false) to use optimization flags
USE_OPTIMIZATION_FLAGS ?= false

# Whether or not (true/false) to use KECCAK
USE_KECCAK ?= false

LDLIBS += -lcrypto

# If the target is not none, define all required parameters for building a target
ifneq ($(TARGET),none)
	# The name of the parameter set, such as hrss701
	parameter_set := $(GROUP)$(PARAMETERS)

	# Add common source and headers
	source := $(shell find lib -maxdepth 1 -type f -iname "*.c" -not -iname "crypto_sort_int32.c" -not -iname "fips202.c")
	headers := $(shell find lib -maxdepth 1 -type f -iname "*.h" -not -iname "crypto_sort_int32.h")

	# Add common poly and sample source
	source += $(shell find lib/poly/common -type f -iname "*.c")
	source += $(shell find lib/sample/common -type f -iname "*.c")

	# Enabled features (avx2, keccak, optimized compiler flags etc.)
	features :=

	ifeq ($(USE_KECCAK),true)
		LDLIBS += -lkeccak

		features += keccak

		ifeq ($(USE_AVX2),true)
			CPPFLAGS += -DUSE_KECCAK -I ../xkcp/build/AVX2/keccak
			LDFLAGS += -L ../xkcp/build/AVX2/keccak
		else
			CPPFLAGS += -DUSE_KECCAK -I ../xkcp/build/plain-64bitsua/keccak
			LDFLAGS += -L ../xkcp/build/plain-64bitsua/keccak
		endif
	else
		source += lib/fips202.c
	endif

	ifeq ($(USE_AVX2),true)
		# Add the target-specific AVX2 poly implementation
		source += $(shell find lib/poly/avx2 -maxdepth 1 -type f -iname "*.c" -or -iname "*.s")
		source += $(shell find lib/poly/avx2/$(parameter_set) -type f -iname "*.c" -or -iname "*.s")
		headers += lib/poly/avx2/poly_r2_inv.h
		# Add the target-specific AVX2 sample implementation
		source += lib/sample/avx2/$(parameter_set)/vec32_sample_iid.s
		CFLAGS += -mavx2
		CPPFLAGS += -DUSE_AVX2
		features += avx2
	else
		# Add the reference poly implementation
		source += $(shell find lib/poly/ref -type f -iname "*.c")
	endif

	# Add source only used by the HPS family
	ifeq ($(GROUP),hps)
		source += lib/crypto_sort_int32.c
		headers += lib/crypto_sort_int32.h
	endif

	# If no features are enabled, use "ref" for "reference"
	ifeq ($(features),)
		features := ref
	endif

	ifeq ($(USE_OPTIMIZATION_FLAGS),true)
		CFLAGS += -march=native
		features += optimized
	endif

	# Define the target name like so:
	# build/ntru_hrss701_keccak-avx2-optimized
	target_name := build/ntru_$(parameter_set)_$(shell echo "$(strip $(features))" | tr ' ' '-')

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
	# NTRU_HPS_2048677. Possible values are defined in params.h
	NTRU_PARAMETER_SET := NTRU_$(shell echo $(GROUP) | tr '[:lower:]' '[:upper:]')_$(PARAMETERS)
	CPPFLAGS += -D NTRU_PARAMETER_SET=$(NTRU_PARAMETER_SET)
endif

# This macro creates a build target for a configuration of NTUR
define create-target
$(strip $(1)):
ifeq ($$(strip $(5)),true)
ifeq ($$(shell uname),Darwin)
ifeq ($$(shell sysctl machdep.cpu.leaf7_features | cut -d ':' -f2 | grep -o AVX2),)
	@echo "Warning: skipping unsupported AVX2 build"
else
	TARGET=$(strip $(2)) GROUP=$(strip $(3)) PARAMETERS=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) USE_KECCAK=$(strip $(7)) $$(MAKE) target
endif
else
ifeq ($$(shell lscpu | grep '^Flags' | cut -d ':' -f2 | grep -o avx2),)
	@echo "Warning: skipping unsupported AVX2 build"
else
	TARGET=$(strip $(2)) GROUP=$(strip $(3)) PARAMETERS=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) USE_KECCAK=$(strip $(7)) $$(MAKE) target
endif
endif
else
	TARGET=$(strip $(2)) GROUP=$(strip $(3)) PARAMETERS=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) USE_KECCAK=$(strip $(7)) $$(MAKE) target
endif
endef

all: tests benchmarks

# Build all tests
# first line is HRS701 targets, the second HPS4096821
tests: ntru_hrss701_ref_test ntru_hrss701_avx2_test ntru_hrss701_keccak_test \
	ntru_hps4096821_ref_test ntru_hps4096821_avx2_test ntru_hps4096821_keccak_test

# Build all benchmarks
# first line is HRS701 targets, the second HPS4096821
benchmarks: ntru_hrss701_ref ntru_hrss701_avx2 ntru_hrss701_ref-optimized ntru_hrss701_avx2-optimized \
	ntru_hps4096821_ref ntru_hps4096821_avx2 ntru_hps4096821_ref-optimized ntru_hps4096821_avx2-optimized

# Define all HRSS701 targets
#                            rule name                    target     group  parameter  avx2   optimize  keccak
$(eval $(call create-target, ntru_hrss701_ref,            benchmark, hrss,  701,       false, false,    false))
$(eval $(call create-target, ntru_hrss701_avx2,           benchmark, hrss,  701,       true,  false,    false))
$(eval $(call create-target, ntru_hrss701_ref-optimized,  benchmark, hrss,  701,       false, true,     false))
$(eval $(call create-target, ntru_hrss701_avx2-optimized, benchmark, hrss,  701,       true,  true,     false))

$(eval $(call create-target, ntru_hrss701_ref_test,       test,       hrss,  701,       false, false,    false))
$(eval $(call create-target, ntru_hrss701_avx2_test,      test,       hrss,  701,       true,  false,    false))
$(eval $(call create-target, ntru_hrss701_keccak_test,    test,       hrss,  701,       false,  false,   true))

# Define all HPS4096821 targets
#                            rule name                       target     group  parameter  avx2   optimize  keccak
$(eval $(call create-target, ntru_hps4096821_ref,            benchmark, hps,   4096821,   false, false,    false))
$(eval $(call create-target, ntru_hps4096821_avx2,           benchmark, hps,   4096821,   true,  false,    false))
$(eval $(call create-target, ntru_hps4096821_ref-optimized,  benchmark, hps,   4096821,   false, true,     false))
$(eval $(call create-target, ntru_hps4096821_avx2-optimized, benchmark, hps,   4096821,   true,  true,     false))

$(eval $(call create-target, ntru_hps4096821_ref_test,       test,      hps,   4096821,   false, false,    false))
$(eval $(call create-target, ntru_hps4096821_avx2_test,      test,      hps,   4096821,   true,  false,    false))
$(eval $(call create-target, ntru_hps4096821_keccak_test,    test,      hps,   4096821,   false, false,    true))

# Run all tests
test: tests
	# Test the HRSS701 implementation
	build/ntru_hrss701_ref_test
	bash -c 'diff <(build/ntru_hrss701_ref_test) <(build/ntru_hrss701_avx2_test)'
	bash -c 'diff <(build/ntru_hrss701_ref_test) <(build/ntru_hrss701_keccak_test)'

	# Test the HPS4086821 implementation
	build/ntru_hps4096821_ref_test
	bash -c 'diff <(build/ntru_hps4096821_ref_test) <(build/ntru_hps4096821_avx2_test)'
	bash -c 'diff <(build/ntru_hps4096821_ref_test) <(build/ntru_hps4096821_keccak_test)'

# Analyze the hot paths of the algorithm
hotpaths: ntru_hrss701_ref_test ntru_hps4096821_ref_test
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/ntru_hrss701_ref_test.profile build/ntru_hrss701_ref_test > /dev/null
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=build/ntru_hps4096821_ref_test.profile build/ntru_hps4096821_ref_test > /dev/null

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
