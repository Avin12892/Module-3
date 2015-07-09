//
//  Alexander Chatham
//  Jesse Spencer
//  
//  This is a P-Machine virtual machine
//  The P- machine is a stack machine with two memory stores:
//  stack, which is organized as a stack and contains the data to be used by the PM/0 CPU, and code, 
//  which is organized as a list and contains the instructions for the VM. The PM/0 CPU has four registers. 
//  The registers are base pointer bp, stack pointer sp, program counter pc and instruction register ir.
//
//  NOTES:
//  - Input file name is hardcoded, must be named "mcode.txt" and be in the same directory
//  - Output is printed to a file named "stacktrace.txt" and is created in the working directory


#include <stdio.h>


// Constants
//
static const int MAX_STACK_HEIGHT = 2000;
static const int MAX_CODE_LENGTH = 500;
static const int MAX_LEXI_LEVELS = 3;
//
// Helpful constants
#define TRUE 1
#define FALSE 0
#define MAX(x, y) (x>y?x:y)
//
// Enums to define codes
//
// op codes
typedef enum {
    
    LIT = 1,
    OPR = 2,
    LOD = 3,
    STO = 4,
    CAL = 5,
    INC = 6,
    JMP = 7,
    JPC = 8,
    SIO = 9
    
} opcode;
//
// operators
typedef enum {
    
    RET = 0,
    NEG = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    ODD = 6,
    MOD = 7,
    EQL = 8,
    NEQ = 9,
    LSS = 10,
    LEQ = 11,
    GTR = 12,
    GEQ = 13
    
} operator;
//
// List of strings for op codes
static const char* opCodes[] = {
    "",     // 00
    "lit",  // 01
    "opr",  // 02
    "lod",  // 03
    "sto",  // 04
    "cal",  // 05
    "inc",  // 06
    "jmp",  // 07
    "jpc",  // 08
    "sio"   // 09
};


// Data structs
//
// Instructions
typedef struct {
    // opcode, L, M
    int op;
    int l;
    int m;
} instruction;


// Global storage
//
// PM/0 CPU resgiters
int sp = 0; // stack pointer
int bp = 1; // base pointer
int pc = 0; // program counter
instruction ir; // instruction register
//
// Array implementation of stack, defined using MAX_STACK_HEIGHT
int stack[MAX_STACK_HEIGHT];
//
// Code memory store, defined using MAX_CODE_LENGTH
instruction code[MAX_CODE_LENGTH];


// Function prototypes
//
int base(int level, int base);
int loadFileToMemory(FILE* input);
void outputInterpreted(FILE* output, int numberOfInstructions);
void outputCurrent(FILE* output, int line, int activationRegisterList[]);
void fetch();
int execute(int activationRegisterList[], int* numberOfAR);


int main(int argc, const char * argv[]) {
    
    int isRunning = TRUE;
    int numberOfAR = 0;
    int currentLine;
    
    int numberOfInstructions = 0;
    
    int activationRegisterList[MAX_STACK_HEIGHT];
    
    // Initialize
    //
    // stack
    stack[1] = 0;
    stack[2] = 0;
    stack[3] = 0;
    //
    // ir
    ir.op = 0;
    ir.l = 0;
    ir.m = 0;
    
    
    // Open input file
    FILE* input = fopen("mcode.txt", "r");
    // Check for success
    if (input == NULL) {
        printf("Error accessing input file");
        return -1;
    }
    
    // Load file to memory and record number of instructions
    numberOfInstructions = loadFileToMemory(input);
    // Check that return was successful
    if (numberOfInstructions < 0) {
        return -1;
    }
    
    
    // Create output file
    FILE* output = fopen("stacktrace.txt", "w");
    
    // Print to output file the program in interpreted assmbly language with line numbers
    outputInterpreted(output, numberOfInstructions);
    
    // Print initial state, called with line at -1 to print the format information
    outputCurrent(output, -1, activationRegisterList);
    
    
    // Running loop for execution
    while(isRunning) {
        
        currentLine = pc;
        
        // Fetch
        fetch();
        
        // Execution
        if ( ! execute(activationRegisterList, &numberOfAR) ) isRunning = FALSE;

        // Check that stack does not overflow
        if (sp > MAX_STACK_HEIGHT) {
            printf("Stack size exceeded");
            return -1;
        }
        
        // Output current values to file
        outputCurrent(output, currentLine, activationRegisterList);
        
    }
    
    
    fclose(input);
    fclose(output);
    return 0;
}


int base(int level, int base) {
    
    while (level > 0) {
        base = stack[base + 2];
        level--;
    }
    
    return base;
}

int loadFileToMemory(FILE* input) {
    
    // Fill list code with file data
    
    int numberOfInstructions = 0;
    
    // Check file pointer and continue with loading
    if (input) {
        
        // Loop to the end of the file
        while ( ! feof(input)) {
            
            // Check that code does not exceed MAX_CODE_LENGTH and exit if so
            if (numberOfInstructions >= MAX_CODE_LENGTH) {
                printf("MAX_CODE_LENGTH exceeded. Halting...\n");
                return -1;
            }
            
            // Scan file
            fscanf(input, "%d ", &code[numberOfInstructions].op);
            fscanf(input, "%d ", &code[numberOfInstructions].l);
            fscanf(input, "%d ", &code[numberOfInstructions].m);
            
            numberOfInstructions++;
        }
        
        return numberOfInstructions;
        
    } else return -1;
    
    
}

