.PHONY: all test clean

all: test

test: build/test
	./build/test

build/test: test.c lib/*.c lib/*.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ test.c lib/*.c $(LDLIBS)

clean:
	rm -rf build &> /dev/null || true
