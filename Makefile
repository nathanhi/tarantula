CC=clang-3.7
CFLAGS=

all: libtarantula
sample: libtarantula

libtarantula: src/tarantula.c
	$(CC) -c src/tarantula.c -o src/tarantula.o $(CFLAGS) -O2 -Wall -Werror -fpic -std=c11
	$(CC) -shared -o src/libtarantula.so src/tarantula.o

debug: src/tarantula.c
	$(CC) -c src/tarantula.c -o src/tarantula.o $(CFLAGS) -ggdb -Q -Wall -Werror -fpic -std=c11
	$(CC) -shared -o src/libtarantula.so src/tarantula.o

sample: sample/sample.c
	$(CC) sample/sample.c -o src/sample -Lsrc -ltarantula -O2 -Wall -Werror -Wl,-rpath=$(PWD)/src -std=c11

clean:
	rm -rf src/*.so src/*.o src/sample
