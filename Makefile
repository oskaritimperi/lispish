SOURCES = parse.c atom.c eval.c tokens.c env.c
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(foreach obj,$(OBJECTS),test_$(obj))

CFLAGS = -Wall -Wextra -g
LDFLAGS =

CC = gcc
LD = gcc

all: test repl

test_%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

test: CFLAGS := $(CFLAGS) -DBUILD_TEST
test: $(TEST_OBJECTS) tst_main.o
	$(LD) $(LDFLAGS) -o $@ $^

repl: $(OBJECTS) repl.o linenoise.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o
	rm -f test
	rm -f repl
