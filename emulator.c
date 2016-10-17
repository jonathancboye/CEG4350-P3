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
#define DEBUG 0 //turn debugging on or off


typedef char mtype;

//contains the condition codes
typedef struct {
	uint8_t zf:1;
	uint8_t sf:1;
	uint8_t of:1;
} ConditionCodes;

typedef struct {
	uint8_t bit_1:1; //least significant bit
	uint8_t bit_2:1;
	uint8_t bit_3:1;
	uint8_t bit_4:1;
	uint8_t bit_5:1;
	uint8_t bit_6:1;
	uint8_t bit_7:1;
	uint8_t bit_8:1; //most significant bit
}Bits;

typedef struct {
	uint8_t lower:4;
	uint8_t upper:4;
}Nibbles;

typedef union {
	uint8_t byte;
	Nibbles nibbles;
	Bits bits;
}Byte;

typedef struct {
	Byte byte_1; //least significant byte
	Byte byte_2;
	Byte byte_3;
	Byte byte_4; //most significant byte
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
	uint32_t dword;
	Words words;
	Bytes bytes;
}Register;


void printInstruction_6(char *instruction, int opcode, int nibl, int nibu, Register r); //prints a 6 byte instruction
void printInstruction_2(char *instruction, int opcode, int nibl, int nibu); //prints a 2 byte instruction
void printState(Register *regs, ConditionCodes cc, int PC); //prints state of registers, condition codes, and program counter

