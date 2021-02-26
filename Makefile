# Disable echoing of commands
MAKEFLAGS += --silent

CPPFLAGS += -I $(CURDIR)/perf/build/include
LDFLAGS += -L $(CURDIR)/perf/build/lib

.PHONY: perf ecdh ntru

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

clean:
	rm -rf build compile_commands.json &> /dev/null || true
	$(MAKE) -C ecdh clean
	$(MAKE) -C ntru clean
