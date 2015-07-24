//  Alex Chatham
//  Jesse Spencer
//
//  Parser.c
//  --
//  Should exit with code 1 for expected errors and -1 for unexpected errors (file I/O)
//  --


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_SYMBOL_TABLE_SIZE 100
#define MAX_IDENT_LENGTH 11
#define MAX_NUM_LENGTH 5
#define CODE_BUFFER 10000


// Symbol table struct
typedef struct {
    int kind;           // const = 1, var = 2, procedure = 3
    char name[12];      // name of identifier
    int val;            // value for constants / numbers
    int level;          // L level
    int addr;           // M address
} symbol;


//Identify all lexical conventions
typedef enum {
    nulsym = 1,
    identsym = 2,
    numbersym = 3,
    plussym = 4,
    minussym = 5,
    multsym = 6,
    slashsym = 7,
    oddsym = 8,
    eqlsym = 9,
    neqsym = 10,
    lessym = 11,
    leqsym = 12,
    gtrsym = 13,
    geqsym = 14,
    lparentsym = 15,
    rparentsym = 16,
    commasym = 17,
    semicolonsym = 18,
    periodsym = 19,
    becomessym = 20,
    beginsym = 21,
    endsym = 22,
    ifsym = 23,
    thensym = 24,
    whilesym = 25,
    dosym = 26,
    callsym = 27,
    constsym = 28,
    varsym = 29,
    procsym = 30,
    writesym = 31,
    readsym = 32,
    elsesym = 33
} token_type;


// Op codes
typedef enum {
    LIT = 1,
    RTN,
    LOD,
    STO,
    CAL,
    INC,
    JMP,
    JPC,
    SIO1,
    SIO2,
    SIO3,
    NEG,
    ADD,
    SUB,
    MUL,
    DIV,
    ODD,
    MOD,
    EQL,
    NEQ,
    LSS,
    LEQ,
    GTR,
    GEQ
} op_code;


// Symbol types
typedef enum {
    constant = 1,
    variable,
    procedure
} symbol_kind;


// Node to represent a token
typedef struct node {
    int token;
    struct node *next;
} node;

// Struct to hold an instruction
typedef struct {
    int op;
    int r;
    int l;
    int m;
} instruction;


// Functions
//
// Functions for processing
void program(node* currentNode);
void block(node* currentNode);
int constantDeclaration(node* currentNode);
int variableDeclaration(node* currentNode);
int procedureDeclaration(node* currentNode);
void statement(node* currentNode);
void condition(node* currentNode);
int relOp();
void expression(node* currentNode);
void checkCode(node* currentNode);
void factor(node* currentNode);
void nextLexeme(node* currentNode);
void reportError(int errorType);
void addtoSymbolTable(int symbolKind, int symListIndex);
int findToken(int token);
//
// Helper functions
node* newNode(int data);
node* insertNode(node* head, node* tail, int token);
node* getLexemeList();
int getSymbolList(symbol* st);
void storeCode(int op, int r, int l, int m);
void outputCodeToFile();
//
//
// Global variables
//
int currentToken;
int currentRegister;
symbol symbolList[50];
symbol symbolTable[100];
int symbolTableIndex;
int codeLine;
instruction code[CODE_BUFFER];
int printSuccess;
int level;

//
int main(int argc, char* argv[]) {
    
    // Initialize globals that need it
    currentRegister = -1;
    level = -1;
    
    
    if (argc > 1) {
        printSuccess = 1;
    }
    
    
    // Retrieve the lexemelist and store it
    node* currentNode;
    currentNode = getLexemeList();
    
    // Retrieve the symbol table and store it
    getSymbolList(symbolList);
    
    // Begin processing
    program(currentNode);
    outputCodeToFile();
    
    
    return 0;
}

//
void program(node* currentNode) {
    
    nextLexeme(currentNode);
    
    block(currentNode);
    
    // Error for missing period
    if (currentToken != periodsym) {
        reportError(6);
    }
    else {
        storeCode(SIO3, 0, 0, 3);
        
        if (printSuccess)
            printf("\nNo errors, program is syntactically correct.\n");
    }
    
}