void irmovl(int PC, mtype *memory, Register *regs); //immediate -> register
void rrmovl(int PC, mtype *memory, Register *regs); //register -> register
void rmmovl(int PC, mtype *memory, Register *regs); //register -> memory
void mrmovl(int PC, mtype *memory, Register *regs); //register <--- memory(offset(base register))
void OPl(int PC, mtype *memory, Register *regs, ConditionCodes *cc,char operation); //registerA + registerB --> registerB
void jmp(int *PC, mtype *memory); //PC = address
void pushl(int PC, mtype *memory, Register *regs);  //push double word onto top of stack
void popl(int PC, mtype *memory, Register *regs);   //pop double word off the top of stack
void call(int *PC, mtype *memory, Register *regs);  //push PC onto stack and jmp to destination
void ret(int *PC, mtype *memory, Register *regs); 	//pop a value from the stack and set it value of PC
int asciiToByte(char ascii);	//Correct's design of project by converting each ASCII character to a byte

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
	Register regs[8] = {eax, ecx, edx, ebx, esp, ebp, esi, edi};
	int i, halted, PC; //program counter, points to next instruction in memory
	int instructionLength; //number of bytes that make up an instruction
	Byte firstByte; //first read in byte
	int unib;
	ConditionCodes cc;
	mtype *memory = calloc(MEMORYSIZE, sizeof(mtype)); //Main memory for y86

	/* setup input */

	if(argc != 2) {
		printf("Programs needs a ascii file as input\n");
		printf("syntax: project3 <fileName>\n");
		exit(1);
	}
	FILE *fp = NULL;
	char *filename = argv[1];
	char upperNibble, lowerNibble;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("Could not find file\n");
		exit(1);
	}
	i = 0;
	while(fscanf(fp,"%c%c", &upperNibble, &lowerNibble) != EOF) {
		firstByte.nibbles.upper = asciiToByte(upperNibble);
		firstByte.nibbles.lower = asciiToByte(lowerNibble);
		memory[i++] = firstByte.byte;
	}

	/* initialize program counter and flags and registers*/
	PC = 0;
	cc.zf = 0;
	cc.sf = 0;
	cc.of = 0;
	for(i=0;i < 8; ++i) {
		regs[i].dword = 0;
	}
	regs[4].dword = MEMORYSIZE;
	halted = 0;
	/* fetch, decode, execute - loop*/
	while(!halted) {
		/* Fetch && Decode */

		//Read in first byte
		firstByte.byte = memory[PC];
		unib = firstByte.nibbles.upper;

		/* These instruction are not within the switch case range */
		if(unib == 0xA) {			//pushl instruction
			instructionLength = 2;
			pushl(PC, memory, regs);
		} else if (unib == 0xB) {	//popl instruction
			instructionLength = 2;
			popl(PC, memory, regs);
		} else if (unib == 0x8) { 	//call instruction
			instructionLength = 0;
			call(&PC, memory, regs);
		} else if (unib == 0x9) { 	//return instruction
			instructionLength = 0;
			ret(&PC, memory, regs);
		} else {
			/* These instruction are within the switch case range */
			switch(firstByte.byte) {
			case 0x00: 					//halt instruction
				printf("00 00 00 00 00 00 :HALT\n");
				halted = 1;
				break;
			case 0x10: 					//nop instruction
				printf("10 00 00 00 00 00 :NOP\n");
				instructionLength = 1;
				break;
			case 0x20:					//rrmovl - Register -> Register unconditional move
				instructionLength = 2;
				rrmovl(PC, memory, regs);
				break;
			case 0x21:					//cmovle - Register -> Register when less or equal
				instructionLength = 2;
				if(cc.zf || cc.sf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x22:					//cmovl - Register -> Register when less
				instructionLength = 2;
				if(cc.sf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x23:					//cmove - Register -> Register when equal
				instructionLength = 2;
				if(cc.zf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x24:					//cmovne - Register -> Register when not equal
				instructionLength = 2;
				if(!cc.zf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x25:					//cmovge - Register -> Register when greater or equal
				instructionLength = 2;
				if(cc.zf || !cc.sf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x26:					//cmovg - Register -> Register when greater
				instructionLength = 2;
				if(!cc.sf && !cc.zf) {
					rrmovl(PC, memory, regs);
				}
				break;
			case 0x30:					//irmovl - immediate -> Register
				instructionLength = 6;
				irmovl(PC, memory, regs);
				break;
			case 0x40:					//rmmovl - register -> memory(offset(base register))
				instructionLength = 6;
				rmmovl(PC, memory, regs);
				break;
			case 0x50:					//mrmovl - register <--- memory(offset(base register))
				instructionLength = 6;
				mrmovl(PC, memory, regs);
				break;
			case 0x60:					//addl - register A + register B -> register B
				instructionLength = 2;
				OPl(PC, memory, regs, &cc, '+');
				break;
			case 0x61:					//subl - register A - register B -> register B
				instructionLength = 2;
				OPl(PC, memory, regs, &cc, '-');
				break;
			case 0x62:					//andl - register A & register B -> register B
				instructionLength = 2;
				OPl(PC, memory, regs, &cc, '&');
				break;
			case 0x63:					//xorl - register A xor register B -> register B
				instructionLength = 2;
				OPl(PC, memory, regs, &cc, '^');
				break;
			case 0x70:					//jmp - PC = address
				instructionLength = 0;
				jmp(&PC, memory);
				break;
			case 0x71:					//jle - PC = address when less or equal
				instructionLength = 0;
				if(cc.sf || cc.zf) {
					jmp(&PC, memory);
				}
				break;
			case 0x72:					//jl - PC = address when less
				instructionLength = 0;
				if(cc.sf) {
					jmp(&PC, memory);
				}
				break;
			case 0x73:					//je - PC = address when equal
				instructionLength = 0;
				if(cc.zf) {
					jmp(&PC, memory);
				}
				break;
			case 0x74:					//jne - PC = address when not equal
				instructionLength = 0;
				if(!cc.zf) {
					jmp(&PC, memory);
				}
				break;
			case 0x75:					//jge - PC = address when greater or equal
				instructionLength = 0;
				if(cc.zf || !cc.sf) {
					jmp(&PC, memory);
				}
				break;
			case 0x76:					//jg - PC = address when greater
				instructionLength = 0;
				if(!cc.zf && !cc.sf) {
					jmp(&PC, memory);
				}
				break;
			default: 					//error
				printf("Something didn't work\n");
				exit(1);
				/* no break */
			}
		}

		if(DEBUG) {
			printState(regs, cc, PC);
		}


		/* Update PC */
		PC += instructionLength;
	}

	printState(regs, cc, PC);
	free(memory);
	return EXIT_SUCCESS;
}

void irmovl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 * 	1: [0x8, register to store value]
	 * 	2 - 5: [value]
	 */

	Byte b; //next byte
	int id; //register id

	//get second byte: [upper nibble has register id, lower nibble should have 8 for instruction]
	b.byte = memory[PC + 1];

	//verify last nibble of instruction
	if(b.nibbles.upper == 8) {

		id = b.nibbles.lower;

		//move next 4 bytes into register
		regs[id].bytes.byte_1.byte = memory[PC + 2];
		regs[id].bytes.byte_2.byte = memory[PC + 3];
		regs[id].bytes.byte_4.byte = memory[PC + 5];
		regs[id].bytes.byte_3.byte = memory[PC + 4];


		printInstruction_6("irmovl", 0x30, id, b.nibbles.upper, regs[id]);


	} else {
		printf("Something is wrong!!\n");
		exit(1);
	}
}

void rrmovl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 * 	1: [source register id, destination register id]
	 */
	Byte b; //next byte
	int idS; //source register id;
	int idD; //destination register id;

	//get source and destination registers
	b.byte = memory[PC + 1];
	idD = b.nibbles.lower;
	idS = b.nibbles.upper;

	//move contents of source register into destination register
	regs[idD].dword = regs[idS].dword;

	printInstruction_2("rrmovl", 0x20, idS, idD);

}

void rmmovl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 *  1: [source register, base register]
	 *  2-5: [value of offset]
	 */
	Byte b;
	int idS; //source register id
	int idB; //base register id
	int address; //address to write to
	Register offset;

	b.byte = memory[PC + 1];
	idS = b.nibbles.upper;
	idB = b.nibbles.lower;
	offset.bytes.byte_1.byte = memory[PC + 2];
	offset.bytes.byte_2.byte = memory[PC + 3];
	offset.bytes.byte_3.byte = memory[PC + 4];
	offset.bytes.byte_4.byte = memory[PC + 5];
	address = regs[idB].dword + offset.dword;

	//write source register to memory if within memory bounds
	if(address + 3 < regs[4].dword) {
		memory[address] = regs[idS].bytes.byte_1.byte;
		memory[address + 1] = regs[idS].bytes.byte_2.byte;
		memory[address + 2] = regs[idS].bytes.byte_3.byte;
		memory[address + 3] = regs[idS].bytes.byte_4.byte;

		printInstruction_6("rmmovl", 0x40, idB, idS, offset);

	} else {
		printf("Writing to out of bounds memory address");
		exit(1);
	}
}

void mrmovl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 * 	1: [destination register id, base register id]
	 * 	2 - 5: [offset value]
	 */
	Byte b;
	int idD; //destination register id
	int idB; //base register id
	int address; //address to read from
	Register offset;

	//Calculate address load from and register id's
	b.byte = memory[PC + 1];
	idB = b.nibbles.lower;
	idD = b.nibbles.upper;
	offset.bytes.byte_1.byte = memory[PC + 2];
	offset.bytes.byte_2.byte = memory[PC + 3];
	offset.bytes.byte_3.byte = memory[PC + 4];
	offset.bytes.byte_4.byte = memory[PC + 5];
	address = offset.dword + regs[idB].dword;

	//Load contents of memory address into destination register
	if(address + 3 < regs[4].dword) {
		regs[idD].bytes.byte_1.byte = memory[address];
		regs[idD].bytes.byte_2.byte = memory[address + 1];
		regs[idD].bytes.byte_3.byte = memory[address + 2];
		regs[idD].bytes.byte_4.byte = memory[address + 3];

		printInstruction_6("mrmovl", 0x50, idB, idD, offset);

	} else {
		printf("Reading from out of bounds memory address\n");
	}
}

