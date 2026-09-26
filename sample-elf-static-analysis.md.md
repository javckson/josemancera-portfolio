# Static Analysis of an x86-64 Linux Botnet Sample

## Executive summary

I analyzed a stripped, statically linked Linux ELF sample in Ghidra without executing it. The code traces in my notes show a routine that builds random IPv4 targets, connects to ports associated with Telnet and HTTP, and sends hardcoded credential or HTTP request data over sockets. The binary also contains command strings for network attack modes; I traced one `TCPNUKE` parsing branch to a handler call. These findings are consistent with an IoT botnet that attempts to spread through exposed services and accepts attack commands. The available analysis does not establish a particular malware family, successful compromise, or what the remote second-stage script actually does.

**SHA-256:** `29e9a9a5caa9f7be607f87ed24d4bba51c9b2ae37cc1fd263e82af00a99525cb`  
**Format:** ELF 64-bit, x86-64, statically linked and stripped, targeting GNU/Linux 3.2.0  
**Method:** Static analysis in Ghidra inside an isolated Debian VM; no execution or network observation  
**Analysis date:** September 2026

## Scope and method

I examined embedded data, cross-references, decompiled control flow, and assembly around relevant call sites. For each behavioral conclusion, I looked for a path from the data to the function that uses it and checked the arguments at the call. An embedded string can indicate intent, but it cannot by itself prove that a feature executes or succeeds. Function names such as `FUN_00402ae0` are Ghidra's generated labels, not symbols recovered from the binary.

The supporting material consists of two analysis notes, not the executable or a full Ghidra project. Addresses and reconstructed flows below are transcribed from those notes. I did not independently rerun Ghidra or validate the hash against sample bytes for this write-up.

## Findings

### 1. Scanning and outbound connections

The routine labeled `FUN_00402ae0` creates an IPv4 TCP socket through `FUN_0045ee40(2, 1, 0)` and builds a target socket address. Its local port list contains `0x17`, `0x50`, `0x1f90`, and `0x51`, corresponding to 23, 80, 8080, and 81. The decompiled routine calls `FUN_004115e0` four times and formats the results as `%d.%d.%d.%d`; the notes describe this as random target generation. The resulting address is used in a call to `FUN_0045ead0`, identified in the notes as a `connect` wrapper for syscall 42.

The routine's loop and target construction support the conclusion that the program attempts repeated outbound connections to generated IPv4 addresses. The notes do not establish the distribution of generated addresses, exclusions, connection success rate, or whether any real target was reached.

### 2. Credential attempts and HTTP exploit requests

The port-23 branch sends a fixed sequence containing `root` and `admin` alongside common passwords. That is evidence of a Telnet credential attempt. The notes do not show a complete login transcript or confirm successful authentication, so I describe this as an attempt rather than a successful brute-force infection.

The HTTP branches contain request templates resembling three known exploitation patterns:

| Template observed in the binary | Interpretation | Limit |
| --- | --- | --- |
| ThinkPHP invocation path with `call_user_func_array` and `shell_exec` | Attempted command execution through a ThinkPHP endpoint | No successful server response was observed. |
| SOAP `AddPortMapping` request to `/picsdesc.xml` with a command in `NewInternalClient` | Attempted command injection through a router UPnP implementation | The specific vulnerable device and CVE were not verified. |
| SOAP request to `/ctrlt/DeviceUpgrade_1` with a command in `NewStatusURL` | Pattern associated with Huawei HG532 / CVE-2017-17215 | The request template alone does not show a successful exploit. |

The HTTP data is associated in the notes with the same scanning routine and socket send wrapper. The recorded trace is most explicit for the shared shell-command string discussed below; each HTTP branch would benefit from its own saved call-site excerpt before claiming all three requests were individually verified end to end.

### 3. Second-stage download command is transmitted

At `0x004d0080`, the notes record a shell command that tries several writable directories, then uses `wget`, `curl`, or BusyBox `wget` to fetch `zombie_scanner.sh` from `178.132.198.200` and pipe it to a shell. I have omitted the executable command text here because the file path, destination, and delivery mechanism are enough to explain the finding.