//
void block(node* currentNode) {
    
    int space;
    int numberOfConstants = 0;
    int numberOfVars = 0;
    int numberOfProcs = 0;
    int jmpAddress;
    
    level++;
    
    space = 4;
    
    jmpAddress = codeLine;
    
    storeCode(JMP, 0, 0, 0);
    
    // Checks current token to call the matching function
    //
    // constsym
    if (currentToken == constsym)
        numberOfConstants = constantDeclaration(currentNode);
    
    // varsym
    if (currentToken == varsym) {
        numberOfVars = variableDeclaration( currentNode );
    }
    
    space += numberOfVars;
    
    // procsym
    if (currentToken == procsym) {
        numberOfProcs = procedureDeclaration( currentNode );
    }
    
    code[jmpAddress].m = codeLine;
    
    storeCode(INC, 0, 0 , space);
    
    statement(currentNode);
    
    symbolTableIndex = symbolTableIndex - (numberOfVars + numberOfProcs + numberOfConstants);
    
    storeCode(RTN, 0, 0, 0);
    
    
    level--;
}

//
int constantDeclaration(node* currentNode) {
    
    int symListIndex;
    int constantIndex;
    int constantValue;
    int constantCount = 0;
    
    // Get constants
    do {
        
        nextLexeme(currentNode);
        
        if (currentToken != identsym) {
            reportError(4);
        }
        
        nextLexeme(currentNode);
        
        
        symListIndex = currentToken;
        addtoSymbolTable(constant, symListIndex);
        constantCount++;
        
        
        nextLexeme(currentNode);
        
        if (currentToken != eqlsym) {
            
            if (currentToken == becomessym) {
                reportError(1);
            }
            
            else reportError(3);
        }
        
        nextLexeme(currentNode);
        
        if (currentToken != numbersym) {
            reportError(2);
        }
        
        // Assign value of the constant
        nextLexeme(currentNode);
        
        constantIndex = currentToken;
        constantValue = atoi(symbolList[constantIndex].name);
        
        symbolTable[symbolTableIndex].val = constantValue;
        
        nextLexeme(currentNode);
        
    } while (currentToken == commasym);
    
    // Here a semiocolon should be encountered
    if (currentToken != semicolonsym) {
        reportError(5);
    }
    
    nextLexeme(currentNode);
    
    
    return constantCount;
}


//
int variableDeclaration(node* currentNode) {
    
    int symListIndex;
    int variableCount = 0;
    
    // Get variables
    do {
        
        nextLexeme(currentNode);
        
        if (currentToken != identsym) {
            reportError(4);
        }
        
        nextLexeme(currentNode);
        
        symListIndex = currentToken;
        
        addtoSymbolTable(variable, symListIndex);
        symbolTable[symbolTableIndex].addr = 4 + variableCount;
        
        nextLexeme(currentNode);
        
        variableCount++;
        
    } while (currentToken == commasym);
    
    // Semicolon should be encountered
    if (currentToken != semicolonsym) {
        reportError(5);
    }
    
    nextLexeme(currentNode);
    
    
    return variableCount;
}

//
int procedureDeclaration(node* currentNode) {
    
    int symListIndex;
    int procedureCount = 0;
    
    do {
        procedureCount++;
        
        nextLexeme(currentNode);
        
        if (currentToken != identsym) {
            reportError(4);
        }
        
        nextLexeme(currentNode);
        
        symListIndex = currentToken;
        
        addtoSymbolTable(procedure, symListIndex);
        
        symbolTable[symbolTableIndex].level = level;
        symbolTable[symbolTableIndex].addr = codeLine;
        
        nextLexeme(currentNode);
        
        // Semicolon should be encountered
        if (currentToken != semicolonsym) {
            reportError(5);
        }
        
        nextLexeme(currentNode);
        
        block(currentNode);
        
        // Semicolon should be encountered
        if (currentToken != semicolonsym) {
            reportError(5);
        }
        
        nextLexeme(currentNode);
        
    } while (currentToken == procsym);
    
    
    return procedureCount;
}

