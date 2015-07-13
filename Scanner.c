//  Alex Chatham
//  Jesse Spencer
//
//  Lexical Analyzer
//  --
//  This is a Lexical Analyzer/Scaner for the PL/0 programming language.
//  This will read a source program written in PL/0, identify some errors, and produce as output:
//  the source program (without comments),
//  the lexeme table,
//  and the list of lexemes.
//  --
//  
//  


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define MAX_SYMBOL_TABLE_SIZE 100
#define MAX_IDENTIFIER_LENGTH 11
#define MAX_NUMBER_LENGTH 5
#define NAME_SIZE 12

#define INPUT_NAME "input.txt"
#define OUTPUT_NAME "lexemetable.txt"

// Struct to hold symbols
typedef struct {
    int kind;           // const = 1, var = 2, procedure = 3
    char name[NAME_SIZE];   // reg
    int val;            // number (ASCII value)
    int level;          // L level
    int addr;           // M level
} symbol;

// Node to contain a symbol or word
typedef struct node {
    char* word;
    struct node* next;
} node;


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


// Functions
node* createNode();
node* isLetter(char firstLetter, FILE* input, node* tail, FILE* output);
node* isNumber(char firstDigit, FILE* input, node* tail, FILE* output);
node* isSymbol(char firstSymbol, FILE* input, node* tail, FILE* output);
void findLexeme(FILE* outputPointer, char* text, FILE* lexemelistPointer, symbol* table, int* numberSymbol);
int putInSymbolTable(symbol* table, char* text, int* numberSymbol);


int main() {
    
    int currentChar;
    int buffer;
    char letter;
    
    int numberOfSymbols = 0;
    
    // Create linked list
    node *head, *tail;
    head = tail = createNode();
    
    
    
    // Symbol table
    symbol table[MAX_SYMBOL_TABLE_SIZE];
    
    for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
        
        table[i].kind = 0;
        strcpy(table[i].name,  "");
        table[i].val = 0;
        table[i].level = 0;
        table[i].addr = 0;
    
    }
    
    // Open input file
    FILE* input = fopen(INPUT_NAME, "rb");
    
    if ( ! input) {
        printf("\nScanner unable to open input file.\n");
        exit(1);
    }
    
    // Create output file
    FILE *output = fopen(OUTPUT_NAME, "w+");
    
    fprintf(output, "Source Program:\n");
    
    // Prime with the first character from input
    currentChar = fgetc(input);
    
    
    // Processing of each word, removing comments
    while (currentChar  != EOF) {
        
        // if the first character is a letter, handle as a letter
        if (isalpha(currentChar)) {
            
            letter = currentChar;
            tail = isLetter(currentChar, input, tail, output);
            currentChar = fgetc(input);
        
        }
        
        // Is a number
        else if (isdigit(currentChar)) {
            
            tail = isNumber(currentChar, input, tail, output );
            currentChar = fgetc(input);
        
        }
        
        // Is a symbol
        else if (ispunct(currentChar)) {
            
            letter = currentChar;
            tail = isSymbol(currentChar, input, tail, output);
            currentChar = fgetc(input);
            
        }
        
        // White space
        else {
            fprintf(output, "%c", currentChar);
            currentChar = fgetc(input);
        }
    
    }
    
    fclose (input);
    
    // Files for output
    FILE* lexemeFP = fopen("lexemelist.txt", "w+");
    FILE* symbTabFP = fopen("symboltable.txt", "w+");
    
    
    fprintf(output, "\n\nLexeme Table:\n");
    fprintf(output, "lexeme\t\ttoken type\n");
    for (; head->next != NULL; head = head->next ) {
        fprintf(output, "%s\t\t", head->word);
        findLexeme(output, head->word, lexemeFP, table, &numberOfSymbols);
    }
    

    fclose(lexemeFP);
    
    
    fprintf(output, "\nSymbol Table:\n");
    fprintf(output, "index\t\tsymbol\n");
 
    
    for (int i = 0; i < numberOfSymbols; i++ ) {
        
        fprintf(output, "%d\t\t%s \n", i, table[i].name);
        fprintf(symbTabFP, "%s ", table[i].name);
    }
    
    
    fclose( symbTabFP);
    
    
    FILE *newIFP = fopen("lexemelist.txt", "rb");
    
    fprintf(output, "\nLexeme List:\n");
    while (fscanf(newIFP, "%d", &buffer) != EOF)
    {
        fprintf(output, "%d ", buffer);
    }
    
    
    fclose(newIFP);
    fclose (output);
}

// Make a new node
node* createNode() {
    
    node* pointer = malloc(sizeof(node));
    
    pointer->next = NULL;
    
    return pointer;
}

