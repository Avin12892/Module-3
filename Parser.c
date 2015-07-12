//
//  parser.c
//  
//
//  
//


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
    LIT = 1, RTN, LOD, STO, CAL, INC, JMP, JPC, SIO1, SIO2, SIO3,
    NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
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


// Global variables
int currentToken;
int currentRegister;
symbol symbolList[50];
symbol symbolTable[100];
int symbolTableIndex;
int codeLine;
instruction code[CODE_BUFFER];
int printSuccess;
int level;


// Functions
//
// Functions for processing
void program(node* currentNode);
void block(node* currentNode);
int const_declaration(node* currentNode);
int  var_declaration(node* currentNode);
int procedure_declaration(node* currentNode);
void statement(node* currentNode);
void condition(node* currentNode);
int  rel_op();
void expression(node* currentNode);
void checkCode(node* currentNode);
void factor(node* currentNode);
void nextLexeme(node* currentNode);
void error(int errorType);
void addtoSymbolTable(int symbolKind, int symListIndex);
int findToken(int token);
//
// Helper functions
node* newNode(int data);
node* insertNode(node* head, node* tail, int token);
node* getLexemeList();
int getSymbolList(symbol* st);
void destroyNodes(node* headNode);
void emit(int op, int r, int l, int m);
void printCode();
void printNodes(node** head);


int main(int argc, char **argv) {
    
    
    currentRegister = -1;
    codeLine = 0;
    printSuccess = 0;
    level = -1;
    symbolTableIndex = 0;
    
    
    // if Compile Driver passes an argument, then print to screen the success result
    if ( argc > 1 ) {
        printSuccess = 1;
    }
    
    
    // Retrieve the lexemelist and store it
    node* currentNode;
    currentNode = getLexemeList();
    
    // Retrieve the symbol table and store it
    int symbolTableSize = getSymbolList( symbolList );
    
    
    // Begin processing
    program( currentNode );
    
    printCode();
    
    
    return 0;
}


void program( node *currentNode )
{
    // get the first token
    nextLexeme( currentNode );
    
    block( currentNode );
    
    // if the program does not end with a period, error
    if (currentToken != periodsym )
    {
        error(6);
    }
    else
    {
        emit( SIO3, 0, 0, 3 );  // SIO R 0 3 - halt program
        if ( printSuccess )
            printf("No errors, program is syntactically correct\n" );
    }
}


void block( node *currentNode )
{
    int space, numVars = 0, numProcs = 0, numConsts = 0;
    int jmpAddress, procAddress;
    int i;
    
    level++;
    /*
     if ( level == 0 )  // we are in main, so only need space for new variables
     {
     space = 0;
     }
     else    // also create space in AR for funct val, SL, DL, & RA
     {
     space = 4;
     }
     */
    space = 4;
    
    jmpAddress = codeLine;
    emit( JMP, 0, 0, 0 );
    
    // calls the appropriate function based on current token
    if (currentToken == constsym )
        numConsts = const_declaration( currentNode );
    
    if ( currentToken == varsym )
    {
        numVars = var_declaration( currentNode );
    }
    
    space += numVars;
    
    if ( currentToken == procsym )
        numProcs = procedure_declaration( currentNode );
    
    code[jmpAddress].m = codeLine;
    
    emit( INC, 0, 0 , space );
    
    statement( currentNode );
    
    
    // uncomment next section for debugging purposes
    /*
     for ( i = 0; i <= stIndex; i++ )
     printf("%s\t%d\t%d\t%d\t%d\n", symbolTable[i].name,
     symbolTable[i].kind, symbolTable[i].val, symbolTable[i].level, symbolTable[i].addr);
     */
    
    symbolTableIndex = symbolTableIndex - (numVars + numProcs + numConsts);
    emit( RTN, 0, 0, 0 );
    
    level--;
    
}


int const_declaration( node *currentNode )
{
    int symListIndex, constIndex, constValue;
    int constCount = 0;
    
    // repeat getting new constants as long as there are more
    // (indicated by a comma)
    do{
        
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
            error(4);
        
        // gets the index in the symbol table of the current identifier,
        // and changes its kind to a constant
        nextLexeme( currentNode );
        symListIndex = currentToken;
        addtoSymbolTable( constant, symListIndex );
        constCount++;
        
        //symbolTable[stIndex].kind = constant;
        
        nextLexeme( currentNode );
        if ( currentToken != eqlsym )
        {
            if ( currentToken == becomessym )
                error(1);
            else
                error(3);
        }
        
        
        nextLexeme( currentNode );
        
        if ( currentToken != numbersym )
            error(2);
        
        // get the value the constant is set to, and sets the value of the
        // constant to the value of the integer
        nextLexeme( currentNode );
        constIndex = currentToken;
        constValue = atoi( symbolList[constIndex].name );
        symbolTable[symbolTableIndex].val = constValue;
        
        nextLexeme( currentNode );
        
    } while ( currentToken == commasym );
    
    // if there are no more const, then we should have a semicolon
    if ( currentToken != semicolonsym )
        error(5);
    
    nextLexeme( currentNode );
    
    return constCount;
    
}