//
void statement(node* currentNode) {
    
    int i;
    int index;
    int codeLineTemp;
    int codeLineTemp2;
    int codeLineTemp3;

    // identsym
    if (currentToken == identsym) {
        
        nextLexeme(currentNode);
        
        i = currentToken;
        
        index = findToken(i);
        
        if ( index == 0 ) {
            reportError(7);
        }
        
        if (symbolTable[index].kind != variable )
        {
            reportError(8);
        }
        
        nextLexeme( currentNode );
        
        if ( currentToken != becomessym )
            reportError(9);
        
        nextLexeme( currentNode );
        
        expression( currentNode );
        
        storeCode( STO, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );
        currentRegister--;
        
    }
    
    // callsym
    else if ( currentToken == callsym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
            reportError(23);
        
        nextLexeme( currentNode );
        
        i = findToken( currentToken );
        
        if ( i == 0 )
            reportError(7);
        
        if ( symbolTable[i].kind != procedure )
            reportError(24);
        
        storeCode ( CAL, 0, level - symbolTable[i].level, symbolTable[i].addr );
        
        nextLexeme( currentNode );
    }
    
    // beginsym
    else if ( currentToken == beginsym )
    {
        nextLexeme( currentNode );
        
        statement( currentNode );
        
        while ( currentToken == semicolonsym )
        {
            nextLexeme( currentNode );
            statement( currentNode );
        }
        
        if ( currentToken != endsym )
            reportError(11);
        
        nextLexeme( currentNode );
    }
    
    // ifsym
    else if ( currentToken == ifsym )
    {
        nextLexeme( currentNode );
        
        condition( currentNode );
        
        if ( currentToken != thensym )
            reportError(10);
        
        nextLexeme( currentNode );
        
        codeLineTemp = codeLine;
        storeCode( JPC, currentRegister, 0, 0 );
        currentRegister--;
        
        statement( currentNode );
        
        // elsesym
        if ( currentToken == elsesym )
        {
            nextLexeme( currentNode );
            
            codeLineTemp3 = codeLine;
            storeCode (JMP, 0, 0, 0 );
            
            code[codeLineTemp].m = codeLine;
            
            statement( currentNode );
            code[codeLineTemp3].m = codeLine;
        }
        else
        {
            code[codeLineTemp].m = codeLine;
        }
        
    }
    
    // whilesym
    else if ( currentToken == whilesym )
    {
        codeLineTemp2 = codeLine;
        
        nextLexeme( currentNode );
        
        condition( currentNode );
        
        codeLineTemp3 = codeLine;
        
        storeCode( JPC, currentRegister, 0, 0 );
        
        if ( currentToken != dosym ) {
            reportError(12);
        }
        
        nextLexeme( currentNode );
        
        statement( currentNode );
        
        storeCode( JMP, 0, 0, codeLineTemp2 );
        
        code[codeLineTemp3].m = codeLine;
        
    }
    
    // readsym
    else if ( currentToken == readsym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
        {
            reportError(18);
        }
        
        nextLexeme(currentNode);
        
        i = currentToken;
        index = findToken(i);
        
        if ( symbolTable[index].kind != variable )
        {
            reportError(11);
        }
        
        currentRegister++;
        storeCode( SIO2, currentRegister, 0, 2 );
        storeCode( STO, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );
        currentRegister--;
        
        nextLexeme( currentNode );
        
    }
    
    // writesym
    else if ( currentToken == writesym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
        {
            reportError(18);
        }
        
        nextLexeme(currentNode);
        i = currentToken;
        index = findToken(i);
        
        if ( symbolTable[index].kind != variable )
        {
            reportError(11);
        }
        
        currentRegister++;
        storeCode( LOD, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );
        storeCode( SIO1, currentRegister, 0, 1 );
        currentRegister--;
        
        nextLexeme( currentNode );
        
    }
    
    
}

//
void condition(node* currentNode) {
    
    int relOpCode;
    
    if (currentToken == oddsym) {

        nextLexeme(currentNode);
        
        expression(currentNode);
        
        storeCode(ODD, currentRegister, 0, 0);
    } else {
        
        expression( currentNode );
        
        relOpCode = relOp();
        if ( ! relOpCode ) {
            reportError(13);
        }
        
        nextLexeme( currentNode );
        
        expression( currentNode );
        
        storeCode( relOpCode, currentRegister-1, currentRegister-1, currentRegister );
        
        currentRegister--;
    }
}