// If the character is a letter, procede accordingly
node* isLetter(char firstLetter, FILE* input, node* tail, FILE* output)
{
    int wordLen = 11;
    int nextLetter;
    int letterPos = 1;
    
    // create space for a word of size 11
    char *word = calloc(wordLen + 1, sizeof(char) );
    
    // put the first letter to begin building the word
    strcpy( word, "" );
    word[0] = firstLetter;
    
    // get the next character
    nextLetter = fgetc(input);
    
    // as long as the next character is a letter or number, keep adding it to
    // the word
    while( isalpha(nextLetter) || isdigit(nextLetter) )
    {
        // reallocate space if the word is too big
        if ( letterPos >= wordLen )
        {
            wordLen *= 2;
            word = realloc(word, wordLen + 1);
        }
        word[letterPos] = nextLetter;
        letterPos++;
        nextLetter = fgetc(input);
    }// ends when the next character is not a letter or number
    
    // copy the word to the linkedlist node
    tail->word = malloc( strlen(word) + 1 );
    strcpy( tail->word,  word);
    tail->next = createNode();
    
    // free up the space occupied by the word
    free(word);
    
    // go back in the file so the character can be read again in the main program
    if (nextLetter != EOF )
        fseek(input, -1, SEEK_CUR);
    
    // print the word to the file
    fprintf( output, "%s", tail->word );
    
    // return pointer to the newly created node
    return tail->next;
    
}

// If the character is a number, procede accordingly
node* isNumber(char firstDigit, FILE* input, node* tail, FILE* output)
{
    int numDigits = 5;
    int digitPos = 1;
    int nextDigit;
    
    
    // allocate space for the number and begin copying the first digit
    char *word = calloc(numDigits + 1, sizeof(char) );
    strcpy( word, "" );
    word[0] = firstDigit;
    
    // get the next character from the file
    nextDigit = fgetc(input);
    
    // as long as the next character is a number, keep adding it to the string
    while( isdigit(nextDigit) )
    {
        // reallocate space if necessary
        if ( digitPos >= numDigits )
        {
            numDigits *= 2;
            word = realloc(word, numDigits + 1);
        }
        // add new digits to the string
        word[digitPos] = nextDigit;
        digitPos++;
        nextDigit = fgetc(input);
        
    }// end if nextDigit is not a digit
    
    // if the next character is a letter, print error message and exit program
    if ( isalpha(nextDigit) )
    {
        printf("Error 22. Variable does not start with a letter. \n");
        exit(1);
    }
    
    // copy string to the linked list
    tail->word = malloc( strlen(word) + 1 );
    strcpy( tail->word,  word);
    tail->next = createNode();
    
    // go back a character so the main file can read the non-digit character
    if ( nextDigit != EOF )
        fseek(input, -1, SEEK_CUR);
    
    // free memory for the word
    free(word);
    
    // print number to output
    fprintf( output, "%s", tail->word );
    
    return tail->next;
    
}