void OPl(int PC, mtype *memory, Register *regs, ConditionCodes *cc,char operation) {
	/*
	 * Byte offsets:
	 * 	1: [register A, register B]
	 */
	Byte b;
	int idB; //register B
	int idA; //register A
	int A; //value of register A
	int B; //value of register B
	int result; //sum of register B and A
	char *instruction;
	int opcode;

	/* calculate the result */
	b.byte = memory[PC + 1];
	idB = b.nibbles.lower;
	idA = b.nibbles.upper;
	A = regs[idA].dword;
	B = regs[idB].dword;
	if(operation == '+' || operation == '-') {

		if(operation == '+') {
			result = A + B;
			instruction = "addl";
			opcode = 0x60;
		} else {
			B = -B; //flip sign so we can check turn it in to addition to check for overflow
			result = A + B;
			instruction = "subl";
			opcode = 0x61;
		}

		/* check if overflow occurred */
		if((A > 0 && B > 0 && result < 0) ||
				(A < 0 && B < 0 && result > 0)) {
			cc -> of = 1;
		} else {
			cc -> of = 0;
		}

	} else if(operation == '&') {
		result = A & B;
		instruction = "andl";
		opcode = 0x62;
	} else if(operation == '^') {
		result = A ^ B;
		instruction = "xorl";
		opcode = 0x63;
	}
	regs[idB].dword = result;

	/*set condition flags*/
	if(result == 0) {
		cc -> zf = 1;
		cc -> sf = 0;
	} else if (result < 0) {
		cc -> sf = 1;
		cc -> zf = 0;
	}


	printInstruction_2(instruction, opcode, idA, idB);

}

