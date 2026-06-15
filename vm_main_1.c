// Redoing cuz string one is mad messy.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>
#include <stdint.h>

uint16_t * datablocks;
uint16_t * outputbuffer;


// Data blocks from address space: 0x0000 to 0xFFF0 -> 65521 Address spaces. Changable Memory. 
// Each pointer points to 1 Byte of data. 
// 
// Output Buffer address 0xFFFE - 0xFFFF --> Last 8 bits
// Default output and error buffers etc etc from 0xFFF0 -> 0xFFF7
// Addition Result: 0xFFF1 and 0xFFF2 
// Subtraction Result: 0xFFF3 and 0xFFF4 
// Overflow Bit: 0xFFF5 and 0xFFF6
// Multiplication Result:  
// After changing the data blocks type, there is no longer a need to have 2 bytes for each. But just keeping the diffs cuzz lazyyyyy :) 

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
  OP_JMP_RELP = 0x0029, // OPCODE [VALUE] --> Goes forward by value number of bytes. (value must be even number) 
  OP_JMP_RELN = 0x0030, // OPCODE [VALUE] --> Goes back by value number of bytes. (value must be even)
  OP_CMP_JMP = 0x0031, // OPCODE [REG1] [REG2] [JMP] --> jumps JMP number of bytes forward. ( JMP must be even )
  OP_MUL = 0X0032, // OPCODE [REG1] [REG2] --> Stored in 0xFFF7 and 0xFFF8
  OP_DIV = 0x0033, // OPCODE [REG1] [REG2] --> Store quotent in 0xFFF9 and 0xFFFA ; Remainder in 0xFFFB and 0xFFFC;
  OP_CMP_GTR = 0x0034, // OPCODE [REG1] [REG2] [JMP1] [JMP2] --> compares values. true if reg1>reg2. If true, it jumps toJMP1 pointer. If false it jumps to JMP2 pointer.
  OP_CMP_LSR = 0x0035, // OPCODE [REG1] [REG2] [JMP1] [JMP2] --> compares values. true if reg1<reg2. If true, it jumps toJMP1 pointer. If false it jumps to JMP2 pointer.
  OP_CMP_GTR_JMP = 0x0036, // OPCODE [REG1] [REG2] [JMP] --> jumps JMP number of bytes forward. ( JMP must be even )
  OP_CMP_LSR_JMP = 0x0037, // OPCODE [REG1] [REG2] [JMP] --> jumps JMP number of bytes forward. ( JMP must be even )
  OP_HALT = 0x0038,
}Opcodes;
// Note to self: A future reimplementation is required. Right now, there are too many arbitrary constraints for this to be a VM. In future, simulate a real chip from datasheet. And encode those constraints and implement clock cycles also. 
  // uint16_t instruction_set[] = {
  //   WRITE_CONST_INT, 0x0104, 0x0105,OP_RETURN, 0x0104, WRITE_CONST_INT, 0x0105, 0x0105, OP_RETURN, 0x0105, OP_CMP_JMP, 0x0104, 0x0105, 0x000c, WRITE_CONST_INT, 0x0106, 0x0004, OP_JMP_RELP, 0x0006, WRITE_CONST_INT, 0x0106, 0x0003, OP_RETURN, 0x0106, OP_HALT
  // };
// rn the integers accepted are 16 bits. i.e. 2 bytes, but we process only single byte integers. 
// Option 1: make it 1 byte integers -> horrible for everything will have to redesign everything. 
// Option 2: make it process 2 byte integers and let it output 2 bytes. -> Much simpler and better. 

//uint16_t * instruction_set;
 
uint16_t instruction_set[] = {WRITE_CONST_INT, 0xffef, 0x105, OP_LOAD_REG, 0xffef, 0xfff0, OP_RETURN, 0xfff0, OP_HALT};

void print_hex_dump(uint16_t *buffer) {
  size_t elements = 8;
    for (size_t i = 0; i < elements; i += 8) {
        // Print memory offset in hex (each element is 2 bytes)
        printf("%08zx: ", i * 2);

        // Print the 16-bit values cleanly
        for (size_t j = 0; j < 8; j++) {
            if (i + j < elements) {
                printf("%04x ", buffer[i + j]);
            } else {
                printf("     "); // Alignment padding
            }
        }
        printf("\n");
    }
}