// If the charcter is a symbol, procede accordingly
node* isSymbol(char firstSymbol, FILE* input, node* tail, FILE* output)
{
    int maxNumSymbols = 2;
    
    // allocate space and begin building string for the symbol
    char *symbol = calloc(maxNumSymbols + 1, sizeof(char) );
    
    strcpy( symbol, "" );
    symbol[0] = firstSymbol;
    
    // check for the first character, and determine string accordingly
    switch ( firstSymbol )
    {
        case '/':   // could be / or /*
        {
            char nextChar;
            
            nextChar = fgetc(input);
            
            // if the string is /*, begin comments.  Remove all characters until
            // the close comments */ string is found. Do not output any
            // characters between comments.
            if ( nextChar == '*' )
            {
                nextChar = fgetc(input);
                // repeat until the / charcter is found, but only checks right
                // after the * is found
                do
                {
                    // repeat until the * is found
                    while (nextChar != '*' )
                    {
                        // exit program with error if no closing comments found
                        if ( nextChar == EOF )
                        {
                            printf("Error 21. No end to comments. */ required. \n");
                            exit(1);
                        }
                        nextChar = fgetc(input);
                    }
                    nextChar = fgetc(input);
                } while( nextChar != '/' );
                
                // dont add new node to the linked list
                return tail;
            }
            
            // if the 2nd character is not comments go back so main program
            // can read it
            else
            {
                if ( nextChar != EOF )
                    fseek(input, -1, SEEK_CUR);
            }
            break;
        }// end case for /
            
        case '<' :      // can be <, <>, abd <=
        {
            char nextChar;
            
            int digitPos = 1;
            
            nextChar = fgetc(input);
            
            // if the next character is = or >, add to string
            if ( nextChar == '=' || nextChar == '>' )
            {
                symbol[digitPos] = nextChar;
            }
            else    // is only <, go back so character can be re-read
            {
                if ( nextChar != EOF )
                    fseek(input, -1, SEEK_CUR);
                
            }
            break;
        }// end case for <
            
        case '>' :       // can be > or >=
        {
            char nextChar;
            
            int digitPos = 1;
            
            nextChar = fgetc(input);
            
            // add = to string to make >=
            if ( nextChar == '=')
            {
                symbol[digitPos] = nextChar;
            }
            else    // if not = then go back so character can be reread by
                // main file
            {
                if ( nextChar != EOF )
                    fseek(input, -1, SEEK_CUR);
                
            }
            break;
        }// end case for >
            
        case ':' :      // can only be :=
        {
            char nextChar;
            
            int digitPos = 1;
            
            nextChar = fgetc(input);
            
            // if :=
            if ( nextChar == '=')
            {
                symbol[digitPos] = nextChar;
            }
            else    // if not :=, then is invalid symbol
            {
                printf("Error 20. Invalid symbol.  Exiting program.\n");
                exit(1);
            }
            break;
        }// end case for :
            
            // if any other approved symbol, string is ok, and nothing else needs
            // to be done
        case '+' :
        case '-' :
        case '*' :
        case '(' :
        case ')' :
        case '=' :
        case ',' :
        case '.' :
        case ';' :
            break;
            // if any other symbol, it is invalid.  Exit program.
        default :
            printf("Error 20. Invalid symbol.  Exiting program.\n");
            exit(1);
            
    }// end switch case statement
    
    // add the string for the symbol to the linked list
    tail->word = malloc( strlen(symbol) + 1 );
    strcpy( tail->word,  symbol);
    tail->next = createNode();
    
    // free space used for the symbol
    free(symbol);
    
    // print symbol to output file
    fprintf( output, "%s", tail->word );
    
    return tail->next;
    
}

