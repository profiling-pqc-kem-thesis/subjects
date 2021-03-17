.PHONY: all test clean

all: test

test: build/test
	./build/test

benchmark: build/benchmark
	./build/benchmark 100000 0

build/test: test.c lib/*.c lib/*.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ test.c lib/*.c $(LDLIBS)

build/benchmark: benchmark.c lib/*.c lib/*.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ benchmark.c lib/*.c $(LDLIBS)

clean:
	rm -rf build &> /dev/null || true
