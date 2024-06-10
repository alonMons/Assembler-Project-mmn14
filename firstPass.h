#include "data_structures/node.h"
#include "structs.h"
#include <stdio.h>

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

/* the firstPass goes through the parsed lines for the first time and creates the symbol table */
int firstPass (const char* fileName, translation * translation, ParsedSyntaxLine **lines,int lineCount);

/* the handle_symbol_definition function, handles the decleration of a entry/external symbol */
void handle_symbol_definition(const char* fileName, int lineNumber, translation* translation, ParsedSyntaxLine line, int* error);

/* the checkNumOfOperands function checks if the number of operands given to an instruction is correct */
int checkNumOfOperands(const char * fileName,int lineNum, int numOfOperands, Opcode instruction);

#endif