uint16_t* readBinaryStream() {
    // 1. Defend against garbage inputs
    char filename[] = "bsp.out";
    size_t dummy_count = 0;
    size_t *out_count = &dummy_count;
    if (filename == NULL || out_count == NULL) {
        fprintf(stderr, "[ERROR] Invalid arguments passed to readBinaryStream.\n");
        return NULL;
    }
    *out_count = 0;

    // 2. Open the file in "rb" mode (Read Binary)
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("[ERROR] Failed to open file");
        return NULL;
    }

    // 3. Determine the exact file size in bytes
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("[ERROR] Failed to seek to end of file");
        fclose(file);
        return NULL;
    }

    long signed_size = ftell(file);
    if (signed_size < 0) {
        perror("[ERROR] Failed to determine file size via ftell");
        fclose(file);
        return NULL;
    }
    size_t file_bytes = (size_t)signed_size;

    // 4. Critical check: Is the file actually empty?
    if (file_bytes == 0) {
        fprintf(stderr, "[ERROR] File '%s' is empty (0 bytes).\n", filename);
        fclose(file);
        return NULL;
    }

    // 5. Critical check: Is the file size a multiple of 2 bytes (uint16_t)?
    if (file_bytes % sizeof(uint16_t) != 0) {
        fprintf(stderr, "[ERROR] File size (%zu bytes) is corrupt. It must be a multiple of %zu bytes for uint16_t alignment.\n", 
                file_bytes, sizeof(uint16_t));
        fclose(file);
        return NULL;
    }

    // Calculate exact element count
    size_t elements_to_read = file_bytes / sizeof(uint16_t);

    // 6. Reset file pointer back to the beginning to prepare for the read
    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("[ERROR] Failed to reset file pointer to beginning");
        fclose(file);
        return NULL;
    }

    // 7. Dynamic memory allocation with safety check
    uint16_t *buffer = malloc(file_bytes);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed. Cannot allocate %zu bytes.\n", file_bytes);
        fclose(file);
        return NULL;
    }

    // 8. Read the raw data into our buffer
    size_t elements_read = fread(buffer, sizeof(uint16_t), elements_to_read, file);
    
    // 9. Verify the integrity of the data read
    if (elements_read != elements_to_read) {
        if (ferror(file)) {
            fprintf(stderr, "[ERROR] Direct OS read error occurred while parsing file stream.\n");
        } else if (feof(file)) {
            fprintf(stderr, "[ERROR] Unexpected End-of-File reached. File may have been truncated mid-read.\n");
        }
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 10. Clean up file handle and update output variables
    fclose(file);
    *out_count = elements_to_read;
    print_hex_dump(buffer);
    exit(1);
    return buffer; 
}
//   uint16_t instruction_set[] = {
//     0x0020, 0xffef, 0x0105, 0x0025, 0xffef, 0xfff0, 0x0020, 0xffec,
//     0x0105, 0x0025, 0xffec, 0xffed, 0x0020, 0xffe9, 0x0000, 0x0025,
//     0xffe9, 0xffea, 0x0022, 0x0037, 0xfff0, 0xffed, 0x0009, 0x0020,
//     0xffe1, 0x0004, 0x0025, 0xffe1, 0xffea, 0x0029, 0x0007, 0x0020,
//     0xffdf, 0x0003, 0x0025, 0xffdf, 0xffea, 0x0022, 0x0038
// };

void outputtobuffer(uint16_t * output){
  memcpy(outputbuffer, output, 2);
}

void init(){
  datablocks = malloc(sizeof(uint16_t)*65521); // 65521 bytes. 1 byte is 8 bits ( uint8_t)
  outputbuffer = malloc(sizeof(uint16_t)*1); // 1
  memset(datablocks, 0, 65521);

  memset(outputbuffer, 0, 2);
  //instruction_set = readBinaryStream();


}

