# Disable echoing of commands
MAKEFLAGS += --silent

CPPFLAGS += -I $(CURDIR)/perf/build/include
LDFLAGS += -L $(CURDIR)/perf/build/lib

source := $(shell find dh ecdh ntru -type f -name "*.c" -or -name "*.h")

.PHONY: perf xkcp dh ecdh ntru test benchmark clean

all: perf xkcp dh ecdh ntru

perf:
ifeq ($(shell uname), Linux)
	$(MAKE) -C perf
else
	echo "Warning: skipping Linux-only task 'perf' - may result in later build failure"
endif

xkcp:
	$(MAKE) -C xkcp plain-64bits/ua
ifeq ($(shell uname), Linux)
	$(MAKE) -C xkcp AVX2
else
	echo "Warning: skipping Linux-only tasks for xkcp - may result in later build failure"
endif

dh:
	$(MAKE) -C dh

ecdh:
	$(MAKE) -C ecdh

ntru:
	$(MAKE) -C ntru

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: $(source)
	clang-format -style=file -i $(source)

test:
	# Build tests
	$(MAKE) -C ntru tests
	$(MAKE) -C dh tests
	$(MAKE) -C ecdh tests

	# Run tests
	$(MAKE) -C ntru test
	$(MAKE) -C dh test
	$(MAKE) -C ecdh test

benchmark:
	# Build benchmarks
	$(MAKE) -C ntru benchmarks
	$(MAKE) -C dh benchmarks
	$(MAKE) -C ecdh benchmarks

	# Run benchmarks
	$(MAKE) -C ntru benchmark
	$(MAKE) -C dh benchmark
	$(MAKE) -C ecdh benchmark

clean:
	rm -rf build compile_commands.json &> /dev/null || true
	$(MAKE) -C dh clean || true
	$(MAKE) -C ecdh clean || true
	$(MAKE) -C ntru clean || true
	$(MAKE) -C xkcp clean || true