void jmp(int *PC, mtype *memory) {
	/*
	 * Byte offsets
	 * 	1 - 5: [address to jump to], but y86 address space can only contain 2^32 bytes so ignore the last byte
	 */
	Register address;
	address.bytes.byte_1.byte = memory[*PC + 1];
	address.bytes.byte_2.byte = memory[*PC + 2];
	address.bytes.byte_3.byte = memory[*PC + 3];
	address.bytes.byte_4.byte = memory[*PC + 4];
	if(address.dword < MEMORYSIZE) {
		*PC = address.dword;
		if(DEBUG) {
			printInstruction_6("jmp", 0x70, 8, 8, address);
		}
	} else {
		printf("Jumping out of bounds\n");
		exit(1);
	}
}

void pushl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 * 	1:[the register id of the register who's contents need to be pushed onto the stack, dummy register]
	 */
	Byte b;
	int id; //register id
	b.byte = memory[PC + 1];
	id = b.nibbles.upper;

	//reduce stack pointer by 4 and push contents of register onto stack
	int sp = regs[4].dword;
	if(sp - 4 >= 0) {
		memory[sp - 4] = regs[id].bytes.byte_1.byte;
		memory[sp - 3] = regs[id].bytes.byte_2.byte;
		memory[sp - 2] = regs[id].bytes.byte_3.byte;
		memory[sp - 1] = regs[id].bytes.byte_4.byte;
		regs[4].dword -= 4;
	} else {
		printf("pushing out of bounds!");
	}

	printInstruction_2("pushl", 0xA0, b.nibbles.upper, b.nibbles.lower);
}

void popl(int PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets
	 * 	1: [the register to place value that is poped off the stack, dummy register]
	 */
	Byte b;
	int id; //register id
	b.byte = memory[PC + 1];
	id = b.nibbles.upper;

	//pop value of stack into register and increment stack pointer by four
	int sp = regs[4].dword;
	if(sp + 4 <= MEMORYSIZE) {
		regs[id].bytes.byte_1.byte = memory[sp];
		regs[id].bytes.byte_2.byte = memory[sp + 1];
		regs[id].bytes.byte_3.byte = memory[sp + 2];
		regs[id].bytes.byte_4.byte = memory[sp + 3];
		regs[4].dword += 4;
	} else {
		printf("poping out of bounds!\n");
		exit(1);
	}

	printInstruction_2("popl", 0xB0, b.nibbles.upper, b.nibbles.upper);

}

