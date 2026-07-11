# minisearch

A small modular C project that indexes all `.txt` files under a directory and lets you run simple queries.

## Features
- Recursive directory walk to discover `.txt` files
- Tokenization (lowercased, split on non-alphanumeric)
- Inverted index: word -> list of (doc_id, term_frequency)
- Query engine with **AND** (default) and **OR** operators
- Ranked results by simple TF (term frequency) sum
- Logging and configuration
- Unit tests for core pieces

## Build
```bash
make
```
This builds two binaries:
- `bin/minisearch` – the CLI
- `bin/tests` – unit tests

## Run
Index a directory and search:
```bash
./bin/minisearch --root ./sample_docs --query "memory pointer OR stack"
```

Run tests:
```bash
./bin/tests
```

## Project layout
```
include/
  config.h        logger.h        tokenizer.h    hashmap.h    index.h    filewalker.h    search.h    strutil.h
src/
  main.c          config.c        logger.c       tokenizer.c  hashmap.c  index.c         filewalker.c search.c strutil.c
tests/
  test_main.c
sample_docs/      (created on first run with small example files)
Makefile
```
