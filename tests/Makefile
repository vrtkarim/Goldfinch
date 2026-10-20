CC ?= cc
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -g
SANITIZE ?= 0

ifeq ($(SANITIZE),1)
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

INCLUDES = -I../engine/include -I../storage/include
HEADERS = $(wildcard ../engine/include/*.h ../storage/include/*.h)
SOURCES = test_engine.c \
          ../storage/src/dictionary.c \
          ../storage/src/hashtable.c \
          ../storage/src/triplestore.c \
          ../engine/src/engine.c

.PHONY: all test clean

all: test_engine

test_engine: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@

test: test_engine
	./test_engine

clean:
	rm -f test_engine test_engine.exe
