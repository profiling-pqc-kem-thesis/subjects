.PHONY: all tests benchmarks test target clean

# The build type, 'none', 'test' or 'benchmark'
TARGET ?= none

# The group to build, 'hrss' or 'hps'
GROUP ?=

# The parameters, such as 701
PARAMETERS ?=

# Whether or not (true/false) to use AVX2
USE_AVX2 ?= false

# Whether or not (true/false) to use optimization flags
USE_OPTIMIZATION_FLAGS ?= false

# If the target is not none, define all required parameters for building a target
ifneq ($(TARGET),none)
	# The name of the parameter set, such as hrss701
	parameter_set := $(GROUP)$(PARAMETERS)

	# Add common source and headers
	source := $(shell find * -maxdepth 1 -type f -iname "*.c" -not -iname "crypto_sort_int32.c" -not -iname "test.c" -not -iname "benchmark.c")
	headers := $(shell find * -maxdepth 1 -type f -iname "*.h" -not -iname "crypto_sort_int32.h")

	# Add common poly and sample source
	source += $(shell find poly/common -type f -iname "*.c")
	source += $(shell find sample/common -type f -iname "*.c")

	# Define the target name
	target_name := build/ntru_$(parameter_set)

	ifeq ($(USE_AVX2),true)
		# Add the target-specific AVX2 poly implementation
		source += $(shell find poly/avx2 -maxdepth 1 -type f -iname "*.c" -or -iname "*.s")
		source += $(shell find poly/avx2/$(parameter_set) -type f -iname "*.c" -or -iname "*.s")
		headers += poly/avx2/poly_r2_inv.h
		# Add the target-specific AVX2 sample implementation
		source += sample/avx2/$(parameter_set)/vec32_sample_iid.s
		target_name := $(target_name)_avx2
		CFLAGS += -mavx2
		CPPFLAGS += -DUSE_AVX2
	else
		# Add the reference poly implementation
		source += $(shell find poly/ref -type f -iname "*.c")
		target_name := $(target_name)_ref
	endif

	# Add source only used by the HPS family
	ifeq ($(GROUP),hps)
		source += crypto_sort_int32.c
		headers += crypto_sort_int32.h
	endif

	ifeq ($(USE_OPTIMIZATION_FLAGS),true)
		target_name := $(target_name)_optimized
		CFLAGS += -march=native
	else
		target_name := $(target_name)_plain
	endif

	# Set the main target to either test or benchmark
	ifeq ($(TARGET),benchmark)
		source += benchmark.c
	else ifeq ($(TARGET),test)
		source += test.c
		target_name := $(target_name)_test
	endif

	# Format the constant for controlling the implementation. Will look like
	# NTRU_HPS_2048677. Possible values are defined in params.h
	NTRU_PARAMETER_SET := NTRU_$(shell echo $(GROUP) | tr  '[:lower:]' '[:upper:]')_$(PARAMETERS)
	CPPFLAGS += -D NTRU_PARAMETER_SET=$(NTRU_PARAMETER_SET)
endif

# This macro creates a build target for a configuration of NTUR
define create-target
$(strip $(1)):
	TARGET=$(strip $(2)) GROUP=$(strip $(3)) PARAMETERS=$(strip $(4)) USE_AVX2=$(strip $(5)) USE_OPTIMIZATION_FLAGS=$(strip $(6)) $$(MAKE) target
endef

all: tests benchmarks test

# Build all tests
# first line is HRS701 targets, the second HPS4096821
tests: ntru_hrss701_ref_plain_test ntru_hrss701_avx2_plain_test \
	ntru_hps4096821_ref_plain_test ntru_hps4096821_avx2_plain_test

# Build all benchmarks
# first line is HRS701 targets, the second HPS4096821
benchmarks: ntru_hrss701_ref_plain ntru_hrss701_avx2_plain ntru_hrss701_ref_optimized ntru_hrss701_avx2_optimized \
	ntru_hps4096821_ref_plain ntru_hps4096821_avx2_plain ntru_hps4096821_ref_optimized ntru_hps4096821_avx2_optimized

# Define all HRSS701 targets
#                            rule name                     target     group  parameter  avx2   optimize
$(eval $(call create-target, ntru_hrss701_ref_plain,       benchmark, hrss,  701,       false, false))
$(eval $(call create-target, ntru_hrss701_avx2_plain,      benchmark, hrss,  701,       true,  false))
$(eval $(call create-target, ntru_hrss701_ref_optimized,   benchmark, hrss,  701,       false, true))
$(eval $(call create-target, ntru_hrss701_avx2_optimized,  benchmark, hrss,  701,       true,  true))
$(eval $(call create-target, ntru_hrss701_ref_plain_test,  test,      hrss,  701,       false, false))
$(eval $(call create-target, ntru_hrss701_avx2_plain_test, test,      hrss,  701,       true,  false))

# Define all HPS4096821 targets
#                            rule name                        target     group  parameter  avx2   optimize
$(eval $(call create-target, ntru_hps4096821_ref_plain,       benchmark, hps,   4096821,   false, false))
$(eval $(call create-target, ntru_hps4096821_avx2_plain,      benchmark, hps,   4096821,   true,  false))
$(eval $(call create-target, ntru_hps4096821_ref_optimized,   benchmark, hps,   4096821,   false, true))
$(eval $(call create-target, ntru_hps4096821_avx2_optimized,  benchmark, hps,   4096821,   true,  true))
$(eval $(call create-target, ntru_hps4096821_ref_plain_test,  test,      hps,   4096821,   false, false))
$(eval $(call create-target, ntru_hps4096821_avx2_plain_test, test,      hps,   4096821,   true,  false))

# Run all tests
test: tests
	# Test the HRSS701 implementation
	build/ntru_hrss701_ref_plain_test
	bash -c 'diff <(build/ntru_hrss701_ref_plain_test) <(build/ntru_hrss701_avx2_plain_test)'

	# Test the HPS4086821 implementation
	build/ntru_hps4096821_ref_plain_test
	bash -c 'diff <(build/ntru_hps4096821_ref_plain_test) <(build/ntru_hps4096821_avx2_plain_test)'

# Run all benchmarks
benchmark: benchmarks
	# Benchmark the HRSS701 implementation
	build/ntru_hrss701_ref_plain
	build/ntru_hrss701_avx2_plain
	build/ntru_hrss701_ref_optimized
	build/ntru_hrss701_avx2_optimized

	# Benchmark the HPS4096821 implementation
	build/ntru_hps4096821_ref_plain
	build/ntru_hps4096821_avx2_plain
	build/ntru_hps4096821_ref_optimized
	build/ntru_hps4096821_avx2_optimized

clean:
	rm -rf build &> /dev/null || true

# This target will simply evaluate to build the target as specified by the setup
ifneq ($(TARGET),none)
target: $(target_name)
$(target_name): $(source) $(headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(source) -lcrypto
else
target:
	@echo "Cannot use target directly"
endif
