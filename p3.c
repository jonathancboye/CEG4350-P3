/*
 * emulator.c
 *
 *  Created on: Jul 21, 2015
 *      Author: jon
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEMORYSIZE 5000
#define DEBUG 1 //turn debugging on or off

typedef char mtype;

//contains the condition codes
typedef struct {
	uint8_t zf:1;
	uint8_t sf:1;
	uint8_t of:1;
} ConditionCodes;

typedef struct {
	uint8_t bit_1:1;
	uint8_t bit_2:1;
	uint8_t bit_3:1;
	uint8_t bit_4:1;
	uint8_t bit_5:1;
	uint8_t bit_6:1;
	uint8_t bit_7:1;
	uint8_t bit_8:1;
}Bits;

typedef struct {
	uint8_t lower:4;
	uint8_t upper:4;
}Nibbles;

typedef struct {
	uint32_t byte_1:8; //most significant byte
	uint32_t byte_2:8;
	uint32_t byte_3:8;
	uint32_t byte_4:8; //least significant byte
}Bytes;

typedef struct {
	uint32_t lower:16;
	uint32_t upper:16;
}Words;

/*Register
 * integer: full register, 32bits
 * upper: upper half of the register, 16bits
 * lower: lower half of the register, 16bits
 * byte_1: first byte, 8bits
 * byte_2: second byte, 8bits
 * byte_3: third byte, 8bits
 * byte_4: fourth byte, 8bits
 */

typedef union {
	uint32_t reg;
	Words words;
	Bytes bytes;
}Register;

typedef union {
	uint8_t byte;
	Nibbles nibbles;
	Bits bits;
}Byte;


void irmovl(int PC, mtype *memory, Register *regs); //immediate -> register


int main(int argc, char *argv[]) {
	/* 32bit program registers */
	Register eax;
	Register ecx;
	Register edx;
	Register ebx;
	Register esi;
	Register edi;
	Register esp; //stack pointer register
	Register ebp;
	Register regs[8] = {eax, ecx, edx, ebx, esi, edi, esp, ebp};
	int i, PC; //program counter, points to next instruction in memory
	int instructionLength; //number of bytes that make up an instruction
	ConditionCodes cc;
	mtype *memory = calloc(MEMORYSIZE, sizeof(mtype)); //Main memory for y86

	/* setup input */

	//nop
	memory[0] = 0x10;

	//irmovl register 0 <--- 0x0403
	memory[1] = 0x30;
	memory[2] = 0x83;
	//value
	memory[3] = 0x03;
	memory[4] = 0x04;
	memory[5] = 0x01;
	memory[6] = 0x20;

	//halt
	memory[7] = 0x00;

	/* initialize program counter and flags and registers*/
	PC = 0;
	cc.zf = 0;
	cc.sf = 0;
	cc.of = 0;
	for(i=0;i < 8; ++i) {
		regs[i].reg = 0;
	}

	/* fetch, decode, execute - loop*/
	while(1) {
		/* Fetch && Decode */

		//Read in first byte
		switch(memory[PC]) {

		case 0x00: 					//halt instruction
			if(DEBUG) {
				printf("HALT\n");
			}
			return EXIT_SUCCESS;

		case 0x10: 					//nop instruction
			instructionLength = 1;
			if(DEBUG) {
				printf("NOP\n");
			}
			break;

		case 0x20:					//rrmovl - Register -> Register
			if(DEBUG) {
				printf("rrmovl");
			}
			break;

		case 0x30:					//irmovl - immediate -> Register
			/*
			 * Byte offsets
			 * 	1: [0x8, register to store value]
			 * 	2 - 5: [value]
			 */
			instructionLength = 6;
			if(DEBUG) {
				printf("irmovl: 0x30");
			}
			irmovl(PC, memory, regs);
			break;

		default: 					//error
			printf("Something didn't work");
			/* no break */
		}

		/* Update PC */
		PC += instructionLength;
	}

	return EXIT_SUCCESS;
}

void irmovl(int PC, mtype *memory, Register *regs) {
	Byte b; //next byte
	int id; //register id

	//get second byte : [upper nibble has register id, lower nibble should have 8 for instruction]
	b.byte = memory[PC + 1];

	//verify last nibble of instruction
	if(b.nibbles.upper == 8) {

		id = b.nibbles.lower;

		//move next 4 bytes into register
		//ugly: *((uint32_t*)&memory[PC + 2])
		regs[id].bytes.byte_1 = memory[PC + 2];
		regs[id].bytes.byte_2 = memory[PC + 3];
		regs[id].bytes.byte_3 = memory[PC + 4];
		regs[id].bytes.byte_4 = memory[PC + 5];

		if(DEBUG) {
			printf(" 0x%d%d 0x%2x\n", b.nibbles.upper, id, regs[id].reg);
		}

	} else {
		printf("Something is wrong!!");
		exit(1);
	}
}
