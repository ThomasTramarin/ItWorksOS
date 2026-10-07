# Syscall (x86)

## System Call ABI

IWOS provides system calls through the `int 0x80` interrupt.

The system call number is passed in `EAX`, while arguments (max 5 args) are passed through the remaining general-purpose registers.

All arguments and return values are represented as 32-bit values. Their interpretation depends on the individual system call.

The kernel dispatches the requested system call and stores its return value in `EAX`.

| Register | Purpose |
| --- | --- |
| `EAX` | Syscall number / return value |
| `EBX` | Argument 1 |
| `ECX` | Argument 2 |
| `EDX` | Argument 3 |
| `ESI` | Argument 4 |
| `EDI` | Argument 5 |

