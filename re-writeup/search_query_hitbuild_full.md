# RE Write-up: `search_query` — SearchHit Conversion & Scoring Loop

**Binary:** `search.o` (ELF64-x86-64)
**Function:** `search_query`
**Section analyzed:** `0x629`–`0x8cc` (final block of the function — building `SearchHit` results, sorting, returning)
**Source:** `search.c`, "Convert to SearchHit and attach a naive TF-based score" block

## What this code does

For every deduplicated `(doc_id, score)` pair produced earlier in `search_query`, this block:

1. Re-tokenizes the *original query string* and re-walks postings lists to compute a naive term-frequency score for that doc.
2. Grows a `SearchHit` array as needed and appends the result.
3. Sorts the finished array by score (descending) and returns it through the output parameter.

## Local variable map (stack offsets → source variables)

| Offset | Variable | Notes |
|---|---|---|
| `rbp-0x68` | `hits` | pointer, grows via `realloc` |
| `rbp-0x70` | `hcount` | |
| `rbp-0x78` | `hcap` | |
| `rbp-0x80` | `i` (outer loop index over `scores[]`) | |
| `rbp-0xbc` | `doc_id` (local int, per iteration) | |
| `rbp-0x88` | `s` (double accumulator) | |
| `rbp-0xc8` | `ql` (lowercased query dup) | |
| `rbp-0x90` | `tok` | |
| `rbp-0x110` / `rbp-0x118` | `plist` / `pc` (postings out-params) | |
| `rbp-0x98` | `b` (inner postings index) | |

## Outer loop: `0x64e`–`0x885`

```
0x64e–0x662   doc_id = scores[i].doc_id      ; scores stride = 0x10 (shl 0x4)
0x668–0x673   s = 0.0                        ; pxor xmm0,xmm0 (zeroing idiom)
0x674–0x683   ql = str_to_lower_dup(query)
0x68a–0x6a3   tok = strtok(ql, " ")
0x6aa         jmp to while-condition (0x7bd) — do-while-style loop shape
```

The compiler emitted the `while(tok)` loop as **test-at-bottom**: the body runs first, then jumps back up to `0x6af` only if the condition at `0x7bd` still holds. This is the standard GCC transformation for `while` loops — worth remembering so you don't misread it as a `do-while` in source.

### Inner `while(tok)` body: `0x6af`–`0x7c5`

```
0x6af–0x6ca   strcmp(tok, "or") == 0 ?
0x6cc–0x6e7      -> tok = strtok(NULL, " "); continue;
0x6ec–0x726   index_get_postings(idx, tok, &plist, &pc)
0x728         if false, skip inner scoring loop (je 0x7a2)
0x72a–0x7a0   for(b=0; b<pc; b++) if(plist[b].doc_id==doc_id) s += plist[b].tf;
0x7a2–0x7b6   tok = strtok(NULL, " ")
```

### ⚠️ Finding: `Posting.tf` is stored as an `int`, not a `double`

At `0x756`–`0x76e`:

```
75d: mov  rdx, QWORD PTR [rbp-0x98]     ; b
764: shl  rdx, 0x3                      ; b * 8  → Posting stride is 8 bytes
768: add  rax, rdx
76b: mov  eax, DWORD PTR [rax+0x4]      ; load 4-byte int at offset +4
76e: pxor xmm0, xmm0
772: cvtsi2sd xmm0, eax                 ; convert int → double
```

The stride confirms `Posting` is **8 bytes total**: a 4-byte `doc_id` at offset `0x0` and a 4-byte field at offset `0x4` — not a `double tf` (which would need 8-byte alignment and push the struct to 16 bytes with a stride of `shl 0x4`, like `SearchHit` below). The `cvtsi2sd` confirms the compiler is converting an **integer** to double before accumulating.

So despite `s += plist[b].tf` reading like straightforward double addition in the source, the underlying `Posting.tf` field is an integer (likely a raw term-frequency count), and the "TF score" is really `int → double` widening happening at the point of use. If you were assuming `tf` was already a `double` from the header, this is the kind of thing that only shows up once you check field offsets against real disassembly.

## Growth check and struct write: `0x7da`–`0x870`

```
0x7da–0x7de   if (hcount == hcap)
0x7e4–0x7f9      hcap = hcap ? hcap*2 : 16     ; ternary via branch, not cmov
0x7fd–0x817      hits = realloc(hits, hcap*0x10)
0x81b–0x82a   addr = hits + hcount*0x10        ; SearchHit stride = 16 bytes
0x82d–0x833   hits[hcount].doc_id = doc_id     ; offset +0x0
0x835–0x858   s > 0.0 ? s : 1.0                ; comisd + branch
0x859–0x86b   hits[hcount].score = result      ; offset +0x8
0x870         hcount++
```

`SearchHit` stride of `0x10` (16 bytes) with `doc_id` at `+0x0` and `score` at `+0x8` confirms the expected layout: 4-byte `int`, 4 bytes of padding for alignment, then an 8-byte `double`.

Note the `s > 0.0 ? s : 1.0` ternary at `0x835`–`0x858` is implemented as a **compare-and-branch** (`comisd` + `jbe`), not a conditional move. x86 has no direct `cmov` for XMM registers before certain SSE4.1-era tricks, so GCC falls back to branching for floating-point ternaries — a pattern worth recognizing so you don't expect `cmovg`-style codegen here the way you might for integer ternaries.

## Loop close and tail: `0x875`–`0x8cc`

```
0x875         i++
0x87a–0x885   cmp i, scount; jb 0x64e         ; outer loop back-edge
0x88b–0x895   free(scores)
0x89a–0x8b9   qsort(hits, hcount, 0x10, cmp_score_desc)
0x8a2         lea rdx, [rip+...] # cmp_score_desc  ; PIE-relative function pointer
0x8b9–0x8c4   *out_hits = hits
0x8c7         mov rax, [rbp-0x70]             ; return hcount
0x8cb–0x8cc   leave; ret
```

The `qsort` callback address at `0x8a2` is built with a `rip`-relative `lea`, not a hardcoded absolute address — expected for a PIE/PIC build, and confirms `cmp_score_desc` is resolved at load time via a relocation rather than baked in as a literal address.

## Design note (not a bug, but worth flagging in review)

The outer loop re-derives `ql` and re-tokenizes the *entire query string* from scratch on every single iteration of the outer `for(i=0;i<scount;i++)` loop, then frees it (`0x7cb`) before the next iteration re-allocates it. For `scount` matched documents and `m` query terms, that's `O(scount * m)` tokenizing work, most of it redundant — the source comment already flags this as "very naive," and the disassembly confirms the naivety is real at the machine level, not just a documentation note.

## Summary

This block is a clean instance of:
- struct-stride reasoning from `shl` immediates (`Posting` = 8 bytes, `SearchHit` = 16 bytes),
- catching a source-level assumption (`tf` "looks like" a double due to how it's used) that disassembly shows is actually an `int` converted at the use site,
- recognizing compiler-standard `while`-loop and ternary codegen shapes so they aren't mistaken for something else.
