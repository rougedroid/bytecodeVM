# Instruction Set Architecture Documentation

## Overview
This ISA defines a simple bytecode VM with fixed-size instructions and a flat address space.

## Instruction Format
- Each instruction is 2 bytes long.
- The VM reads instructions 2 bytes at a time.
- Operands are encoded after the opcode according to each instruction's operand layout.
- Values are generally processed as unsigned data internally, but some opcodes interpret values as signed when required.

## Memory and Address Space
- The VM uses a flat address space from `0x0000` to `0xFFFF`.
- Data blocks occupy `0x0000` to `0xFFF0` (65,521 bytes of changeable memory).
- Each pointer references 1 byte of data.
- The remaining high memory is reserved for runtime state, outputs, and special registers.
- The architecture is von Neumann style: code and data share the same memory space, so assembly must be carefully crafted.

## Error and Output Buffers
- Default operation output/error buffers are located in reserved high memory: `0xFFF0` to `0xFFF7`.
- Output buffer address range: `0xFFFE` to `0xFFFF` (only the last 8 bits are written here).
- These high-memory buffer locations are used as the final output path rather than a general-purpose register.

## OPCODES
| Opcode | Hex | Operands | Description |
|:---|:---:|:---|:---|
| `OP_NONE` | `0x0000` | none | No operation / skip |
| `WRITE_CONST_INT` | `0x0020` | `[REG] [Value]` | Write a 16-bit signed integer constant into register. Value range: -32768 to +32767. |
| `OP_RETURN` | `0x0022` | `[REG]` | Print the value stored in register. |
| `OP_LOAD_REG` | `0x0025` | `[Source REG] [Destination REG]` | Copy a register value from source to destination. |
| `LOAD_REG` | `0x0023` | `[REG]` | Load a register (details depend on implementation). |
| `OP_ADD` | `0x0024` | `[REG1] [REG2]` | Add two registers and store result in default register/stack. |
| `OP_SUB` | `0x0026` | `[REG1] [REG2]` | Subtract second register from first, store result in default register; carry bit available. |
| `OP_JUMP` | `0x0027` | `[REG]` | Jump to address contained in register. |
| `OP_CMP` | `0x0028` | `[REG1] [REG2] [JMP1] [JMP2]` | Compare registers; jump to `JMP1` if equal, otherwise jump to `JMP2`. |
| `OP_JMP_RELP` | `0x0029` | `[VALUE]` | Jump forward by an even number of bytes. |
| `OP_JMP_RELN` | `0x0030` | `[VALUE]` | Jump backward by an even number of bytes. |
| `OP_CMP_JMP` | `0x0031` | `[REG1] [REG2] [JMP]` | Compare registers and jump forward by `JMP` bytes if equal. |
| `OP_MUL` | `0x0032` | `[REG1] [REG2]` | Multiply registers; results stored in memory addresses `0xFFF7` and `0xFFF8`. |
| `OP_DIV` | `0x0033` | `[REG1] [REG2]` | Divide registers; quotient stored at `0xFFF9`/`0xFFFA`, remainder at `0xFFFB`/`0xFFFC`. |
| `OP_CMP_GTR` | `0x0034` | `[REG1] [REG2] [JMP1] [JMP2]` | If `REG1 > REG2`, jump to `JMP1`; otherwise jump to `JMP2`. |
| `OP_CMP_LSR` | `0x0035` | `[REG1] [REG2] [JMP1] [JMP2]` | If `REG1 < REG2`, jump to `JMP1`; otherwise jump to `JMP2`. |
| `OP_CMP_GTR_JMP` | `0x0036` | `[REG1] [REG2] [JMP1] [JMP2]` | Compare registers; if `REG1 > REG2` jump to `JMP1`, otherwise jump to `JMP2`. |
| `OP_CMP_LSR_JMP` | `0x0037` | `[REG1] [REG2] [JMP1] [JMP2]` | Compare registers; if `REG1 < REG2` jump to `JMP1`, otherwise jump to `JMP2`. |

## Reserved Result Addresses
- Addition result: `0xFFF1` 
- Subtraction result: `0xFFF3` 
- Overflow bits: `0xFFF5` 
- Multiplication result: `0xFFF7` 
- Division quotient: `0xFFF9`
- Division remainder: `0xFFFB` 

## Limitations
- The architecture reserves a large portion of high memory for special-purpose buffers and result locations, reducing the effective data space.
- Every instruction is fixed at 2 bytes, so small immediate values and simple register moves still consume a full instruction slot.
- Many operations require intermediary memory writes instead of direct register-to-register transfer, causing extra data movement and wasted cycles.
- Results for arithmetic operations are stored in reserved addresses, meaning the VM often needs extra instructions to move them from the result buffer to the final destination.
- Output is not emitted directly from a register; it must be staged through the reserved output buffer, which adds at least one extra copy step.
- Jump offsets must be even, which limits code density and wastes address space when values must be padded.
- The current design lacks a clearly defined register file and general-purpose temporary registers, so most data movement is handled through memory buffers.
- The memory model is byte-addressed while instructions are word-aligned, creating alignment overhead and possible wasted bytes.
- The signed constant range is limited to 16-bit values, and there is no explicit overflow handling beyond the reserved overflow bits.
- There is no explicit stack or call frame support, so complex control flow and nested procedure calls are hard to support efficiently.
- Because special-purpose result registers occupy fixed addresses, the architecture cannot easily scale to more complex operations without adding more reserved buffer space.

## Notes
- After changing the data block type, instructions may no longer require 2-byte operands for all data values, but the current format keeps the original 2-byte instruction alignment for compatibility.
- All jump offsets and values should be even when they represent byte distances in instruction memory.