// returns # of variables declared
int var_declaration( node *currentNode )
{
    int symListIndex;
    int varCount = 0;
    
    // repeat getting new constants as long as there are more
    // (indicated by a comma)
    do{
        
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
            error(4);
        
        nextLexeme( currentNode );
        
        // get the symbol table index for the variable and initialize the
        // appropriate symbol table values
        symListIndex = currentToken;
        
        
        addtoSymbolTable( variable, symListIndex );
        symbolTable[symbolTableIndex].addr = 4 + varCount; // add 4 to account for other
        // items in activation record  (space from base pointer)
        
        
        nextLexeme( currentNode );
        
        // keep track of the number of variables declared
        varCount++;
        
    } while ( currentToken == commasym );
    
    // if there are no more const, then we should have a semicolon
    if ( currentToken != semicolonsym )
        error(5);
    
    nextLexeme( currentNode );
    //emit( INC, 0 , 0, varCount );     // INC
    
    return varCount;
    
}


int procedure_declaration( node *currentNode )
{
    int symListIndex, procCount = 0;
    
    do{
        procCount++;
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
            error(4);
        
        nextLexeme( currentNode );
        
        // get the symbol table index for the variable and initialize the
        // appropriate symbol table values
        symListIndex = currentToken;
        addtoSymbolTable( procedure, symListIndex );
        symbolTable[symbolTableIndex].level = level;
        symbolTable[symbolTableIndex].addr = codeLine;
        
        //printf("Procedure %s is at line %d\n", symbolTable[stIndex].name, symbolTable[stIndex].addr);
        
        nextLexeme( currentNode );
        
        // if there are no more const, then we should have a semicolon
        if ( currentToken != semicolonsym )
            error(5);
        
        nextLexeme( currentNode );
        
        block( currentNode );
        
        // if there are no more const, then we should have a semicolon
        if ( currentToken != semicolonsym )
            error(5);
        
        nextLexeme( currentNode );
        
    } while ( currentToken == procsym );
    
    return procCount;
    
}


void statement( node *currentNode )
{
    int i, ctemp, cx1, cx2;
    int index;
    
    // ident ":=" expression
    if (currentToken == identsym )
    {
        nextLexeme( currentNode );
        
        i = currentToken;
        
        index = findToken(i);
        //printf("At location %d\n", index);
        
        if ( index == 0 )
            error(7);
        
        if (symbolTable[index].kind != variable )
        {
            error(8);
        }
        
        nextLexeme( currentNode );
        
        if ( currentToken != becomessym )
            error(9);
        
        nextLexeme( currentNode );
        
        expression( currentNode );
        
        emit( STO, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );    // STO = 4
        currentRegister--;
        
    } // end ident if
    
    // "call" ident
    else if ( currentToken == callsym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
            error(23);
        
        nextLexeme( currentNode );
        
        i = findToken( currentToken );
        
        if ( i == 0 )
            error(7);
        
        if ( symbolTable[i].kind != procedure )
            error(24);
        
        // printf( "CAL on index %d for %s is at location %d\n", i, symbolTable[i].name, symbolTable[i].addr);
        emit ( CAL, 0, level - symbolTable[i].level, symbolTable[i].addr ); // CAL = 5
        
        nextLexeme( currentNode );
        
    }
    
    
    // "begin" statement { ";" statement } "end"
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
            error(11);
        
        nextLexeme( currentNode );
    }// end "begin" if
    
    
    // "if" condition "then" statement ["else" statement]
    else if ( currentToken == ifsym )
    {
        nextLexeme( currentNode );
        
        condition( currentNode );
        
        if ( currentToken != thensym )
            error(10);
        
        nextLexeme( currentNode );
        
        ctemp = codeLine;
        emit( JPC, currentRegister, 0, 0 );    // JPC
        currentRegister--;
        
        statement( currentNode );
        
        if ( currentToken == elsesym )
        {
            nextLexeme( currentNode );
            
            cx2 = codeLine;
            emit (JMP, 0, 0, 0 );
            
            code[ctemp].m = codeLine;
            
            statement( currentNode );
            code[cx2].m = codeLine;
        }
        else
        {
            code[ctemp].m = codeLine;
        }
        
    }// end "if" if
    
    // "while" condition "do" statement
    else if ( currentToken == whilesym )
    {
        cx1 = codeLine;
        
        nextLexeme( currentNode );
        
        condition( currentNode );
        
        cx2 = codeLine;
        
        emit( JPC, currentRegister, 0, 0 );    // JPC
        
        if ( currentToken != dosym )
            error(12);          // then expected
        
        nextLexeme( currentNode );
        
        statement( currentNode );
        
        emit( JMP, 0, 0, cx1 );    // JMP
        
        code[cx2].m = codeLine;
        
    }// end "while" if
    
    // "read" ident
    else if ( currentToken == readsym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
        {
            error(18);
        }
        
        nextLexeme(currentNode);
        
        i = currentToken;
        index = findToken(i);
        
        if ( symbolTable[index].kind != variable )
        {
            error(11);
        }
        
        // read in user input and store it in variable
        currentRegister++;
        emit( SIO2, currentRegister, 0, 2 );   // SIO R 0 2 - read
        emit( STO, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );    // STO
        currentRegister--;
        
        nextLexeme( currentNode );
        
    }// end "read" if
    
    // "write"  ident
    else if ( currentToken == writesym )
    {
        nextLexeme( currentNode );
        
        if ( currentToken != identsym )
        {
            error(18);
        }
        
        nextLexeme(currentNode);
        i = currentToken;
        index = findToken(i);
        
        if ( symbolTable[index].kind != variable )
        {
            error(11);
        }
        
        // write variable to screen
        currentRegister++;
        emit( LOD, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );    // LOD
        emit( SIO1, currentRegister, 0, 1 );    // SIO R 0 1 - print
        currentRegister--;
        
        nextLexeme( currentNode );
        
    }// end "write" if
    
    // empty string do nothing
    
    
}


