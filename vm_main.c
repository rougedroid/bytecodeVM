#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>




typedef enum{
  WRITE_CONST_INT = 0x0020, // OPCODE [REG] [Value] Value is 16 bits so 15 bits of signed int. INT Range = -32768 to +32768.
  OP_NONE  = 0x0000, // SKIP
  OP_RETURN = 0x0022, // OPCODE [REG] Print out [REG]
  OP_ADD = 0x0024, // OPCODE [REG 1] [REG 2] -->Store in default regs/stack. --> default addr SMTN. then do 
  OP_LOAD_REG = 0x0025, // OPCODE [Source REG] [Destination REG]
  LOAD_REG = 0x0023, // OPCODE [REG] 
  OP_SUB = 0x0026, // OPCODE [REG 1] [REG 2] --> again put in default reg. have 1 bit for carry. 
  OP_JUMP = 0x0027, // OPCODE [REG]
  }Opcodes;

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
  //
  // DEBUG SETUP
  char in_commands[] = "[WRITE_CONST_INT][0000000100000100][0000000000000101][WRITE_CONST_INT][0000000100000101][0000000000000111][OP_ADD][0000000100000100][0000000100000101][OP_LOAD_REG][0000000000000001][0000000100000100][WRITE_CONST_INT][0000000100000101][0000000000000011][OP_ADD][0000000100000101][0000000100000100][OP_LOAD_REG][0000000000000001][0000000100000110]";
  const char delimeters[] = "[]";
  char * in_commands_bin = malloc(sizeof('o')*400);
  char *token = strtok(in_commands, delimeters);
  int instruction_len = 0;
  for (int i = 0; i < 400; i++){
    memcpy((in_commands_bin + i), "\0", 1);
  }
  while (token!=NULL){
    if (strcmp(token, "WRITE_CONST_INT") == 0)  {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100000", 16);
    }else if(strcmp(token, "OP_NONE") == 0)  {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000000000", 16);
    }else if( strcmp(token, "OP_RETURN")==0){
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100010", 16);

    }else if (strcmp(token, "OP_ADD")==0) {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100011", 16);
    }else if (strcmp(token, "OP_LOAD_REG")==0) {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100101", 16);
     
    }else if (strcmp(token, "OP_SUB")==0) {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100110", 16);
    
    }else if (strcmp(token, "OP_JUMP")==0) {
      memcpy(in_commands_bin + (instruction_len * 16), "0000000000100111", 16);
    }else {
      memcpy(in_commands_bin + (instruction_len * 16), token, 16);
    } 
    

    instruction_len++;
    token = strtok(NULL, delimeters);
  }
  printf("%s \n", in_commands_bin);
  
  // ACTUAL DECODER AND EXECUTER
  int total_char = strlen(in_commands_bin);
  instruction_len = total_char/16;

  for (int i = 0; i < instruction_len; i ++){
    char chunk[17];
    memcpy(chunk, in_commands_bin + (i*16), 16);
    chunk[16] = '\0';
    uint16_t numeric_value = (uint16_t)strtoul(chunk, NULL, 2);
    Opcodes op = (Opcodes)numeric_value;
    printf("Hex Value: 0x%04x\n", op);
    

    

    
  }
  


}
