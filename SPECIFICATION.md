# BytecodeVM - Hardware Specification

## Overview
BytecodeVM is a 16-bit virtual machine architecture implementing a von Neumann computing model with unified code/data memory and fixed-size 2-byte instructions.

## Core Specifications

### Instruction Word Size
- **Instruction width**: 16 bits (2 bytes)
- **Instruction format**: Fixed-length opcodes with variable-length operands
- **Instruction count**: Addressed by instruction index (0 to 65,535 for the 16-bit address space)

### Memory Architecture

#### Memory Organization
- **Total unified memory**: 0x0000–0xFFFF (131,072 bytes / 65,536 16-bit words)
- **Memory model**: Von Neumann (shared code/data space)
- **Address width**: 16 bits
- **Address space type**: Flat, linear addressing with byte granularity

#### Memory Layout
| Address Range | Size | Purpose |
|---|---|---|
| 0x0000–0xFFF0 | 65,521 bytes | General-purpose data/code storage |
| 0xFFF1 | 2 bytes | Addition result buffer |
| 0xFFF3 | 2 bytes | Subtraction result buffer |
| 0xFFF5 | 2 bytes | Overflow/carry bit flag |
| 0xFFF7 | 2 bytes | Multiplication result buffer |
| 0xFFF9 | 2 bytes | Division quotient result buffer |
| 0xFFFB | 2 bytes | Division remainder result buffer |
| 0xFFFE | 2 bytes | Output buffer address (reserved) |

**Total reserved high memory**: ~16 bytes (0xFFF0–0xFFFF)

### Data Types

#### Supported Data Types
- **Unsigned integers**: 16-bit (range: 0 to 65,535)
- **Addresses**: 16-bit instruction indices for jumps, byte addresses for data

#### No Explicit Support For
- Signed integers (all values treated as unsigned)
- Floating-point numbers
- Strings or character arrays (must be implemented in application code)
- Bit-level operations (beyond integer arithmetic)

### Execution Model

#### Program Counter
- **Width**: 16 bits
- **Unit**: Instruction indices (each increment moves to the next 16-bit word)
- **Initial value**: 0
- **Behavior**: Linear increments except during jump operations

#### Fetch-Execute Cycle
1. Fetch 16-bit instruction from memory at current program counter
2. Decode opcode and operands
3. Execute instruction
4. Write output buffer value to stdout
5. Increment program counter
6. Repeat until HALT instruction

#### Jump Semantics
- **Absolute jumps**: Specify target instruction index directly (OP_JUMP, OP_CMP with absolute targets)
- **Relative jumps**: Specify offset as instruction count, added/subtracted from current position (OP_JMP_RELP, OP_JMP_RELN)
- **Conditional jumps**: Jump forward by instruction count if condition met (OP_CMP_GTR, OP_CMP_LSR, etc.)
- **Jump resolution**: Program counter set to target index; +1 automatically happens in main loop

### I/O Architecture

#### Input
- **Source**: Binary file named `bsp.out` in current working directory
- **Format**: Raw binary stream of 16-bit words
- **Alignment**: Must contain even number of bytes (multiple of 2)
- **Loading**: Entire file read into memory at startup, copied to address space starting at 0x0000

#### Output
- **Mechanism**: After each instruction execution, the output buffer value is printed to stdout
- **Format**: Decimal integer value via `printf("Output Buffer Value: %d \n", value);`
- **Buffer location**: Separate C runtime memory allocation (not directly in address space, despite reserved address)

### Instruction Set Summary

#### Instruction Classes

**No-Op and Control**
- `OP_NONE` (0x0000): No operation
- `OP_HALT` (0x0038): Stop execution

**Data Movement**
- `WRITE_CONST_INT` (0x0020): Load 16-bit constant into memory location
- `OP_LOAD_REG` (0x0025): Copy value from one memory location to another

**Arithmetic Operations**
- `OP_ADD` (0x0024): Add two values, result → 0xFFF1
- `OP_SUB` (0x0026): Subtract two values, result → 0xFFF3
- `OP_MUL` (0x0032): Multiply two values, result → 0xFFF7
- `OP_DIV` (0x0033): Divide two values, quotient → 0xFFF9, remainder → 0xFFFB