void condition( node *currentNode )
{
    int relOpCode;
    
    // "odd" expression
    if ( currentToken == oddsym )
    {
        nextLexeme( currentNode );
        
        expression( currentNode );
        
        emit( ODD, currentRegister, 0, 0 );   // ODD
        
    }
    
    // expression rel_op expression
    else
    {
        expression( currentNode );
        
        relOpCode = rel_op( );
        if ( !relOpCode )
        {
            error(13);
        }
        
        nextLexeme( currentNode );
        
        expression( currentNode );
        
        emit( relOpCode, currentRegister-1, currentRegister-1, currentRegister );  // EQL thru GEQ
        currentRegister--;
    }
}

// returns token value for relational op, returns 0 if it is not one
int rel_op (  )
{
    switch ( currentToken )
    {
        case eqlsym:
            return EQL;  // 19
            break;
        case neqsym:
            return NEQ;  // 20
            break;
        case lessym:
            return LSS;  // 21
            break;
        case leqsym:
            return LEQ;  // 22
            break;
        case gtrsym:
            return GTR;  // 23
            break;
        case geqsym:
            return GEQ;  // 24
            break;
        default:
            return 0;   // error
    }
    
}


void expression( node *currentNode )
{
    int addop;
    
    // if there is a plus or minus symbol in front
    if ( currentToken == plussym || currentToken == minussym )
    {
        addop = currentToken;
        
        nextLexeme( currentNode );
        checkCode ( currentNode );
        
        emit( NEG, currentRegister, currentRegister, 0 );  // 12
    }
    else
    {
        checkCode ( currentNode );
    }
    // keep looping as long as there is another plus or minus symbol
    while ( currentToken == plussym || currentToken == minussym )
    {
        addop = currentToken;
        
        nextLexeme( currentNode );
        checkCode( currentNode );
        
        if ( addop == plussym )
        {
            emit ( ADD, currentRegister-1, currentRegister-1, currentRegister );    // ADD = 13
            currentRegister--;
        }
        if ( addop == minussym )
        {
            emit ( SUB, currentRegister-1, currentRegister-1, currentRegister );    // SUB = 14
            currentRegister--;
        }
    }
    
}


void checkCode(node *currentNode) {
    
    int mulOp;
    
    factor( currentNode );
    
    // keep looping if there are more divide or multiplication symbols
    while ( currentToken == slashsym || currentToken == multsym )
    {
        mulOp = currentToken;
        
        nextLexeme( currentNode );
        factor( currentNode );
        
        if ( mulOp == multsym )     // Multiplication
        {
            emit( MUL, currentRegister-1, currentRegister-1, currentRegister ); // MUL = 15
            currentRegister--;
        }
        if ( mulOp == slashsym )    // Division
        {
            emit( DIV, currentRegister-1, currentRegister-1, currentRegister ); // DIV = 16
            currentRegister--;
        }
    }
}


