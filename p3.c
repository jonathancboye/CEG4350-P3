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
	Register esp;
	Register ebp;
	Register PC; //program counter
	ConditionCodes cc;
	char *memory = calloc(MEMORYSIZE, sizeof(char)); //Main memory for y86


	return EXIT_SUCCESS;
}
