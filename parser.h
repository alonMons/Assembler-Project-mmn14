#include "structs.h"
#include "constants.h"
#include "data_structures/node.h"
#include "data_structures/hashtable.h"

/* Takes a line and returns a tokenized array of strings which are the tokens of the line
 For example: "add r1, r2, r3" would return ["add", "r1", ",", "r2", "," "r3"]. 
 The seperating delimiters are
 - whitespace ( )
 - commas (,)
 - equal sign (=)

Note: it ignores these rules if it's inside quotes or square barckets.
For example, "  bne X[2 ]" would translate to ["bne", "X[2 ]"] */
node* tokenize_line(const char line[MAX_LINE_LENGTH + 1], int* allocationError);

/* Function to parse a line and return a ParsedSyntaxLine struct representin the parsed symbols of the line. 
If an error has occured then the return's value error property will contain a different character than a '\0'. In that case the statement data inside the return value is undefined*/
ParsedSyntaxLine* parse_line(char line[MAX_LINE_LENGTH + 1], hashtable* constantsTable, Symbol_Node ** symbol_table_head);

/* Function to parse a file and return an array of ParsedSyntaxLine structs consisting of the parsed data of the file as AST */
ParsedSyntaxLine** parse_file(const char* file_name, int* line_count, Symbol_Node ** symbol_table_head, int* error);