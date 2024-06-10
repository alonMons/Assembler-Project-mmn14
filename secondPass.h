#include "structs.h"


#ifndef SECOND_PASS_H
#define SECOND_PASS_H

/* the secondPass function's purpose is to build the translation of the program
 as binary, and ready it for file creation */
int secondPass(ParsedSyntaxLine **lines, translation *output, int lineCount, char * filename);

/* the operandType function mathes the type of the operand (which is defined in structs.h) 
    to the num of the type */
int operandType (int originalOperandType);

/* the directAddress function creates the word that deribes the adress of the symbol, 
    and returns a pointer to the symbol */
Symbol_Node * directAddress (translation *output, ParsedSyntaxLine line, int i,int j, char * filename, char * label);

/* the isNumTooLarge function checks if a number can be represented by the given bits, in the two's complement method */
int isNumTooLarge (int num, int bits);

#endif