void factor( node *currentNode )
{
    int index, i;
    int value;
    
    // identifier
    if ( currentToken == identsym )
    {
        nextLexeme( currentNode );
        i = currentToken;
        index = findToken(i);
        
        currentRegister++;
        
        if ( symbolTable[index].kind == variable )
        {
            emit( LOD, currentRegister, level - symbolTable[index].level, symbolTable[index].addr );    // LOD = 3
        }
        else if ( symbolTable[index].kind == constant )
        {
            emit( LIT, currentRegister, 0, symbolTable[index].val );     // LIT = 1
        }
        else
        {
            error(14);
        }
        
        nextLexeme( currentNode );
    }
    // number
    else if ( currentToken == numbersym )
    {
        nextLexeme( currentNode );
        i = currentToken;
        //index = findToken(i);
        
        value = atoi( symbolList[i].name );
        currentRegister++;
        emit( LIT, currentRegister, 0, value );                   // LIT = 1
        
        nextLexeme( currentNode );
    }
    // "(" expression ")"
    else if ( currentToken == lparentsym )
    {
        nextLexeme( currentNode );
        expression( currentNode );
        
        if ( currentToken != rparentsym )
            error(15);
        
        nextLexeme( currentNode );
    }
    else
        error(16);
    
}


// Output an appropriate error message
void error(int errorType) {
    
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
    
    exit(-1);
}


// Retrieve the lexeme list and store it, returns the head of the list
node* getLexemeList() {
    
    int buffer;
    
    FILE *lexemelistPointer;
    
    // create a pointer for the input file
    lexemelistPointer = fopen("lexemelist.txt", "rb");
    
    // exit program if file not found
    if ( ! lexemelistPointer) {
        printf("Parser unable to open lexemelist\n");
        exit(1);
    }
    
    node* head = NULL;
    node* tail = NULL;
    
    fscanf(lexemelistPointer, " %d", &buffer);
    
    head = tail = insertNode(head, tail, buffer);
    
    // keep scanning until reaching the end of the file
    while ( fscanf(lexemelistPointer, "%d", &buffer) != EOF) {
        
        tail = insertNode(head, tail, buffer);
        tail->token = buffer;
        
    }
    
    
    fclose(lexemelistPointer);
    return head;
}


// Retrieve the next tokenized lexeme from the linked list
void nextLexeme(node *currentNode) {
    
    currentToken = currentNode->token;
    
    if (currentNode->next != NULL) {
        *currentNode = *currentNode->next;
    }
    
}


// insert a new node into the linked list
node *insertNode(node* head, node* tail, int token)
{
    // if this is the first node
    if ( head == NULL )
    {
        return newNode( token );
    }
    else
    {
        tail->next = newNode( token );
        return tail->next;
    }
    
}


node *newNode(int data) {
    
    node *pointer = malloc(sizeof(node));
    
    pointer->token = data;
    pointer->next = NULL;
    
    
    return pointer;
}


// Retrieve the symbol table and store it in an array, returns the length
int getSymbolList(symbol* symList) {
    
    char buffer[MAX_IDENT_LENGTH + 1];
    int i = 0;
    
    FILE *lexemelistPointer;
    
    // create a pointer for the input file
    lexemelistPointer = fopen("symboltable.txt", "rb");
    
    // exit program if file not found
    if (lexemelistPointer == NULL) {
        printf("File with symbol table info not found\n");
        exit(1);
    }
    
    // keep scanning until reaching the end of the file
    while ( fscanf(lexemelistPointer, "%s", buffer) != EOF)
    {
        strcpy(symList[i].name, buffer );
        //st[i].kind = -1;
        //st[i].val = -1;
        //st[i].level = -1;
        //st[i].addr = -1;
        i++;
    }
    
    fclose( lexemelistPointer );
    
    return i;
    
}// end function getSymbolTable



// Print the code to output
void emit( int op, int r, int l, int m )
{
    code[codeLine].op = op;
    code[codeLine].r = r;
    code[codeLine].l = l;
    code[codeLine].m = m;
    
    //fprintf( outFP, "%d %d %d %d\n", op, r, l, m );
    
    codeLine++;
    
}


// prints the generated code to the code.txt output file
void printCode()
{
    int i;
    
    FILE *ofp;
    ofp = fopen("mcode.txt", "w");
    
    for ( i = 0; i < codeLine; i++ )
    {
        fprintf( ofp, "%d %d %d %d\n", code[i].op, code[i].r, code[i].l,
                code[i].m );
    }
    fclose(ofp);
}


// adds the symbol from the symbol table list (from Scanner) to the symbol table
void addtoSymbolTable( int symbolKind, int symListIndex ) {
    
    symbolTableIndex++;
    
    // copy name from symbolList to symbolTable
    strcpy( symbolTable[symbolTableIndex].name, symbolList[symListIndex].name );
    
    symbolTable[symbolTableIndex].level = level;
    symbolTable[symbolTableIndex].kind = symbolKind;
    
}


// finds the location of the token in the symbol table
int findToken( int token ) {
    
    int location;
    
    for ( location = symbolTableIndex; location > 0; location-- )
        if ( strcmp( symbolTable[location].name, symbolList[token].name ) == 0 )
            return location;
    
    return location;
}