**Comparison and Branching**
- `OP_CMP` (0x0028): Compare two values, absolute jump based on equality
- `OP_CMP_GTR` (0x0034): Jump forward if first > second
- `OP_CMP_LSR` (0x0035): Jump forward if first < second
- `OP_CMP_JMP` (0x0031): Conditional relative jump on inequality
- `OP_CMP_GTR_JMP` (0x0036): Conditional relative jump on greater-than
- `OP_CMP_LSR_JMP` (0x0037): Conditional relative jump on less-than

**Unconditional Branching**
- `OP_JUMP` (0x0027): Absolute jump to instruction index
- `OP_JMP_RELP` (0x0029): Jump forward by instruction count
- `OP_JMP_RELN` (0x0030): Jump backward by instruction count

**Output**
- `OP_RETURN` (0x0022): Read value from memory and write to output buffer

**Total Instruction Count**: 18 opcodes

### Operand Encoding

#### Operand Sizes
- All operands are 16-bit (2 bytes)
- Operands are zero-extended or sign-extended as needed
- No sub-word operand access

#### Operand Types
- **Register/Memory addresses**: 16-bit address indices
- **Constants**: 16-bit signed integers (-32,768 to +32,767)
- **Jump targets**: 16-bit instruction indices (for absolute jumps)
- **Jump offsets**: 16-bit instruction counts (for relative jumps)

### Performance Characteristics

#### Instruction Latency
- All instructions execute in a single cycle (no pipelining)
- Memory access is uniform (same latency for all addresses)
- Output buffer write: Single cycle

#### Code Density
- Minimum instruction: 2 bytes (opcode only, e.g., OP_HALT, OP_NONE)
- Maximum instruction: 10 bytes (opcode + 4 operands, e.g., OP_CMP with 4 address/jump operands)
- Average instruction: 4-6 bytes (opcode + 1-2 operands)

#### Memory Throughput
- Instruction fetch: 1 word (2 bytes) per cycle
- Operand fetch: Up to 4 words per instruction
- Result storage: 1 write per instruction (result buffer)

### Architecture Constraints and Design Decisions

#### Fixed-Size Instructions
- All instructions are word-aligned (16-bit boundaries)
- Benefits: Predictable decoding, simplified address calculation
- Cost: Code density (cannot pack multiple small instructions)

#### Unified Memory
- Code and data share address space
- Benefits: Flexibility (can load code dynamically, self-modifying code possible)
- Cost: Security concerns, potential code corruption

#### Reserved High Memory
- ~16 bytes reserved for result buffers and special registers
- Reduces effective data space to 65,521 bytes
- Benefits: Deterministic result access, simplified architecture
- Cost: Limited addressable memory, inflexible result storage

#### Separate Output Buffer
- Output buffer stored in separate C runtime memory, not address space
- Benefits: Prevents accidental data corruption via address space writes
- Cost: Cannot directly access output buffer via load/store instructions

#### Byte-Addressed Data, Word-Addressed Instructions
- Data memory: Byte granularity (8 bits per address)
- Instructions: Word granularity (16 bits per instruction index)
- Semantic: Simplifies program counter increment, complicates mixed usage

### Limits and Ranges

| Metric | Value |
|---|---|
| Maximum address | 0xFFFF (65,535) |
| Maximum instruction index | 0xFFFF (65,535) |
| Maximum constant value | 0xFFFF (65,535) |
| Minimum constant value | 0x0000 (0) |
| Maximum memory allocation | 65,536 16-bit words (131,072 bytes) |
| Usable data space | 65,521 bytes |
| Program counter width | 16 bits |

### Future Enhancement Possibilities

- **Extended instruction set**: 32-bit or 64-bit instructions
- **Stack architecture**: Explicit stack pointer and stack operations
- **Register file**: Dedicated general-purpose registers (instead of memory-based)
- **Memory protection**: Separate code and data address spaces (Harvard architecture)
- **Hardware features**: Interrupts, exceptions, privilege modes
- **Optimization**: Pipelining, instruction caching, branch prediction
- **Debugging support**: Breakpoints, single-step execution, performance counters

---

**Version**: 1.0  
**Last Updated**: 2026-06-16  
**Status**: Current implementation complete; memory management enhancements planned
