ifeq ($(OS),Windows_NT)
	RM = del /s /f /q
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
.PHONY: debug
debug: CFLAGS += -DDEBUG -g
debug: all

posix: 
	$(CC) -c src/posix.c -o src/posix.o $(CFLAGS) -O2 -Wall -Werror $(FPIC)

libtarantula: posix src/tarantula.c
	$(CC) -c src/tarantula.c -o src/tarantula.o $(CFLAGS) -O2 -Wall -Werror $(FPIC) -std=c99
	$(CC) -shared -o src/libtarantula$(LIBEND) src/posix.o src/tarantula.o

sample: sample/sample.c
	$(CC) sample/sample.c -o src/sample -Lsrc -ltarantula $(CFLAGS) -std=c99 -O2 -Wall -Werror

clean:
	$(RM) src/*$(LIBEND) src/*.o src/sample$(BINEND)