int main(){
  init();
  int instruction_len = (sizeof(instruction_set)/sizeof(instruction_set[0]));
  memcpy(datablocks, &instruction_set, 9*2);
  uint16_t op;
  //op = *(test_values)[0]
  int runflag = 1;
  int i =0;
  while (runflag == 1){
  //while ( i < instruction_len ){
    //memcpy(&op, datablocks + i*2, 2); // Skipping 1 i for returning buffer is fine, cuz they are 16 bit address so like an address value also takes up 2 bytes. so does an opcode. 
    op = *((uint16_t *)(datablocks + i));
//    printf("Op Code: 0x%04x\n", op);
    if (op == OP_NONE){
//      printf("In OP1 \n");
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x00;
      outputtobuffer(output);
    } else if (op == OP_RETURN) {
//      printf("In OP_RETURN \n");
      uint16_t addr = *((uint16_t *)(datablocks + (i + 1)));
//      printf("Addr: %d \n", addr);
      i++; // consume the operand word

      uint16_t output = ((uint16_t *)datablocks)[addr];
      // The mysterious byte monster has struck again. ITS STILL SHIFTING BY 2 BYTES.
      // SHIFT ERROR FIXED. MEMSET WAS COPYING 5000 BYTES FROM 0 TO 4999. So when i called 5000, it was on 0. but when i used memset to set 5002, and called 5000, memset set the bytes from 0 to 5001. So then, it appeared like there was a shift of 2 bytes. 
      outputtobuffer(&output);
      
    } else if (op == WRITE_CONST_INT){
      
      uint16_t addr = *((uint16_t *)(datablocks + (i + 1) ));
      
      i++; // consume the operand word
      uint16_t value = *((uint16_t *)(datablocks + (i + 1) ));
      i++;
      datablocks[addr] = (uint16_t) value;
      
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x0000;

      outputtobuffer(output);
      
    } else if (op == OP_ADD) {
      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i) ));

      datablocks[0xFFF1] = (uint16_t)(datablocks[addr1] + datablocks[addr2]);
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x0000;
      outputtobuffer(output);
      
    } else if (op == OP_LOAD_REG) {
      
      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i) ));
      
      datablocks[addr2] = datablocks[addr1];

      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x0000;
      outputtobuffer(output);

    } else if (op == OP_SUB) {
      
      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      
      int k = datablocks[addr1] - datablocks[addr2];
      if (k>=0){
        datablocks[0xFFF3] = (uint16_t)k;

      } else {
        datablocks[0xFFF5] = 0x0001;
        datablocks[0xFFF3] = (uint16_t)k;
      }

      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x00;
      outputtobuffer(output);

    } else if (op == OP_JUMP) {

      i++;
      uint16_t addr = *((uint16_t *)(datablocks + (i)));
      i = (addr/2) -1;
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x00;
      outputtobuffer(output);

    } else if (op == OP_CMP) {
      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t jmp1 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp2 = *((uint16_t *)(datablocks + (i) ));
      
      if (datablocks[addr1]==datablocks[addr2]){
        i = (jmp1/2) -1;
      }else{
        i = (jmp2/2) -1;
      }
    } else if (op == OP_CMP_JMP) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp = *((uint16_t *)(datablocks + (i) ));
//      i++;
//      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      if (datablocks[addr1]!=datablocks[addr2]){
        printf("Comparison Negative\n");
        i = i + (jmp/2) -1 ; // jmp number of bytes offset from current position. -1 to counter the i++
      }
    } else if (op == OP_CMP_GTR_JMP) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp = *((uint16_t *)(datablocks + (i) ));
//      i++;
//      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      if (datablocks[addr1]<=datablocks[addr2]){
        printf("Comparison Negative\n");
        i = i + (jmp/2) -1 ; // jmp number of bytes offset from current position. -1 to counter the i++
      }
    } else if (op == OP_CMP_LSR_JMP) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp = *((uint16_t *)(datablocks + (i) ));
//      i++;
//      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      if (datablocks[addr1]>=datablocks[addr2]){
        printf("Comparison Negative\n");
        i = i + (jmp/2) -1 ; // jmp number of bytes offset from current position. -1 to counter the i++
      }
    } else if (op == OP_CMP_GTR) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp = *((uint16_t *)(datablocks + (i) ));
//      i++;
//      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      if (datablocks[addr1]>datablocks[addr2]){
        printf("Comparison Positive\n");
        i = i + (jmp/2) -1 ; // jmp number of bytes offset from current position. -1 to counter the i++
      }
    } else if (op == OP_CMP_LSR) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i)));
      i++;
      uint16_t jmp = *((uint16_t *)(datablocks + (i) ));
//      i++;
//      uint16_t jmp2 = *((uint16_t *)((uint8_t *)datablocks + (i) * 2));
      if (datablocks[addr1]<datablocks[addr2]){
        printf("Comparison Positive\n");
        i = i + (jmp/2) -1 ; // jmp number of bytes offset from current position. -1 to counter the i++
      }
    } else if (op == OP_JMP_RELP) {

      i++;
      uint16_t value = *((uint16_t *)(datablocks + (i) ));
      i = i + (value/2);
    }else if (op == OP_JMP_RELN) {
      i++;
      uint16_t value = *((uint16_t *)(datablocks + (i)));

      i = i - (value/2);
    }else if (op == OP_MUL) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i) ));

      datablocks[0xFFF7] = (uint16_t)(datablocks[addr1] * datablocks[addr2]);
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x0000;
      outputtobuffer(output);
    }else if (op == OP_DIV) {

      i++;
      uint16_t addr1 = *((uint16_t *)(datablocks + (i) ));
      i++;
      uint16_t addr2 = *((uint16_t *)(datablocks + (i) ));

      datablocks[0xFFF9] = (uint16_t)(datablocks[addr1] / datablocks[addr2]);
      datablocks[0xFFFB] = (uint16_t)(datablocks[addr1]% datablocks[addr2]);
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x0000;
      outputtobuffer(output);
    }else if (op == OP_HALT){
      printf("OP HALT HIT\n");
      exit(1);
      runflag = 0;
    }
    else{
      uint16_t * output = malloc(sizeof(uint16_t));
      *output = (uint16_t)0x00;
      outputtobuffer(output);
    }
    uint16_t outputbyte;
    memcpy(&outputbyte, outputbuffer, 2);
    printf("Output Buffer Value: %d \n", outputbyte);
    
    i++;
  }
}


