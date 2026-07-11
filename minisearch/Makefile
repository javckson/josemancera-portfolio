CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -Iinclude
BINDIR  := bin

# Everything in src/ except main.c is shared between the CLI and the tests.
SRC        := $(wildcard src/*.c)
LIB_SRC    := $(filter-out src/main.c, $(SRC))

MAIN_SRC   := src/main.c
TEST_SRC   := tests/test_main.c

.PHONY: all clean run test

all: $(BINDIR)/minisearch $(BINDIR)/tests

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/minisearch: $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

$(BINDIR)/tests: $(LIB_SRC) $(TEST_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(LIB_SRC) $(TEST_SRC)

run: $(BINDIR)/minisearch
	./$(BINDIR)/minisearch --root ./sample_docs --query "memory pointer OR stack"

test: $(BINDIR)/tests
	./$(BINDIR)/tests

clean:
	rm -rf $(BINDIR)
