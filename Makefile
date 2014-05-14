SOURCES = parse.c list.c atom.c eval.c tokens.c
OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -Wall -g
LDFLAGS =

CC = gcc
LD = gcc

# ifneq ($(BUILD_TEST),)

test: CFLAGS := $(CFLAGS) -DBUILD_TEST
test: $(OBJECTS) tst_main.o
	$(LD) $(LDFLAGS) -o $@ $^

# else

repl: $(OBJECTS) repl.o linenoise.o
	$(LD) $(LDFLAGS) -o $@ $^

# endif

.PHONY: clean
clean:
	rm -f *.o
	rm -f test
	rm -f repl
