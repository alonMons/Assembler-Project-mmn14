#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "data_structures/node.h"

/**
 * create_preprocessed_file - Processes an assembly source file to expand macros and prepare it for assembly.
 * The filename is the base name of the file to process, i.e. excluding an extension.
 * The symbol_table_head is a double pointer to the head of a linked list for symbol management.
 * 
 * This function reads an assembly file (.as), expands macros defined within it, and writes the result to a new file (.am).
 * It updates the symbol table with macro definitions.
 * It returns a PreprocessStatus indicating the success of the preprocessing, or a warning/error status if issues are encountered.
 */
PreprocessStatus create_preprocessed_file(const char* filename, Symbol_Node ** symbol_table_head, char* origialFileName, char* newFileName);

#endif