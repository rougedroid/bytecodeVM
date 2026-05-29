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
// Addition Result: 0xFFF1 
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
  OP_CMP = 0x0028, // OPCODE [REG1] [REG2] [JMP1] [JMP2] --> compares if both values in REG1 and REG2 are same. If true, it jumps toJMP1 pointer. If false it jumps to JMP2 pointer. 
}Opcodes;

uint16_t test_values[] = {
  WRITE_CONST_INT, 0x0104, 0x0005, WRITE_CONST_INT, 0x0105, 0x0007, OP_ADD, 0x0104, 0x0105, OP_LOAD_REG, 0xFFF1, 0x0104, WRITE_CONST_INT, 0x0105, 0x0003, OP_ADD, 0x0105, 0x0104, OP_LOAD_REG, 0xFFF1, 0x0106, OP_RETURN, 0x0106
};

void init(){
  datablocks = malloc(sizeof(uint8_t)*65521); // 65521 bytes. 1 byte is 8 bits ( uint8_t)
  outputbuffer = malloc(sizeof(uint8_t)*1); // 1
  memset(datablocks, 0, 65521);

  memset(outputbuffer, 0, 2);
  datablocks[5000]=4;
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
      *output = (uint8_t)0x00;
      outputtobuffer(output);
    } else if (op == OP_RETURN) {
      printf("In OP_RETURN \n");
      uint16_t addr = *((uint16_t *)((uint8_t *)datablocks + (i + 1) * 2));
      printf("Addr: %d \n", addr);
      i++; // consume the operand word

      uint8_t output = ((uint8_t *)datablocks)[addr];
      // The mysterious byte monster has struck again. ITS STILL SHIFTING BY 2 BYTES.
      // SHIFT ERROR FIXED. MEMSET WAS COPYING 5000 BYTES FROM 0 TO 4999. So when i called 5000, it was on 0. but when i used memset to set 5002, and called 5000, memset set the bytes from 0 to 5001. So then, it appeared like there was a shift of 2 bytes. 
      outputtobuffer(&output);
      
    } else if (op == WRITE_CONST_INT){
      
      uint16_t addr = *((uint16_t *)((uint8_t *)datablocks + (i + 1) * 2));
      
      i++; // consume the operand word
      uint16_t value = *((uint16_t *)((uint8_t *)datablocks + (i + 1) * 2));
      i++;
      datablocks[addr] = (uint8_t) value;

      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;

      outputtobuffer(output);
      
    } else if (op == OP_ADD) {
      i++;
      uint16_t addr1 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t addr2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));

      datablocks[0xFFF1] = datablocks[addr1] + datablocks[addr2];
      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;
      outputtobuffer(output);
      
    } else if (op == OP_LOAD_REG) {
      
      i++;
      uint16_t addr1 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t addr2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      
      datablocks[addr2] = datablocks[addr1];

      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;
      outputtobuffer(output);
    } else if (op == OP_SUB) {

      
      i++;
      uint16_t addr1 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t addr2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      
      int k = datablocks[addr1] - datablocks[addr2];
      if (k>=0){
        datablocks[0xFFF2] = k;

      } else {
        datablocks[0xFFF3] = 1;
        datablocks[0xFFF2] = k;
      }

      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;
      outputtobuffer(output);


    } else if (op == OP_JUMP) {

      i++;
      uint16_t addr = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i = (addr/2) -1;
      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;
      outputtobuffer(output);
    } else if (op == OP_CMP) {

      
      
      i++;
      uint16_t addr1 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t addr2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t jmp1 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      i++;
      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      
      if (datablocks[addr1]==datablocks[addr2]){
        i = (jmp1/2) -1;
      }else{
        i = (jmp2/2) -1;
      }

    }else{

      uint8_t * output = malloc(sizeof(uint8_t));
      *output = (uint8_t)0x00;
      outputtobuffer(output);
    }
    
    

    uint8_t outputbyte;
    memcpy(&outputbyte, outputbuffer, 1);
    printf("Output Buffer Value: %d \n", outputbyte);
  }
}


