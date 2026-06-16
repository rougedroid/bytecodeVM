# BytecodeVM

A bytecode virtual machine written in C, implementing a simple 16-bit instruction set architecture with a flat memory address space (0x0000–0xFFFF) and fixed 2-byte instructions.

## Overview

BytecodeVM is a minimal but functional virtual machine designed to execute bytecode programs. It features:

- **16-bit instruction set** with fixed-size 2-byte instructions
- **Von Neumann architecture** (unified code/data memory space)
- **Flat address space** of 65,536 bytes (0x0000–0xFFFF)
- **Arithmetic operations** (add, subtract, multiply, divide)
- **Comparison and conditional jumps** for control flow
- **Register-based design** with registers stored in memory
- **Reserved high memory** (0xFFF0–0xFFFF) for output buffers and operation results

## Architecture Highlights

### Memory Layout
- **Total unified memory**: 0x0000–0xFFFF (131,072 bytes / 65,536 16-bit words)
- **Data/Code space**: 0x0000–0xFFF0 (65,521 usable memory locations)
- **Reserved high memory**: 0xFFF0–0xFFFF (~16 bytes, used for result buffers and special registers)
- **Result addresses**:
  - Addition result: 0xFFF1
  - Subtraction result: 0xFFF3
  - Multiplication result: 0xFFF7
  - Division quotient: 0xFFF9, remainder: 0xFFFB

### Output Buffer Note
The output buffer has its own address space location (0xFFFE–0xFFFF) but **outputs to a separate memory chunk in the C runtime**, not directly to the address space. Each instruction execution writes its output status to this separate buffer, which is printed after each instruction via `printf()`.

### Supported Instructions

| Opcode | Hex | Description |
|--------|-----|-------------|
| `OP_NONE` | 0x0000 | No operation (skip) |
| `WRITE_CONST_INT` | 0x0020 | Write 16-bit unsigned integer to register |
| `OP_LOAD_REG` | 0x0025 | Copy register value |
| `OP_RETURN` | 0x0022 | Print register value |
| `OP_ADD` | 0x0024 | Add two registers |
| `OP_SUB` | 0x0026 | Subtract two registers |
| `OP_MUL` | 0x0032 | Multiply two registers |
| `OP_DIV` | 0x0033 | Divide two registers |
| `OP_CMP` | 0x0028 | Compare registers with conditional jumps |
| `OP_JUMP` | 0x0027 | Unconditional jump to instruction address |
| `OP_JMP_RELP` | 0x0029 | Jump forward by instruction count |
| `OP_JMP_RELN` | 0x0030 | Jump backward by instruction count |
| `OP_CMP_GTR` | 0x0034 | Jump if greater than |
| `OP_CMP_LSR` | `0x0035` | Jump if less than |
| `OP_HALT` | `0x0038` | Stop execution immediately |

For detailed instruction specifications, see [ISA_Definition.md](ISA_Definition.md).

## Files

- `vm_main_1.c` — **Current implementation** with binary bytecode format and working instruction decoder
- `vm_main.c` — **Abandoned** legacy implementation that attempted string-based bytecode parsing; replaced by `vm_main_1.c` due to inefficiency and design issues
- `ISA_Definition.md` — Complete instruction set architecture documentation
- `TODO.md` — Outstanding tasks and improvements

## Compilation

Compile with any standard C compiler using the **current implementation**:

```sh
gcc -o bytecodevm vm_main_1.c
```

**Note:** `vm_main.c` is an abandoned legacy implementation that used string-based bytecode parsing and is no longer maintained. Use `vm_main_1.c` which implements the proper binary bytecode format.

## Usage

Run the executable:

```sh
./bytecodevm
```

### Input/Output

- **Input**: The VM reads binary bytecode from `bsp.out` file in the current working directory
- **Output**: After each instruction execution, the output buffer value is printed to stdout via `printf("Output Buffer Value: %d \n", outputbyte);`

Bytecode programs can:
- Load constants into registers
- Perform arithmetic operations
- Make conditional decisions based on comparisons
- Jump to different code sections
- Output results through the reserved output buffer

## Bytecode Format
`bsp.out` binary file must contain an even number of bytes to ensure proper 16-bit word alignment. All addresses and jump offsets are specified as **instruction counts** (number of 16-bit words), not byte offsets
Bytecode is stored as a sequence of **16-bit (uint16_t) words**. Each instruction consists of:
- A 16-bit opcode
- Zero or more 16-bit operands depending on the instruction

The default `bsp.out` file must contain an even number of bytes so the 16-bit instruction stream aligns correctly.

For example, the bytecode sequence to load a constant, perform a return, and exit:

```
0x0020 (WRITE_CONST_INT)
0x0104 (register address)
0x0105 (constant value)
0x0022 (OP_RETURN)
0x0104 (register address to print)
```

In `vm_main_1.c`, bytecode is represented as a `uint16_t` array and executed sequentially:

```c
uint16_t test_values[] = {
  0x0020, 0x0104, 0x0105,  // Load constant 0x0105 into register at 0x0104
  0x0022, 0x0104           // Print value from register at 0x0104
};
```

All addresses and jump offsets are specified as instruction indices (number of 16-bit words from the start of memory).

## Current Limitations

- Limited effective data space due to reserved high memory for special buffers and result registers
- Fixed 2-byte instruction format limits code density
- Arithmetic results stored in fixed memory addresses, requiring extra copy instructions
- No explicit stack or call frame support for nested procedures
- Limited signed constant range (16-bit only)

## Contributing

Contributions are welcome! Potential areas for improvement:

- Implement additional instruction opcodes
- Add comprehensive error handling for invalid bytecode
- Create sample bytecode programs and test suites
- Optimize the memory model and instruction decoder
- Add symbolic assembler for easier bytecode generation
- Implement debugger features (breakpoints, step execution)

## License

See [LICENSE](LICENSE) for license information.
