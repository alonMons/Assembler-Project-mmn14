#include "structs.h"
#include "data_structures/hashtable.h"

/* Function to concatenate two strings and return the result */
char* concatenate_strings(const char* str1, const char* str2);

/* Trim leading whitespace */
void trim_start(char *str);

/* Trim trailing whitespace */
void trim_end(char *str);

/* Trim leading and trailing whitespace */
void trim(char *str);

/* Duplicate a string */
char *duplicate_string(const char *src);

/* Identifies the type of assembler directive based on a given token, 
optionally including or excluding a leading dot in the comparison.*/
DirectiveType get_directive_type(const char* token, boolean include_dot);

/* Identifies the type of instruction opcode based on a given token */
Opcode get_opcode(const char* token);

/* Gets the number of the register based on the token. For example 'r2' would return 2 and 'r9' would return ENUM_INVALID because it is out of bounds [0,7]*/
int get_register_num(const char* token);

/* Returns wether a string is a number (integer) */
boolean is_number(const char* token);

/* Converts a string to a number (integer). This function assumes the input string consists of a number*/
int get_number(const char* token);

/* Checks if a string is a constant number or a constant defined in the constants table. */
boolean is_number_with_constants(const char* token, hashtable* constantsTable);

/* Converts a string to a constant number or a constant defined in the constants table. */
int get_number_with_constants(const char* token, hashtable* constants);

/* Checks if a given operand type is allowed for a given instruction */
boolean is_operand_allowed(OperandType type, int allowed_types);

/* Function to free a parsed syntax line */
void free_parsed_syntax_line(ParsedSyntaxLine* line);

/* Function to check if a string is an instruction keyword */
boolean is_instruction_keyword(const char* token);

/* Function to check if a string is a directive keyword */
boolean is_directive_keyword(const char* token, boolean include_dot);

/* Function to check if a string is a register keyword */
boolean is_register_keyword(const char* token);

/* Function to check if a string is a constant definition keyword */
boolean is_const_defintion_keyword(const char* token, boolean include_dot);

/* Function to check if a string is a reserved keyword */
boolean is_keyword(const char* token, boolean include_dot);

/* Function to check if a string is a label. 
Requirements of a valid label:
    - starts with a latin letter
    - consists only of latin letters and numbers
    - max length: 31
    - not a reserved keyword */
boolean is_label(const char* label);

/* Same as is_label except this function checks for the : at the end*/
boolean is_token_label(const char* token);

/* Function to check if a string consists only of printable characters. */
boolean is_string_printable(const char* token);

/* Function to check if convert the types integer to a list of strings consisting the names of the types seperated by a comma. */
void operand_types_to_string(int types, char* buffer);

/* Function to correctly inform the user of an error regarding an invalid operand.
It generates and copies a full error message to the error variable */
void sprint_operand_error(char error[], int opcode, int operandNumber, const char* token, OperandType invalidOperandType);