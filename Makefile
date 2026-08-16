# RimLang Makefile (C17 Standard)
CC ?= gcc
CFLAGS ?= -std=c17 -Wall -Wextra -O2 -Iinclude -D_GNU_SOURCE

SRCS = src/parser/lexer.c \
       src/parser/parser.c \
       src/parser/korean_nlp.c \
       src/parser/korean_synonym.c \
       src/runtime/runtime.c \
       src/runtime/joke_table.c \
       src/engine/engine.c

OBJS = $(SRCS:.c=.o)

all: rimlang test_runner

rimlang: src/main.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test_runner: tests/test_runner.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: test_runner
	./test_runner

clean:
	rm -f rimlang test_runner src/*.o src/*/*.o tests/*.o

.PHONY: all test clean