The strongest trace is from this string to a network send. A pointer at `0x00503138` leads into `FUN_00402ae0`; the recorded sequence places the string pointer in `RSI`, the socket descriptor in `EDI`, and a calculated length in `RDX` before calling `FUN_0045ec70`. That wrapper contains `MOV EAX, 0x2c` followed by `SYSCALL`, consistent with Linux x86-64 `sendto` (syscall 44). This supports that the command text is sent over a socket at this call site. It does **not** mean the binary runs that command locally, that a remote device accepted it, or that the script was downloaded.

The staging IP and script name are indicators embedded in the sample. Their presence does not establish that the IP is the program's command-and-control server; in this trace it is the script download destination within a command sent to another host.

### 4. An attack-mode command reaches a handler

The binary contains `TCPNUKE ` at `0x004cf279` and the format string `TCPNUKE %31s %d %d` at `0x004cf282`. The format string is referenced at `0x0040297f`. The recorded call passes an input buffer and output fields to `FUN_004123a0`, described in the notes as an `sscanf`-equivalent. The branch compares the return value with three, then calls `FUN_004055d0` with the parsed fields when all three conversions succeed.

This demonstrates that `TCPNUKE` is wired into a parsing branch and reaches a handler with user-supplied parameters. The handler's packet-level behavior, the meaning of the two integers, and the source of the input buffer were not established in the supplied traces. Other labels including `UDPBYPASS`, `DISCORD`, and `RAKNET` have similar cross-reference patterns, but their handlers were not traced individually. Calling the binary's network attack capability plausible is justified; claiming a demonstrated, working DDoS attack for each mode would require more analysis.

## Assessment and confidence

| Conclusion | Confidence from the supplied notes | Basis |
| --- | --- | --- |
| Repeated attempts to connect to generated IPv4 targets on ports 23, 80, 81, and 8080 | High | Port array, target formatting, socket creation, connect call, and loop described in one routine. |
| Embedded download-and-execute text is transmitted over a socket | High | Pointer and register flow reaches syscall 44. |
| HTTP request templates are intended for exploitation | Moderate to high | Exploit-shaped requests and association with the scan routine; individual send traces were not supplied for every template. |
| `TCPNUKE` has a reachable parsing branch and handler call | High for the call path; lower for its actual network effects | Format-string cross-reference, conversion count check, and handler call. |
| Exact Mirai-family attribution | Unconfirmed | Similar behavior is insufficient for family attribution without comparative code, configuration, or independent reference. |
| Compromise, second-stage execution, or DDoS traffic in the wild | Not observed | No dynamic execution or network capture. |

**Overall assessment:** The sample is consistent with a Linux/IoT botnet containing scanning, credential-attempt, exploit-delivery, and attack-command components. This is a static assessment of capability and apparent intent, not a report of observed activity.

## Indicators and next checks

| Type | Value or pattern | Role in this analysis |
| --- | --- | --- |
| SHA-256 | `29e9a9a5caa9f7be607f87ed24d4bba51c9b2ae37cc1fd263e82af00a99525cb` | Sample identifier supplied in the notes. |
| IPv4 | `178.132.198.200` | Embedded second-stage script host; current ownership or activity not checked. |
| Script path | `/zombie_scanner.sh` | Embedded in transmitted download command. |
| Service ports | 23, 80, 81, 8080 | Destinations selected by the scanning routine. |
| Request paths | `/picsdesc.xml`, `/ctrlt/DeviceUpgrade_1`, ThinkPHP invocation path | Exploit-template artifacts, not proof of successful requests. |

For a follow-up, I would preserve screenshots or assembly excerpts for each individual HTTP send call, inspect `FUN_004055d0` to characterize the `TCPNUKE` handler, and check how incoming commands reach the parser. Comparative code evidence would be needed before naming a malware family. This report contains no live execution results.

## Source notes

Prepared from `malware-analysis-29e9a9a5.md` and `sample29e9aclaude.md`, supplied with the project. The earlier continuation notes marked several strings as hypotheses pending cross-reference checks; the later write-up recorded traces for the scanner, command transmission, and one attack-mode parsing branch. Where the two notes differ in certainty, this report uses the narrower supported claim.
