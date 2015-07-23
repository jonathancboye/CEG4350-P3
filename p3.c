/*
 * emulator.c
 *
 *  Created on: Jul 21, 2015
 *      Author: jon
 */

/* NEXT TASK
 * Figure out proper length for program counter
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEMORYSIZE 5000

typedef unsigned char mtype;

//contains the condition codes
typedef struct ConditionCodes {
	uint8_t zf:1;
	uint8_t sf:1;
	uint8_t of:1;
} ConditionCodes;

//Register
//	integer: full register, 32bits
//	upper: upper half of the register, 16bits
//  lower: lower half of the register, 16bits
//	byte_1: first byte, 8bits
//	byte_2: second byte, 8bits
//	byte_3: third byte, 8bits
//	byte_4: fourth byte, 8bits
typedef union registers {
	uint32_t integer:32;
	struct sections {
		uint16_t upper:16;
		uint16_t lower:16;
	} section;
	struct bytes {
		uint8_t byte_1:8; //most significant byte
		uint8_t byte_2:8;
		uint8_t byte_3:8;
		uint8_t byte_4:8; //least significant byte
	} byte;
} Register;

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
	int PC; //program counter, points to next instruction in memory
	ConditionCodes cc;
	mtype *memory = calloc(MEMORYSIZE, sizeof(mtype)); //Main memory for y86

	/* setup input */
	memory[0] = 0x10;
	memory[1] = 0x00;

	/* initialize program counter and flags and registers*/
	PC = 0;
	cc.zf = 0;
	cc.sf = 0;
	cc.of = 0;
	eax.integer = 0;
	ecx.integer = 0;
	edx.integer = 0;
	esi.integer = 0;
	edi.integer = 0;
	esp.integer = 0;
	esp.integer = 0;
	ebp.integer = 0;


	/* fetch, decode, execute - loop*/
	while(1) {
		/* Fetch && Decode */

		//Read in first byte
		switch(memory[PC]) {

		case 0x00: 				//halt instruction
			printf("HALT\n");
			return EXIT_SUCCESS;

		case 0x10: 				//nop instruction
			printf("NOP\n");
			break;

		default: 				//error
			printf("Something didn't work");
		}

		/* Execute */

		/* Update PC */
		PC++;
	}



	return EXIT_SUCCESS;
}
