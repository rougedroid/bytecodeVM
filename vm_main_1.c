// Redoing cuz string one is mad messy.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>
#include <stdint.h>

uint8_t * datablocks;
uint8_t * outputbuffer;


// Data blocks from address space: 0x0000 to 0xFFF0 -> 65521 Address spaces. Changable Memory. 
// Each pointer points to 1 Byte of data. 
// 
// Output Buffer address 0xFFFF --> Last 8 bits
// Default output and error buffers etc etc from 0xFFF0 -> 0xFFF7
//

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

uint16_t test_values[] = {
  OP_RETURN, 0x1388
};

void init(){
  datablocks = malloc(sizeof(uint8_t)*65521); // 65521 bytes. 1 byte is 8 bits ( uint8_t)
  outputbuffer = malloc(sizeof(uint8_t)*1); // 1
  memset(datablocks, 0, 65521);

  memset(outputbuffer, 0, 2);
  memset(datablocks, 4, 5002);
}

void outputtobuffer(uint8_t * output){
  memcpy(outputbuffer, output, 1);
}

int main(){
  init();
  int instruction_len = (sizeof(test_values)/sizeof(test_values[0]));
  memcpy(datablocks, &test_values, instruction_len*2);
  uint16_t op;
  //op = *(test_values)[0]
  for (int i = 0; i < instruction_len; i++){
    //memcpy(&op, datablocks + i*2, 2); // Skipping 1 i for returning buffer is fine, cuz they are 16 bit address so like an address value also takes up 2 bytes. so does an opcode. 
    op = *((uint16_t *)(datablocks + i*2));
//    printf("Op Value: %" PRIu16 "\n", op);
    printf("Current op : %d \n", op);
    if (op == OP_NONE){
      printf("In OP1 \n");
      uint8_t * output = malloc(sizeof(uint8_t));
//      memset(output, 0x0007, 2);
      *output = (uint8_t)0x07;
      outputtobuffer(output);
    }else if (op == OP_RETURN){
      printf("In OP_RETURN \n");
      i++;
      uint8_t output;
      uint16_t addr;

      addr = *((uint16_t *)((uint8_t *)datablocks + i*2));
      printf("Addr: %d \n", addr);
//      memcpy(&output, (datablocks + addr), 2);
      output = *((char *)(datablocks + addr)); // There is a 2 byte shift error again. and idk what is causing it.
      output = *((char *)(datablocks + 5000));
      //memset(&output, 0x0008, 2 );
      outputtobuffer(&output);

    }
    

    uint8_t outputbyte;
    memcpy(&outputbyte, outputbuffer, 1);
    printf("Output Buffer Value: %d \n", outputbyte);
  }
}


