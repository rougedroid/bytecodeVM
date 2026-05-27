#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>



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
// Total chip data addresses: 0x0000 to 0xFFF0 -> 65521*4 bits => each address is 1 Byte last one excluded.
// OUTPUT PIN ADDRESSES: 0xFFF0 - 0xFFFF -> 16 bits both included
// We put the actual code from 0x0000 onlt. and change the default output buffers to the ending data blocks instead. 

// 16-BIT Architecture 
// Basic Turing Machine:
// Jump To commands.
// If-Else Statements
// Add Sub 
// REG Addresses-> 0x0100 - 0x01FF 
//
//
void * datablocks;
void * outputbuffer;


void init(){
  datablocks = malloc(sizeof(char)*65521); // Getting address space for the "Data" i.e. 65521 bytes 
  outputbuffer = malloc(sizeof(char)*16); // Getting output buffer -> 16 bytes. Note: 16 bytes is useless, i only want to output one byte, a number. but its not "elegant" to fix it cuz i won't be using the full 16 bit address space so meh let it be 
  memset(datablocks, 0, 65521);
  memset(outputbuffer, 0, 16);
  memset(datablocks, 1, 5000); //test Value

//  memset(datablocks, 2, 5001); //test Value
//  memset(datablocks, 3, 4999); //test Value
//  memset(datablocks, 4, 5002); //test Value
//  memset(datablocks, 5, 4998); //test Value
}

//uint16_t * read_next()

void buffer_return(int * output){
  unsigned char last_byte = *output & 0xFF;
  memcpy( outputbuffer, &last_byte, 1);

}

int main(){
  init();
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
  //char in_commands[] = "[WRITE_CONST_INT][0000000100000100][0000000000000101][WRITE_CONST_INT][0000000100000101][0000000000000111][OP_ADD][0000000100000100][0000000100000101][OP_LOAD_REG][0000000000000001][0000000100000100][WRITE_CONST_INT][0000000100000101][0000000000000011][OP_ADD][0000000100000101][0000000100000100][OP_LOAD_REG][0000000000000001][0000000100000110]";
  //
  // NOTE: 
  // SWITCH TO ACTUAL BINARY. ADAPT CURRENT SYSTEM TO BINARY.
  char in_commands[] = "[OP_NONE][OP_RETURN][0001001110001000]";
  const char delimeters[] = "[]";
  char * in_commands_bin = malloc(sizeof(char)*400);
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
  int datapointer = 0x0000;

  memcpy(datablocks, in_commands_bin, strlen(in_commands_bin));
  datapointer += strlen(in_commands_bin);
  // ACTUAL DECODER AND EXECUTER
  int total_char = strlen(in_commands_bin);
  instruction_len = total_char/16;
  int processed_instructions = 0;
  // OUTPUT OP_Codes for verification
  //reset pointer ( like rebooting a chip or resetting smnt man)
  datapointer = 0x0000;
  for (int i = 0; i < instruction_len; i ++){
    char chunk[17];
    memcpy(chunk, in_commands_bin + (i*16), 16);
    chunk[16] = '\0';
    uint16_t numeric_value = (uint16_t)strtoul(chunk, NULL, 2);
    Opcodes op = (Opcodes)numeric_value;
    printf("Hex Value: 0x%04x\n", op);

    if (op == OP_NONE) {
      int * output= malloc(sizeof(1));
      buffer_return(output);
      
      free(output);
    }else if (op ==  OP_RETURN){
      int * output = malloc(sizeof(1));
      uint16_t * addr = malloc(sizeof(uint16_t));

      memcpy(chunk, in_commands_bin+(i*16) + 16, 16 );
      chunk[16] = '\0';
      i++;
      *addr = (uint16_t)strtoul(chunk, NULL, 2);
//      printf("Address: %d \n", *addr); 
      //memcpy(output, datablocks + *addr, 1);
      
      *output = *((char *)(datablocks + 500)); // OKay, this shit gives correct value somehow wtf 
      buffer_return(output);
      // Giving output 500 for adr but actually printing 04 which is datablocks + 502 address * Layman address god knows where that 2 offset came from.
      // Above error was for memcpy only. wth man......
    } else if (op == WRITE_CONST_INT) {
      
    }

    unsigned char * output_byte = (unsigned char *)outputbuffer;
    printf("Output Byte: %02x \n", output_byte[0]);

  }
}
