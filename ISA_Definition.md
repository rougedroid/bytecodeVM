# Instruction Set Architecture Documentation

## Overview
This ISA defines a simple bytecode VM with fixed-size instructions and a flat address space.
The implementation reads binary bytecode from `bsp.out` file during execution.

## Instruction Format
- Each instruction is 2 bytes (16 bits) long.
- The VM reads instructions sequentially as 2-byte words.
- Operands are encoded after the opcode according to each instruction's operand layout.
- Values are generally processed as unsigned data internally, but some opcodes interpret values as signed when required.

## Memory and Address Space
- The VM uses a unified memory from `0x0000` to `0xFFFF` (131,072 bytes total / 65,536 16-bit words).
- Usable data/code space: `0x0000` to `0xFFF0` (65,521 bytes of accessible memory).
- Each address points to 1 byte of data.
- The remaining high memory is reserved for runtime state, outputs, and special registers.
- The architecture is von Neumann style: code and data share the same memory space.

## Address and Jump Conventions
- All addresses are specified as **instruction indices** (position in the 16-bit word array), not byte offsets.
- For example, instruction at index 0 is the first 16-bit word, instruction at index 1 is the second 16-bit word, etc.
- Jump offsets in relative jump commands are also specified as instruction counts (number of words to advance/retreat).

## Error and Output Buffers
- Default operation output buffers are located in reserved high memory: `0xFFF0` to `0xFFFF`.
- The output buffer is stored in a **separate memory chunk in the C runtime** (via `malloc`), not directly in the address space.
- After each instruction, the output buffer value is printed to stdout.
- Result addresses:
  - Addition result: `0xFFF1`
  - Subtraction result: `0xFFF3`
  - Overflow bit: `0xFFF5`
  - Multiplication result: `0xFFF7`
  - Division quotient: `0xFFF9`, remainder: `0xFFFB`

## OPCODES
| Opcode | Hex | Operands | Description |
|:---|:---:|:---|:---|
| `OP_NONE` | `0x0000` | none | No operation / skip |
| `WRITE_CONST_INT` | `0x0020` | `[REG] [Value]` | Write a 16-bit unsigned integer constant into register. Value range: 0 to 65535. |
| `OP_RETURN` | `0x0022` | `[REG]` | Print the value stored in register. |
| `OP_LOAD_REG` | `0x0025` | `[Source REG] [Destination REG]` | Copy a register value from source to destination. |
| `LOAD_REG` | `0x0023` | `[REG]` | Load a register (details depend on implementation). |
| `OP_ADD` | `0x0024` | `[REG1] [REG2]` | Add two registers and store result in default register/stack. |
| `OP_SUB` | `0x0026` | `[REG1] [REG2]` | Subtract second register from first, store result in default register; carry bit available. |
| `OP_JUMP` | `0x0027` | `[INSTR_INDEX]` | Jump to instruction at index INSTR_INDEX. |
| `OP_CMP` | `0x0028` | `[REG1] [REG2] [INSTR1] [INSTR2]` | Compare registers; jump to INSTR1 if equal, otherwise jump to INSTR2 (both are instruction indices). |
| `OP_JMP_RELP` | `0x0029` | `[INSTR_COUNT]` | Jump forward by INSTR_COUNT instructions. |
| `OP_JMP_RELN` | `0x0030` | `[INSTR_COUNT]` | Jump backward by INSTR_COUNT instructions. |
| `OP_CMP_JMP` | `0x0031` | `[REG1] [REG2] [INSTR_COUNT]` | If REG1 ≠ REG2, jump forward by INSTR_COUNT instructions. |
| `OP_MUL` | `0x0032` | `[REG1] [REG2]` | Multiply registers; result stored in memory address `0xFFF7`. |
| `OP_DIV` | `0x0033` | `[REG1] [REG2]` | Divide registers; quotient stored at `0xFFF9`, remainder at `0xFFFB`. |
| `OP_CMP_GTR` | `0x0034` | `[REG1] [REG2] [INSTR_COUNT]` | If `REG1 > REG2`, jump forward by INSTR_COUNT instructions. |
| `OP_CMP_LSR` | `0x0035` | `[REG1] [REG2] [INSTR_COUNT]` | If `REG1 < REG2`, jump forward by INSTR_COUNT instructions. |
| `OP_CMP_GTR_JMP` | `0x0036` | `[REG1] [REG2] [INSTR_COUNT]` | If `REG1 > REG2`, jump forward by INSTR_COUNT instructions. |
| `OP_CMP_LSR_JMP` | `0x0037` | `[REG1] [REG2] [INSTR_COUNT]` | If `REG1 < REG2`, jump forward by INSTR_COUNT instructions. |
| `OP_HALT` | `0x0038` | none | Stop execution immediately. |

## Reserved Result Addresses
- Addition result: `0xFFF1` 
- Subtraction result: `0xFFF3` 
- Overflow bits: `0xFFF5` 
- Multiplication result: `0xFFF7` 
- Division quotient: `0xFFF9`
- Division remainder: `0xFFFB` 

## Limitations
- The architecture reserves high memory for special-purpose buffers and result locations, reducing effective data space.
- Every instruction is fixed at 2 bytes, so small immediate values and simple register moves consume a full instruction slot.
- Many operations require intermediary memory writes instead of direct register-to-register transfer.
- Results for arithmetic operations are stored in reserved addresses, requiring extra instructions to move them to final destinations.
- Output is not emitted directly from a register; it must be staged through the reserved output buffer (which lives in separate C memory).
- The current design lacks a clearly defined register file and general-purpose temporary registers.
- The memory model is byte-addressed while instructions are word-aligned.
- The signed constant range is limited to 16-bit values with no explicit overflow handling.
- There is no explicit stack or call frame support for nested procedure calls.
- Jump addresses cannot use arbitrary byte offsets; they must correspond to instruction boundaries.

## Notes
- After changing the data block type, instructions may no longer require 2-byte operands for all data values, but the current format keeps the original 2-byte instruction alignment for compatibility.
- All addresses and jumps are specified as instruction indices (position in the 16-bit word sequence), not byte offsets.
- The output buffer stores values in a separate C memory allocation, so writes to the 0xFFF0–0xFFFF address range produce side effects in the C runtime