void call(int *PC, mtype *memory, Register *regs) {
	/*
	 * Byte offsets:
	 * 	1-6[destination of jmp] can ignore last byte since address is larger than addressable space
	 */
	Register address; //address to begin execution at
	Register returnAddress; //address to return to
	int sp = regs[4].dword;

	if(sp - 1 >= 0) {
		//store return address on stack
		returnAddress.dword =  *PC + 6;
		memory[sp - 4] = returnAddress.bytes.byte_1.byte;
		memory[sp - 3] = returnAddress.bytes.byte_2.byte;
		memory[sp - 2] = returnAddress.bytes.byte_3.byte;
		memory[sp - 1] = returnAddress.bytes.byte_4.byte;
		regs[4].dword -= 4;

		//move program counter to address
		address.bytes.byte_1.byte = memory[*PC + 1];
		address.bytes.byte_2.byte = memory[*PC + 2];
		address.bytes.byte_3.byte = memory[*PC + 3];
		address.bytes.byte_4.byte = memory[*PC + 4];
		if(address.dword < MEMORYSIZE) {
			*PC = address.dword;
		} else {
			printf("address of call instruction is beyond memory limits\n");
		}
	} else {
		printf("pushing out of bounds!");
		exit(1);
	}


	printInstruction_6("call",0x80,0xF,0xF, address);

}

void ret(int *PC, mtype *memory, Register *regs) {
	Register address; //popped value from stack
	int sp = regs[4].dword;
	address.bytes.byte_1.byte = memory[sp];
	address.bytes.byte_2.byte = memory[sp + 1];
	address.bytes.byte_3.byte = memory[sp + 2];
	address.bytes.byte_4.byte = memory[sp + 3];
	regs[4].dword += 4;
	*PC = address.dword;


	printf("ret: 90\n");

}

int asciiToByte(char ascii) {
	/*
	 * "A0" --> 0xA0 ===  0x41 0x30 ---> 0xA0
	 */
	int retval;
	switch (ascii) {
	case 'A':
	case 'a':
		retval = 0xA;
		break;
	case 'B':
	case 'b':
		retval = 0xB;
		break;
	case 'C':
	case 'c':
		retval = 0xC;
		break;
	case 'D':
	case 'd':
		retval = 0xD;
		break;
	case 'E':
	case 'e':
		retval = 0xE;
		break;
	case 'F':
	case 'f':
		retval = 0xF;
		break;
	case '0':
		retval = 0x0;
		break;
	case '1':
		retval = 0x1;
		break;
	case '2':
		retval = 0x2;
		break;
	case '3':
		retval = 0x3;
		break;
	case '4':
		retval = 0x4;
		break;
	case '5':
		retval = 0x5;
		break;
	case '6':
		retval = 0x6;
		break;
	case '7':
		retval = 0x7;
		break;
	case '8':
		retval = 0x8;
		break;
	case '9':
		retval = 0x9;
		break;
	default:
		printf("Conversion gone wrong!");
		exit(1);
	}
	return retval;
}

void printState(Register *regs, ConditionCodes cc, int PC) {
	int i;
	printf("===Contents of Registers in HEX===\n");
	for(i=0;i < 8; ++i) {
		printf("register id: %d,  %2x %2x %2x %2x\n", i,
				regs[i].bytes.byte_4.byte, regs[i].bytes.byte_3.byte,
				regs[i].bytes.byte_2.byte, regs[i].bytes.byte_1.byte);
	}
	printf("condition codes: of = %d, zf = %d, sf = %d\n", cc.of, cc.zf, cc.sf);
	printf("Program counter = %d\n", PC);
	printf("===End Contents===\n\n");
}

void printInstruction_6(char *instruction, int opcode, int nibl, int nibu, Register r) {
	printf("%2x %1x%1x %2x %2x %2x %2x :%s\n", opcode, nibu, nibl,
			r.bytes.byte_1.byte, r.bytes.byte_2.byte, r.bytes.byte_3.byte, r.bytes.byte_4.byte, instruction);
}

void printInstruction_2(char *instruction, int opcode, int nibu, int nibl) {
	printf("%2x %1x%1x 00 00 00 00 :%s\n", opcode, nibu, nibl, instruction);
}