//
int relOp() {
    
    switch (currentToken) {
            
        case eqlsym:
            return EQL;
            break;
        case neqsym:
            return NEQ;
            break;
        case lessym:
            return LSS;
            break;
        case leqsym:
            return LEQ;
            break;
        case gtrsym:
            return GTR;
            break;
        case geqsym:
            return GEQ;
            break;
        // Some error
        default:
            return 0;
    }
    
}

//
void expression(node* currentNode) {
    
    int addOp;
    
    if (currentToken == plussym || currentToken == minussym) {

        addOp = currentToken;
        
        nextLexeme(currentNode);
        checkCode (currentNode);
        
        storeCode(NEG, currentRegister, currentRegister, 0);
    } else {
        checkCode ( currentNode );
    }
    
    while ( currentToken == plussym || currentToken == minussym ) {

        addOp = currentToken;
        
        nextLexeme( currentNode );
        checkCode( currentNode );
        
        if ( addOp == plussym ) {
            storeCode ( ADD, currentRegister-1, currentRegister-1, currentRegister );
            currentRegister--;
        }
        if ( addOp == minussym ) {
            storeCode ( SUB, currentRegister-1, currentRegister-1, currentRegister );
            currentRegister--;
        }
    }
    
}

//
void checkCode(node *currentNode) {
    
    int multiplicationOp;
    
    factor( currentNode );
    
    while ( currentToken == slashsym || currentToken == multsym ) {

        multiplicationOp = currentToken;
        
        nextLexeme( currentNode );
        factor( currentNode );
        
        if ( multiplicationOp == multsym ) {
            storeCode( MUL, currentRegister-1, currentRegister-1, currentRegister );
            currentRegister--;
        }
        if ( multiplicationOp == slashsym ) {
            storeCode( DIV, currentRegister-1, currentRegister-1, currentRegister );
            currentRegister--;
        }
    }
    
}

//
void factor(node* currentNode) {
    
    int index;
    int i;
    int value;
    
    // identsym
    if ( currentToken == identsym ) {
        
        nextLexeme( currentNode );
        i = currentToken;
        index = findToken(i);
        
        currentRegister++;
        
        if ( symbolTable[index].kind == variable ) {
            storeCode( LOD, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );
        }
        else if ( symbolTable[index].kind == constant ) {
            storeCode( LIT, currentRegister, 0, symbolTable[index].val );
        } else {
            reportError(14);
        }
        
        nextLexeme( currentNode );
    }

    else if ( currentToken == numbersym ) {

        nextLexeme( currentNode );
        i = currentToken;
        
        value = atoi( symbolList[i].name );

        currentRegister++;
        
        storeCode( LIT, currentRegister, 0, value );
        
        nextLexeme( currentNode );
    }

    else if ( currentToken == lparentsym ) {

        nextLexeme( currentNode );
        expression( currentNode );
        
        if ( currentToken != rparentsym ) {
            reportError(15);
        }
        
        nextLexeme( currentNode );
    } else {
        reportError(16);
    }
    
}


// Output an appropriate error message
void reportError(int errorType) {
    
    printf("Error ");
    
    switch (errorType) {
            
        case 1:
            printf("1. Use = instead of :=.");
            break;
        case 2:
            printf("2. = must be followed by a number.");
            break;
        case 3:
            printf("3. Identifier must be followed by =.");
            break;
        case 4:
            printf("4. const, var, procedure must be followed by identifier.");
            break;
        case 5:
            printf("5. Semicolon or comma missing.");
            break;
        case 6:
            printf("6. Period expected.");
            break;
        case 7:
            printf("7. Undeclared identifier.");
            break;
        case 8:
            printf("8. Assignment to constant or procedure is not allowed.");
            break;
        case 9:
            printf("9. Assignment operator expected.");
            break;
        case 10:
            printf("10. then expected.");
            break;
        case 11:
            printf("11. Semicolon or end expected.");
            break;
        case 12:
            printf("12. do expected.");
            break;
        case 13:
            printf("13. Relational operator expected.");
            break;
        case 14:
            printf("14. Expression must not contain a procedure identifier.");
            break;
        case 15:
            printf("15. Right parenthesis missing.");
            break;
        case 16:
            printf("16. An expression cannot begin with this symbol.");
            break;
        case 17:
            printf("17. This number is too large.");
            break;
        case 18:
            printf("18. Read or write must be followed by an identifier.");
            break;
        case 23:
            printf("23. Call must be followed by an identifier.");
            break;
        case 24:
            printf("24. Call of a constant or variable is meaningless." );
            break;
            
        default:
            break;
            
    }
    
    printf("\n");
    
    exit(1);
}


