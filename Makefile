# Disable echoing of commands
MAKEFLAGS += --silent

CPPFLAGS += -I $(CURDIR)/perf/build/include
LDFLAGS += -L $(CURDIR)/perf/build/lib

source := $(shell find dh ecdh ntru -type f -name "*.c" -or -name "*.h")

.PHONY: perf ecdh ntru test benchmark clean

all: perf ecdh ntru

perf:
ifeq ($(shell uname), Linux)
	$(MAKE) -C perf
else
	echo "Warning: skipping Linux-only task 'perf' - may result in later build failure"
endif

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
	$(MAKE) -C ntru test
	$(MAKE) -C ecdh test

benchmark:
	$(MAKE) -C ntru benchmark
	$(MAKE) -C ecdh benchmark

clean:
	rm -rf build compile_commands.json &> /dev/null || true
	$(MAKE) -C ecdh clean
	$(MAKE) -C ntru clean
