# Disable echoing of commands
MAKEFLAGS += --silent

CPPFLAGS += -I $(CURDIR)/perf/build/include
LDFLAGS += -L $(CURDIR)/perf/build/lib/perf

source := $(shell find dh ecdh ntru -type f -name "*.c" -or -name "*.h")

.PHONY: perf xkcp dh ecdh ntru classic-mceliece test benchmark clean

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

classic-mceliece:
	$(MAKE) -C classic-mceliece

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: $(source)
	clang-format -style=file -i $(source)

# Build tests
tests: xkcp
	$(MAKE) -C ntru tests
	$(MAKE) -C classic-mceliece tests
	$(MAKE) -C dh tests
	$(MAKE) -C ecdh tests

# Run tests
test: tests
	$(MAKE) -C ntru test
	$(MAKE) -C classic-mceliece test
	$(MAKE) -C dh test
	$(MAKE) -C ecdh test

# Build benchmarks
benchmarks: perf xkcp
	# $(MAKE) -C ntru benchmarks
	# $(MAKE) -C classic-mceliece benchmarks
	# $(MAKE) -C dh benchmarks
	$(MAKE) -C ecdh benchmarks

# Build instrumented benchmarks
instrumented-benchmarks:
	LDFLAGS='$(LDFLAGS)' LDLIBS='$(LDLIBS) -lperf -lcap' CPPFLAGS='$(CPPFLAGS) -DINSTRUMENTED' CSOURCE='$(CSOURCE) $(realpath ./utilities/harness.c)' $(MAKE) benchmarks

# Run benchmarks
benchmark: benchmarks
	$(MAKE) -C ntru benchmark
	$(MAKE) -C classic-mceliece benchmark
	$(MAKE) -C dh benchmark
	$(MAKE) -C ecdh benchmark

hotpaths: xkcp
	$(MAKE) -C ntru hotpaths
	$(MAKE) -C classic-mceliece hotpaths

debug-flags:
	echo "export CFLAGS='-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-unused-command-line-argument -fsanitize=address -fno-omit-frame-pointer -g'"
	echo "export CC=clang"
	echo "export ASAN_OPTIONS=detect_leaks=1,symbolize=1"
	echo "export ASAN_SYMBOLIZER_PATH=$(shell which llvm-symbolizer)"

clean:
	rm -rf build compile_commands.json &> /dev/null || true
	$(MAKE) -C dh clean || true
	$(MAKE) -C ecdh clean || true
	$(MAKE) -C ntru clean || true
	$(MAKE) -C classic-mceliece clean || true
	$(MAKE) -C xkcp clean || true
	$(MAKE) -C perf clean || true
