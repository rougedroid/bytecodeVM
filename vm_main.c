#include <stdio.h>
#include <stdlib.h>



typedef enum(
  WRITE_CONST_INT = 0x0020, // OPCODE [REG] [Value] Value is 16 bits so 15 bits of signed int. INT Range = -32768 to +32768.
  OP_NONE  = 0x0000, // SKIP
  OP_RETURN = 0x0022, // OPCODE [REG] Print out [REG]
  OP_ADD = 0x0024, // OPCODE [REG 1] [REG 2] -->Store in default regs/stack. --> default addr SMTN. then do 
  OP_LOAD_REG = 0x0025, // OPCODE [Source REG] [Destination REG]
  LOAD_REG = 0x0023, // OPCODE [REG] 
  OP_SUB = 0x0026, // OPCODE [REG 1] [REG 2] --> again put in default reg. have 1 bit for carry. 
  OP_JUMP = 0x0027, // OPCODE [REG]
)Opcodes;

// We will have it act like a chip only. we will have all the input also "take" us chips space. basically all the code is stored in data lines. 
//
// Total chip data addresses: 0x0000 to 0xFFF0 -> 65520 bits last one excluded 
// OUTPUT PIN ADDRESSES: 0xFFF0 - 0xFFFF -> 16 bits both included
//

// 16-BIT Operating System 
// Basic Turing Machine:
// Jump To commands.
// If-Else Statements
// Add Sub 
// REG Addresses-> 0x0100 - 0x01FF 
//
int main(){
  // X = 5 + 7
  // Y = X + 3 
  // X --> 0x0104 
  //
  // Y --> 0x0106 
  //
  // OP_ADD gives out at --> 0x0001
  // OP_SUB gives out at --> 0x0002 and Sets REMAINDER BIT ON 0x0003
  //
  // 
  
  in_commands = "[WRITE_CONST_INT][0x0104][0x0005][WRITE_CONST_INT][0x0105][0x0007][OP_ADD][0x0104][0x0105][OP_LOAD_REG][0x0001][x0104][WRITE_CONST_INT][0X0105][0X0003][OP_ADD][0X0105][0X0104][OP_LOAD_REG][0x0001][0x0106]";
  

}