// Retrieve the lexeme list and store it, returns the head of the list
node* getLexemeList() {
    
    int buffer;
    
    // Open lexemelist
    FILE* lexemelistPointer = fopen("lexemelist.txt", "rb");
    
    if ( ! lexemelistPointer) {
        printf("\nParser unable to open lexemelist.\n");
        exit(-1);
    }
    
    node* head = NULL;
    node* tail = NULL;
    
    fscanf(lexemelistPointer, " %d", &buffer);
    
    head = tail = insertNode(head, tail, buffer);
    
    // Scan to EOF
    while ( fscanf(lexemelistPointer, "%d", &buffer) != EOF) {
        
        tail = insertNode(head, tail, buffer);
        tail->token = buffer;
        
    }
    
    
    fclose(lexemelistPointer);
    return head;
}


// Retrieve the next lexeme in the linked list
void nextLexeme(node* currentNode) {
    
    currentToken = currentNode->token;
    
    if (currentNode->next != NULL) {
        *currentNode = *currentNode->next;
    }
    
}


// Create a new node
node* newNode(int data) {
    
    node* pointer = malloc(sizeof(node));
    
    pointer->token = data;
    pointer->next = NULL;
    
    
    return pointer;
}


// Linked list insert
node* insertNode(node* head, node* tail, int token) {
    
    // Start a list if needed
    if (head == NULL) {
        return newNode(token);
    }
    else {
        tail->next = newNode(token);
        return tail->next;
    }
    
}


// Retrieve the symbol table and store it in an array, returns the length
int getSymbolList(symbol* symList) {
    
    char buffer[MAX_IDENT_LENGTH + 1];
    int i = 0;
    
    // Open symboltable file
    FILE* symboltablePointer = fopen("symboltable.txt", "rb");
    
    if ( ! symboltablePointer) {
        printf("\nParser unable to open symbol table file.\n");
        exit(-1);
    }
    // Scan to EOF
    while (fscanf(symboltablePointer, "%s", buffer) != EOF) {
        strcpy(symList[i].name, buffer);
        i++;
    }
    
    fclose(symboltablePointer);
    
    return i;
    
}


// Print the code to output
void storeCode(int op, int r, int l, int m) {
    
    code[codeLine].op = op;
    code[codeLine].r = r;
    code[codeLine].l = l;
    code[codeLine].m = m;
    
    codeLine++;
}


// Prints code to the output file
void outputCodeToFile() {
    
    FILE* output = fopen("mcode.txt", "w");
    
    for (int i = 0; i < codeLine; i++) {
        fprintf(output, "%d %d %d %d\n", code[i].op, code[i].r, code[i].l, code[i].m);
    }
    
    fclose(output);
}


// Put symbol from Scanner into table
void addtoSymbolTable(int symbolKind, int symListIndex) {
    
    symbolTableIndex++;
    
    strcpy(symbolTable[symbolTableIndex].name, symbolList[symListIndex].name);
    
    symbolTable[symbolTableIndex].level = level;
    symbolTable[symbolTableIndex].kind = symbolKind;
}


// Locate a token in the symbol table
int findToken(int token) {
    
    int location;
    
    for (location = symbolTableIndex; location > 0; location--)
        if (strcmp(symbolTable[location].name, symbolList[token].name) == 0) {
            return location;
        }
    
    return location;
}