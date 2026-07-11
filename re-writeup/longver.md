# ELF Symbol Table Resolution — Full Write-Up

Reverse engineering write-up — `newon.c` (custom ELF64 parser)
Includes disassembly of the compiled object file (`newon.o`), matched instruction-by-instruction back to the source.

---

## The code in question

```c
Elf64_Shdr *sym_section = &shdr_table[i];
Elf64_Shdr *str_section = &shdr_table[sym_section->sh_link];

Elf64_Sym *symbols = (Elf64_Sym *)(buffer + sym_section->sh_offset);
char *symbol_names = (char *)(buffer + str_section->sh_offset);
```

This sits inside a loop that walks every section header in the file, looking for one whose type is `SHT_SYMTAB` or `SHT_DYNSYM` — the two kinds of symbol tables an ELF file can have.

---

## Part 1: How `sh_link` finds the string table

Symbol names aren't stored next to the symbols. Each symbol just stores an offset (`st_name`) into a separate block of text. The ELF format tells you which block through a field called `sh_link`: for a symbol table section, `sh_link` holds the section number of its matching string table.

So the line `str_section = &shdr_table[sym_section->sh_link]` is doing one lookup: take the number stored in `sh_link`, and use it as an index into the array of section headers.

### What the disassembly shows

Here's the actual machine code for that line (addresses from the disassembly):

```
47c: mov  rax, [rbp-0x48]     ; rax = sym_section
480: mov  eax, [rax+0x28]     ; eax = sym_section->sh_link  (4-byte field)
483: mov  eax, eax            ; zero-extend to 64 bits
485: shl  rax, 0x6            ; rax = sh_link * 64
489: mov  rdx, rax
48c: mov  rax, [rbp-0x38]     ; rax = shdr_table (base address)
490: add  rax, rdx            ; rax = shdr_table + (sh_link * 64)
493: mov  [rbp-0x50], rax     ; store as str_section
```

`sh_link` is read as a plain 4-byte number at offset `0x28` inside the section header struct — that's exactly where the ELF spec puts it. The interesting part is the `shl rax, 0x6` right after: shifting left by 6 is the same as multiplying by 64, and 64 is `sizeof(Elf64_Shdr)`. That's the mechanical meaning of "index into an array of structs" — the compiler can't just add `sh_link` to the base address, because each section header takes up 64 bytes, so it has to convert the index into a byte offset first.

This is the second time I've caught this exact pattern in this program. The first time was earlier in `main`, computing `section_names` from `ehdr->e_shstrndx` (also a section index, also multiplied by 64 the same way). Seeing it happen twice, for two unrelated fields that both happen to be "the index of some other section," is a good confirmation that this is just how the compiler indexes any array of `Elf64_Shdr` — not something specific to symbol tables.

---

## Part 2: Turning offsets into usable pointers

Once `sym_section` and `str_section` are found, two pointers get built:

```
497: mov  rax, [rbp-0x48]     ; sym_section
49b: mov  rdx, [rax+0x18]     ; sym_section->sh_offset
49f: mov  rax, [rbp-0x28]     ; buffer
4a3: add  rax, rdx            ; buffer + sh_offset
4a6: mov  [rbp-0x58], rax     ; store as symbols

4aa: mov  rax, [rbp-0x50]     ; str_section
4ae: mov  rdx, [rax+0x18]     ; str_section->sh_offset
4b2: mov  rax, [rbp-0x28]     ; buffer
4b6: add  rax, rdx            ; buffer + sh_offset
4b9: mov  [rbp-0x60], rax     ; store as symbol_names
```

Both are the same shape: read `sh_offset` out of a section header (offset `0x18` in the struct), then add it to the address where the whole file was loaded into memory. That converts a "byte position in the file" into an actual pointer you can read from.

The two results are used very differently, though:

- `symbols` is treated as an array of fixed-size structs (`Elf64_Sym`), so later code can just do `symbols[j]`.
- `symbol_names` is treated as one long stretch of text with no structure at all — just names separated by null bytes. There's no `symbol_names[j]`; instead, each symbol stores a byte offset (`st_name`) into this blob, and the lookup is `symbol_names + symbols[j].st_name`.

