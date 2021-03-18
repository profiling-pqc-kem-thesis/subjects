.PHONY: all test clean

LDLIBS += -l pthread

all: test

test: build/test
	./build/test

benchmark: build/benchmark
	@echo "=====       raw      ====="
	./build/benchmark 1000 0 0 0
	@echo "===== multi-process ======"
	./build/benchmark 1000 4 5 0

build/test: test.c lib/*.c lib/*.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ test.c lib/*.c $(LDLIBS)

build/benchmark: benchmark.c lib/*.c lib/*.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ benchmark.c lib/*.c $(LDLIBS)

clean:
	rm -rf build &> /dev/null || true