// Tokenizes a node
void findLexeme(FILE* outputPointer, char* text, FILE* lexemelistPointer, symbol* table, int* numberSymbol)
{
    int index;
    
    // if the first character is a letter
    if ( isalpha(text[0]) )
    {
        // check to make sure identifier name is not too long.  Error and exit
        // if it is too long
        if (strlen( text ) > MAX_IDENTIFIER_LENGTH )
        {
            printf("Error 19. Variable name is too long. \n" );
            exit(1);
        }
        
        // print the appropriate token value to the output file and the lexeme
        // list file for all reserved words
        if ( strcmp( text, "odd") == 0 ) {
            fprintf (outputPointer, "%d\n", 8 );
            fprintf (lexemelistPointer, "%d ", 8 );
        }
        else if ( strcmp( text, "begin") == 0 ) {
            fprintf (outputPointer, "%d\n", 21 );
            fprintf (lexemelistPointer, "%d ", 21 );
        }
        else if ( strcmp( text, "end") == 0 ) {
            fprintf (outputPointer, "%d\n", 22 );
            fprintf (lexemelistPointer, "%d ", 22 );
        }
        else if ( strcmp( text, "if") == 0 ) {
            fprintf (outputPointer, "%d\n", 23 );
            fprintf (lexemelistPointer, "%d ", 23 );
        }
        else if ( strcmp( text, "then") == 0 ) {
            fprintf (outputPointer, "%d\n", 24 );
            fprintf (lexemelistPointer, "%d ", 24 );
        }
        else if ( strcmp( text, "while") == 0 ) {
            fprintf (outputPointer, "%d\n", 25 );
            fprintf (lexemelistPointer, "%d ", 25 );
        }
        else if ( strcmp( text, "do") == 0 ) {
            fprintf (outputPointer, "%d\n", 26 );
            fprintf (lexemelistPointer, "%d ", 26 );
        }
        else if ( strcmp( text, "call") == 0 ) {
            fprintf (outputPointer, "%d\n", 27 );
            fprintf (lexemelistPointer, "%d ", 27 );
        }
        else if ( strcmp( text, "const") == 0 ) {
            fprintf (outputPointer, "%d\n", 28 );
            fprintf (lexemelistPointer, "%d ", 28 );
        }
        else if ( strcmp( text, "var") == 0 ) {
            fprintf (outputPointer, "%d\n", 29 );
            fprintf (lexemelistPointer, "%d ", 29 );
        }
        else if ( strcmp( text, "procedure") == 0 ) {
            fprintf (outputPointer, "%d\n", 30 );
            fprintf (lexemelistPointer, "%d ", 30 );
        }
        else if ( strcmp( text, "write") == 0 ) {
            fprintf (outputPointer, "%d\n", 31 );
            fprintf (lexemelistPointer, "%d ", 31 );
        }
        else if ( strcmp( text, "read") == 0 ) {
            fprintf (outputPointer, "%d\n", 32 );
            fprintf (lexemelistPointer, "%d ", 32 );
        }
        else if ( strcmp( text, "else") == 0 ) {
            fprintf (outputPointer, "%d\n", 33 );
            fprintf (lexemelistPointer, "%d ", 33 );
        }
        
        else    // if it is not a reserved word, it is an identifier
        {
            // print the appropriate token "2" and add it to the symbol table
            fprintf (outputPointer, "2\n" );
            index = putInSymbolTable( table, text, numberSymbol );
            fprintf (lexemelistPointer, "2 %d ", index );
        }
    }// end if first character is letter
    
    // if the first character is a number
    else if ( isdigit(text[0]) )
    {
        // error and exit if the number has too many digits
        if (strlen( text ) > MAX_NUMBER_LENGTH )
        {
            printf("Error 17. This number is too large. \n" );
            exit(1);
        }
        // print the appropriate token "3" and add it to the symbol table
        fprintf(outputPointer, "3\n");
        index = putInSymbolTable( table, text, numberSymbol );
        fprintf (lexemelistPointer, "3 %d ", index );
    }// end if first character is number
    
    // if the first character is punctuation.  Already tested for invalid
    // characters, so only need to find which one.
    else if ( ispunct(text[0]) )
    {
        // look at first character
        switch ( text[0] )
        {
            case '+' :
                fprintf(outputPointer, "%d\n", 4);
                fprintf (lexemelistPointer, "%d ", 4);
                break;
            case '-' :
                fprintf(outputPointer, "%d\n", 5);
                fprintf (lexemelistPointer, "%d ", 5);
                break;
            case '*' :
                fprintf(outputPointer, "%d\n", 6);
                fprintf (lexemelistPointer, "%d ", 6);
                break;
            case '/' :
                fprintf(outputPointer, "%d\n", 7);
                fprintf (lexemelistPointer, "%d ", 7);
                break;
            case '(' :
                fprintf(outputPointer, "%d\n", 15);
                fprintf (lexemelistPointer, "%d ", 15);
                break;
            case ')' :
                fprintf(outputPointer, "%d\n", 16);
                fprintf (lexemelistPointer, "%d ", 16);
                break;
            case '=' :
                fprintf(outputPointer, "%d\n", 9);
                fprintf (lexemelistPointer, "%d ", 9);
                break;
            case ',' :
                fprintf(outputPointer, "%d\n", 17);
                fprintf (lexemelistPointer, "%d ", 17);
                break;
            case '.' :
                fprintf(outputPointer, "%d\n", 19);
                fprintf (lexemelistPointer, "%d ", 19);
                break;
            case '<' :     // <, <>, <=
                // check for <> and <=
                if ( strlen(text) > 1 )
                {
                    if ( strcmp(text, "<>") == 0 )
                    {
                        fprintf(outputPointer, "%d\n", 10);
                        fprintf (lexemelistPointer, "%d ", 10);
                    }
                    else if ( strcmp(text, "<=") == 0 )
                    {
                        fprintf(outputPointer, "%d\n", 12);
                        fprintf (lexemelistPointer, "%d ", 12);
                    }
                }
                else
                {
                    fprintf(outputPointer, "%d\n", 11);
                    fprintf (lexemelistPointer, "%d ", 11);
                }
                break;
            case '>' :      // > and >=
                if ( strlen(text) > 1 )
                {
                    if ( strcmp(text, ">=") == 0 )
                    {
                        fprintf(outputPointer, "%d\n", 14);
                        fprintf (lexemelistPointer, "%d ", 14);
                    }
                }
                else
                {
                    fprintf(outputPointer, "%d\n", 13);
                    fprintf (lexemelistPointer, "%d ", 13);
                }
                break;
            case ';' :
                fprintf(outputPointer, "%d\n", 18);
                fprintf (lexemelistPointer, "%d ", 18);
                break;
            case ':' :
                fprintf(outputPointer, "%d\n", 20);
                fprintf (lexemelistPointer, "%d ", 20);
                break;
                
        }
        
    }
}

// Place text into table
int putInSymbolTable(symbol* table, char* text, int* numberSymbol) {
    
    // go through each symbol in symbol table to see if string is already there
    
    for (int i = 0; i < *numberSymbol; i++) {
        // if string in symbol table, return the corresponding index
        if ( strcmp( table[i].name, text ) == 0 )
        {
            return i;
        }
    }
    
    // if string is not in symbol table, add it to the last index
    strcpy( table[*numberSymbol].name, text );
    
    // increment the last index
    *numberSymbol = *numberSymbol + 1;
    
    // return the index for the newly added string
    return ( *numberSymbol - 1 );
    
}