That asymmetry — one side is a proper array, the other is just "start of a wall of text plus an offset" — is the core idea of how ELF stores names efficiently. Names are different lengths, so they can't be packed into a fixed-size array; a flat offset-addressed blob is the standard way around that.

---

## Part 3: How the symbol count is calculated

A few lines later, the code works out how many symbols are in the table:

```c
size_t symbol_count = sym_section->sh_size / sym_section->sh_entsize;
```

Disassembly:

```
4bd: mov  rax, [rbp-0x48]     ; sym_section
4c1: mov  rax, [rax+0x20]     ; sh_size
4c5: mov  rdx, [rbp-0x48]     ; sym_section (again)
4c9: mov  rcx, [rdx+0x38]     ; sh_entsize
4cd: mov  edx, 0x0            ; clear high half of the dividend
4d2: div  rcx                 ; rax = sh_size / sh_entsize
4d5: mov  [rbp-0x68], rax     ; store as symbol_count
```

Nothing hidden here — the C division becomes one `div` instruction, total table size divided by the size of one entry. The `mov edx, 0x0` right before it is just standard setup: `div` on x86-64 divides a 128-bit value (held across `rdx:rax`) by whatever's in the operand, so the high half has to be zeroed out first when you only care about a plain unsigned 64-bit division.

---

## Part 4: Indexing into the symbol array — a multiply without a multiply instruction

Later in the loop, when the code reaches for `symbols[j]`, it needs to convert `j` into a byte offset by multiplying by `sizeof(Elf64_Sym)`, which is 24 bytes. Since 24 isn't a power of two, the compiler can't just use one shift the way it did for the 64-byte section headers. Here's what it does instead:

```
576: mov    eax, [rbp-0xc]     ; eax = j
579: movsxd rdx, eax           ; sign-extend to 64 bits
57c: mov    rax, rdx
57f: add    rax, rax           ; rax = j * 2
582: add    rax, rdx           ; rax = (j*2) + j = j * 3
585: shl    rax, 0x3           ; rax = (j*3) * 8 = j * 24
```

Three times eight is twenty-four, so `(j × 3) × 8` gets the same answer as `j × 24`, using only shifts and adds instead of a slower multiply instruction. This is a common compiler trick: whenever a multiplier can be broken down into small factors, it's usually cheaper to do a couple of shifts and adds than to run an actual multiply. It's the same underlying idea as the `shl 0x6` seen earlier for the 64-byte section headers — just applied to a number that isn't a clean power of two, so it takes a few more steps.

---

## Part 5: `print_symbol_type` compiles to a jump table

Worth a quick mention since it's the other function in the file. The `switch` statement in `print_symbol_type` doesn't compile to a chain of `cmp`/`je` checks — it compiles to a jump table:

```
11: cmp  eax, 0x4        ; is type > 4 (out of the known range)?
14: ja   aa               ; if so, jump straight to the default case
1a: mov  eax, eax         ; zero-extend
1c: lea  rdx, [rax*4]     ; rdx = type * 4  (index into a table of 4-byte offsets)
24: lea  rax, [rip+0]     ; rax = address of the jump table
2b: mov  eax, [rdx+rax]   ; read the table entry for this type
2e: cdqe
30: lea  rdx, [rip+0]     ; base address for the offsets
37: add  rax, rdx         ; rax = actual target address
3a: jmp  rax               ; jump straight there
```

Instead of testing each `case` one at a time, the compiler built a small lookup table of addresses (one per known symbol type) and just jumps to the right one directly. This only kicks in because the `switch` has enough cases with small, contiguous values (`STT_NOTYPE` through `STT_FILE`, values 0–4) — a good sign of a "real" enum-driven switch rather than scattered `if` checks. The out-of-range guard (`cmp`/`ja`) before the jump handles the `default:` case for anything outside 0–4.

---

## A gap worth flagging

`str_section = &shdr_table[sym_section->sh_link]` has no bounds check on `sh_link` before it's used as an index. A malformed or adversarial ELF file could set `sh_link` to a number pointing outside the real section header table, causing an out-of-bounds read. Not a real risk here, since this parser is meant for files you control, but it's exactly the kind of check a hardened parser (or one meant to run against untrusted input) would need to add before doing the lookup.
