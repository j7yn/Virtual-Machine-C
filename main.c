#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#define STACK_SIZE 256
static int stack[STACK_SIZE];

// Instructions
typedef enum {
	HLT, // 0   - hlt		:= halts program
	PSH, // 1   - psh val		:= pushes <val> to stack
	POP, // 2   - pop		:= pops value from stack
	ADD, // 3   - add		:= adds top two vals on stack
	MUL, // 4   - mul		:= multiplies top two vals on stack
	DIV, // 5   - div		:= divides top two vals on stack
	SUB, // 6   - sub		:= subtracts top two vals on stack 
	SLT, // 7   - slt reg_a, reg_b	:= pushes (reg_a < reg_b) to stack
	MOV, // 8   - mov reg_a, reg_b 	:= moves the value in reg_a to reg_b
	SET, // 9   - set reg, val	:= sets the reg to value
	LOG, // 10  - log a		:= prints out a
	IF,  // 11  - if reg val ip	:= if the register == val branch to the ip
	IFN, // 12  - ifn reg val ip	:= if the register != val branch to the ip
	GLD, // 13  - gld reg		:= loads a register to the stack
	GPT, // 14  - gpt reg		:= pushes top of stack to the given register
	NOP  // 15  - nop		:= nothing
} Instructions;

// Registers
typedef enum {
	A, B, C, D, E, F, I, J, // General Purpose
	EX,			// Excess 
	EXA,			// More Excess
	IP,			// Instruction Pointer
	SP,			// Stack Pointer
	REGISTER_SIZE
} Registers;

static int registers[REGISTER_SIZE];

// Instructions Array
int *instructions;

// # of Instructions to do
int instruction_count = 0;

// Allocated Space for Instructions
// 4 Instructions by Default
int instruction_space = 4;

// Program Status
static bool running = true;

// If IP is assigned by JMP instructions, should not increase by 1
bool is_jmp = false;

// Get SP and IP
#define SP (registers[SP])
#define IP (registers[IP])

// Fetching Current Instruction Set
#define FETCH (instructions[IP])

// Fetching Test
// #define FETCH (test_a[IP])
// #define FETCH (test_b[IP])

// Prints the stack from A to B
void print_stack() { 
	for (int i = 0; i < SP; ++i) {
		printf("0x%04d ", stack[i]);
		if ((i + 1) % 4 == 0) {
			printf("\n");
		}
	}
	if (SP != 0) {
		printf("\n");
	}
}

void print_registers() {
	printf("Register Deump:\n");
	for (int i = 0; i < REGISTER_SIZE; ++i) {
		printf("%04d ", registers[i]);
		if ((i + 1) % 4 == 0) {
			printf("\n");
		}
	}
}

int find_empty_register() {
	for (int i = 0; i < REGISTER_SIZE; ++i) {
		if (i != registers[EX] && i != registers[EXA]) {
			return i;
		}
	}
	return EX;
}

void eval(int instr) {
	is_jmp = false;
	switch (instr) {
		case HLT: {
			running = false;
			printf("Finished Execution\n");
			break;
		}
		case PSH: {
			++SP;
			++IP;
			stack[SP] = instructions[IP];
			break;
		}
		case POP: {
			--SP;
			break;
		}
		case ADD: {
			registers[A] = stack[SP];
			--SP;
			registers[B] = stack[SP];
			registers[C] = registers[B] + registers[A];

			stack[SP] = registers[C];
			printf("%d + %d = %d\n", registers[B], registers[A], registers[C]);
            break;
		}
		case MUL: {
			registers[A] = stack[SP];
			--SP;
			registers[B] = stack[SP];
			registers[C] = registers[B] * registers[A];

			stack[SP] = registers[C];
			printf("%d * %d = %d\n", registers[B], registers[A], registers[C]);
            break;
		}
		case DIV: {
			registers[A] = stack[SP];
			--SP;
			registers[B] = stack[SP];
			registers[C] = registers[B] / registers[A];

			stack[SP] = registers[C];
			printf("%d / %d = %d\n", registers[B], registers[A], registers[C]);
            break;
		}
		case SUB: {
			registers[A] = stack[SP];
			--SP;
			registers[B] = stack[SP];
			registers[C] = registers[B] - registers[A];

			stack[SP] = registers[C];
			printf("%d - %d = %d\n", registers[B], registers[A], registers[C]);
			break;
		}
		case SLT: {
			--SP;
			stack[SP] = stack[SP + 1] < stack[SP];
			break;
		}
		case MOV: {
			registers[instructions[IP + 2]] = registers[instructions[IP + 1]];
			IP += 2;
			break; 
		}
		case SET: {
			registers[instructions[IP + 1]] = instructions[IP + 2];
			++IP;
			break;
		}
		case LOG: {
			printf("%d\n", registers[instructions[IP + 1]]);
			++IP;
			break;
		}
		case IF: {
			if (registers[instructions[IP + 1]] == instructions[IP +2 ]) {
				IP = instructions[IP + 3];
				is_jmp = true;
			} else {
				IP += 3;
			}
			break;
		}
		case IFN: {
			if (registers[instructions[IP + 1]] != instructions[IP + 2]) {
				IP = instructions[IP + 3];
				is_jmp = true;
			} else {
				IP += 3;
			}
			break;
		}
		case GLD: {
			++SP;
			++IP;
			stack[SP] = registers[instructions[IP]];
			break;
		}
		case GPT: {
			registers[instructions[IP + 1]] = stack[SP];
			++IP;
			break;
		}
		case NOP: {
			printf("Do Nothing\n");
			break;
		}
		default: {
			printf("Unknown Instruction %d\n", instr);
			break;
		}
	}
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		printf("Error: No input files\n");
		return -1;
	}

	// Filename
	char *filename = argv[1];
	FILE *file = fopen(filename, "r");

	if (!file) {
		printf("Error: Could not read file '%s'\n", filename);
		return -1;
	}

	// Allocating Space for Instructions
	instructions = malloc(sizeof(*instructions) * instruction_space);

	// Reading the Binary File
	int num;
	int i = 0;
	while (fscanf(file, "%d", &num) > 0) {
		instructions[i] = num;
		printf("%d\n", instructions[i]);
		++i;
		if (i >= instruction_space) {
			instruction_space *= 2;
			instructions = realloc(instructions, sizeof(*instructions) * instruction_space);
		}
	}

	// Set 'instruction_count' to # of instructions read
	instruction_count = i;

	// Close File
	fclose(file);

	// Initialize Stack Pointer
	SP = -1;

	// Looping through program 
	while (running && IP < instruction_count) {
		eval(FETCH);
		if (!is_jmp) {
			++IP;
		}
	}

	// Clean up Instructions
	free(instructions);

	return 0;
}