void outputInterpreted(FILE* output, int numberOfInstructions) {
    
    int i = 0;
    
    // Output format info
    fprintf(output, "\nLine   OP   L   M\n");
    
    // Print all the lines of interpreted input
    for (i = 0; i < numberOfInstructions; i++) {
        fprintf(output, "%4d   %s   %1d   %d\n", i, opCodes[code[i].op], code[i].l, code[i].m);
    }
    
    fprintf(output, "\n");
}

void outputCurrent(FILE* output, int line, int activationRegisterList[]) {
    
    int i;
    int k;
    
    // Check if the info header has been created
    if (line < 0) {
        fprintf(output, "                 %3s %3s %3s  Stack\n", "pc", "bp", "sp");
    }
    
    if (line < 0) {
        fprintf(output, "Initial Values   ");
    } else
        fprintf(output, "%3d %s %3d %3d  ", line, opCodes[ir.op], ir.l, ir.m);
    
    
    // Print registers
    fprintf(output, "%3d %3d %3d  ", pc, bp, sp);
    
    // Print the current stack
    for (i = 1, k = 0; i <= MAX(sp, (bp+2)); i++) {
        
        // Track when to insert a bar
        if (activationRegisterList[k] == i) {
            fprintf(output, "| ");
            k++;
        }
        
        fprintf(output, "%d ", stack[i]);
    }
    
    fprintf(output, "\n");
}

// To fetch the instruction
void fetch() {
    
    ir = code[pc++];
    
}

// To execute the instruction
int execute(int activationRegisterList[], int* numberOfAR) {
    
    
    switch( (opcode) ir.op) {
        
        case LIT:
            sp = sp + 1;
            stack[sp] = ir.m;
            break;
            
        case OPR:
            switch( (operator) ir.m) {
                case RET:
                    if(bp == 1) return FALSE;
                    
                    activationRegisterList[--(*numberOfAR)] = 0;
                    sp = bp - 1;
                    pc = stack[sp+3];
                    bp = stack[sp+2];
                    break;
                    
                case NEG:
                    stack[sp] = -stack[sp];
                    break;
                    
                case ADD:
                    (sp)--;
                    stack[sp] = stack[sp] + stack[sp+1];
                    break;
                    
                case SUB:
                    (sp)--;
                    stack[sp] = stack[sp] - stack[sp+1];
                    break;
                    
                case MUL:
                    (sp)--;
                    stack[sp] = stack[sp] * stack[sp+1];
                    break;
                    
                case DIV:
                    (sp)--;
                    stack[sp] = stack[sp] / stack[sp+1];
                    break;
                    
                case ODD:
                    stack[sp] = stack[sp] % 2;
                    break;
                    
                case MOD:
                    (sp)--;
                    stack[sp] = stack[sp] % stack[(sp)+1];
                    break;
                    
                case EQL:
                    (sp)--;
                    stack[sp] = stack[sp] == stack[sp+1];
                    break;
                    
                case NEQ:
                    (sp)--;
                    stack[sp] = stack[sp] != stack[sp+1];
                    break;
                    
                case LSS:
                    (sp)--;
                    stack[sp] = stack[sp] < stack[sp+1];
                    break;
                    
                case LEQ:
                    (sp)--;
                    stack[sp] = stack[sp] <= stack[sp+1];
                    break;
                    
                case GTR:
                    (sp)--;
                    stack[sp] = stack[sp] > stack[sp+1];
                    break;
                    
                case GEQ:
                    (sp)--;
                    stack[sp] = stack[sp] >= stack[sp+1];
                    break;
            }
            break;
            
        case LOD:
            stack[++sp] = stack[base(ir.l, bp) + ir.m];
            break;
            
        case STO:
            stack[base(ir.l, bp) + ir.m] = stack[sp];
            sp = sp - 1;
            break;
            
        case CAL:
            activationRegisterList[(*numberOfAR)++] = sp + 1;
            stack[sp + 1] = base(ir.l, bp);
            stack[sp + 2] = bp;
            stack[sp + 3] = pc;
            bp = sp + 1;
            pc = ir.m;
            break;
            
        case INC:
            sp = sp + ir.m;
            break;
            
        case JMP:
            pc = ir.m;
            break;
            
        case JPC:
            if ( ! stack[(sp)--])
                pc = ir.m;
            break;
            
        case SIO:
            if (ir.m == 0) {
                printf("%d", stack[sp]);
                sp = sp - 1;
            }
            if (ir.m == 1) {
                scanf("%d", &stack[++sp]);
            }
            // Halt execution
            if (ir.m == 2) {
                return FALSE;
            }
            break;
    }
    
    
    return TRUE;
}

