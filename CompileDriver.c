//  Alex Chatham
//  Jesse Spencer
//  
//  CompileDriver.c
//  --
//
//  --

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define true 1
#define false 0
#define CALL_STRING_BUFFER 25


// Global variables for command line arguments (compiler directives)
// Implicitly initialized to 0 (because they are static variables)
//
int directivePrintLexemes;
int directivePrintAssembly;
int directivePrintVMTrace;


// Functions
void checkDirectives(int argc, const char* argv[]);
void printLexemeList();
void printAssemblyCode();
void printVMExecutionTrace();


int main(int argc, const char * argv[]) {
    
    
    char callScanner[CALL_STRING_BUFFER];
    char callParser[CALL_STRING_BUFFER];
    char callPMachine[CALL_STRING_BUFFER];
    
    strcpy(callScanner, "./Scanner");
    strcpy(callParser, "./Parser");
    strcpy(callPMachine, "./PMachine");
    
    
    // Check for the compiler directives (command line args)
    checkDirectives(argc, argv);
    
    
    if (directivePrintAssembly) {
        strcat(callParser, " -a");
    }
    
    
    // Use to call each section and check for success, determine whether or not to continue
    int returnValue = system(callScanner);
    
    if (returnValue < 0 || WEXITSTATUS(returnValue) != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }
    
    returnValue = system(callParser);
    
    if (returnValue < 0 || WEXITSTATUS(returnValue) != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }
    
    // After the Parser has successfully completed
    printf("No errors, program is syntactically correct.\n");
    
    returnValue = system(callPMachine);
    
    
    // Now check call respective funtions for the command line args
    //
    if (directivePrintLexemes) {
        printLexemeList();
    }
    
    if (directivePrintAssembly) {
        printAssemblyCode();
    }
    
    if (directivePrintVMTrace) {
        printVMExecutionTrace();
    }
    
    
    
    
    return 0;
}


void checkDirectives(int argc, const char* argv[]) {
    
    
    for (int i = 1; i < argc; i++) {
        
        if ( (strcmp(argv[i], "-l")) == 0) {
            directivePrintLexemes = true;
        }
        
        else if ( (strcmp(argv[i], "-a")) == 0) {
            directivePrintAssembly = true;
        }
        
        else if ( (strcmp(argv[i], "-v")) == 0) {
            directivePrintVMTrace = true;
        }
        
        else printf("Unrecognized directive: %s", argv[i]);
        
    }
    
    
}


void printLexemeList() {
    
    // Open lexeme list file
    FILE* lexemeListPointer = fopen("lexemelist.txt", "r");
    
    
    if ( ! lexemeListPointer) {
        printf("lexemelist.txt could not be opened");
        exit(-1);
    }
    
    
    int currentChar;
    
    while ( (currentChar = fgetc(lexemeListPointer)) != EOF) {
        printf("%c", currentChar);
    }
    
    printf("\n\n");
    
    
    FILE* symbolTablePointer = fopen("symboltable.txt", "r");
    
    printf("\n\nPrinting out the symbol table:\n");
    
    if ( ! symbolTablePointer) {
        printf("\Error finding symbol table file output\n\n");
        exit(1);
    }
    
    while ( (currentChar = fgetc(symbolTablePointer)) != EOF ) {
        printf("%c", currentChar);
    }
    
    printf("\n\n");
    
    
    fclose(lexemeListPointer);
    fclose(symbolTablePointer);
}

void printAssemblyCode() {
    
    // Open mcode.txt file
    FILE* parserPointer = fopen("mcode.txt", "r");
    
    
    if ( ! parserPointer) {
        printf("\n\nError finding Parser output\n");
        exit(1);
    }
    
    int currentCharacter;
    
    while ( (currentCharacter = fgetc(parserPointer)) != EOF) {
        printf("%c", currentCharacter);
    }
    
    printf("\n\n");
    
    
    fclose(parserPointer);
}

void printVMExecutionTrace() {
    
    // Open stacktrace file
    FILE* VMPointer = fopen("stacktrace.txt", "r");
    
    
    printf("\n\nPrinting out Virtual Machine output:\n\n");
    
    if (! VMPointer) {
        printf("\nError finding VM output\n");
        exit(1);
    }
    
    int currentChar;
    
    while ( (currentChar = fgetc(VMPointer)) != EOF) {
        printf("%c", currentChar);
    }
    
    printf("\n\n");
    
    
    fclose(VMPointer);
}