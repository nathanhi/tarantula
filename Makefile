ifeq ($(OS),Windows_NT)
	RM = del /sfq
	LIBEND = .dll
	BINEND = .exe
else
	FPIC += -fPIC
	RM = rm -rf
	LIBEND = .so
	BINEND = 
endif

all: libtarantula
sample: libtarantula

libtarantula: src/tarantula.c
	$(CC) -c src/tarantula.c -o src/tarantula.o $(CFLAGS) -O2 -Wall -Werror $(FPIC)
	$(CC) -shared -o src/libtarantula$(LIBEND) src/tarantula.o

debug: src/tarantula.c
	$(CC) -c src/tarantula.c -o src/tarantula.o $(CFLAGS) -ggdb -Q -Wall -Werror $(FPIC)
	$(CC) -shared -o src/libtarantula$(LIBEND) src/tarantula.o

sample: sample/sample.c
	$(CC) sample/sample.c -o src/sample -Lsrc -ltarantula $(CFLAGS) -O2 -Wall -Werror -Wl,-rpath=$(PWD)/src

clean:
	$(RM) src/*$(LIBEND) src/*.o src/sample$(BINEND)
