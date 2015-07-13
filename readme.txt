Alexander Chatham
Jesse Spencer

—
Parser & Compile Driver
—


In order for correct operation of this program, these instructions must be followed exactly, otherwise results may be incorrect.


To compile:

Compile AND run all files from the same directory, using a command line and the GCC compiler on a Unix-based system. The following command format should be used to compile all:
—
gcc nameOfFile.c -o nameOfFile
—
For compilation, the following files need to be compiled using the GCC compiler on a Unix-based system:

Scanner.c
Parser.c
PMachine.c
CompileDriver.c

To run the program:

Place the input file, named input.txt, in the same directory as the compiled unix executable files.

Run the full program using the command (still from the same directory):
./CompileDriver

The following command line arguments can be appended to the end of the previous command:

-l : to print the list of lexemes/tokens (scanner output) to the screen
-a : to print the generated assembly code (parser/codegen output) to the screen
-v : to print virtual machine execution trace (virtual machine output) to the screen

WARNING: 
Result of compiling and running on a non Unix-based system, or using a compiler other than GCC, is unknown.