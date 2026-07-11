# ELF Parser

A simple command-line tool written in C that reads and parses the header of an ELF (Executable and Linkable Format) binary file.

It extracts and displays key metadata such as:

- Magic number
- File class (32-bit / 64-bit)
- Endianness
- Entry point address
- ELF type
- Machine architecture

This project is part of my learning process in C programming, Linux systems, and reverse engineering. My current focus is on writing functional code, so this parser reflects a practical implementation with a basic understanding of the ELF format — I'll continue improving it as I deepen my knowledge.

## Goals

This project isn't meant to be a full-featured ELF tool — the goal is to:

- Understand how binary files are structured
- Practice reading raw bytes from files
- Work with structs and memory layout in C
- Begin exploring reverse engineering concepts

## Features

- Validates ELF magic number
- Identifies ELF class (ELF32 / ELF64)
- Displays entry point address
- Reads header fields directly from file
- Minimal and focused implementation

## Requirements

- GCC (or any C compiler)
- Linux environment (recommended)

## Build & Run

```bash
gcc -o elf_parser elf_parser.c
./elf_parser <path_to_elf_file>
```

## Example

```
$ ./elf_parser /bin/ls
 magic   7f  45  4c  46  02  01  01  00  00  00  00  00  00  00  00  00
Class: ELF64
Data: Little endian
Type: Shared object
Machine: x86-64
Entry point: 0x61d0
Section header offset: 149360
Number of program headers: 13
Number of section headers: 31
Section header string table index: 30
```


## Limitations

- Only parses the ELF header (not sections or symbols)
- Limited error handling
- Assumes valid ELF file structure

## Roadmap

- Parse section header table
- Display section names and sizes
- Add symbol table parsing
- Improve error handling

## Disclaimer

This tool is for educational purposes only.

## Author

Jose Mancera
