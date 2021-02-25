# Disable echoing of commands
MAKEFLAGS += --silent

.PHONY: ecdhe

ecdhe:
	$(MAKE) -C ecdhe

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

clean:
	rm -rf build compile_commands.json &> /dev/null || true
	$(MAKE) -C ecdhe clean
