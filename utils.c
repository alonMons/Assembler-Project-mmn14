#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "globals.h"
#include "data_structures/hashtable.h"

/* Function to concatenate two strings and return the result. 
The returned string has to be freed by the caller. */
char* concatenate_strings(const char* str1, const char* str2) {
    /* Allocate memory for the new string: length of str1 + length of str2 + 1 (for the null terminator)*/
    char* result = (char*)malloc(strlen(str1) + strlen(str2) + 1);
    
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    strcpy(result, str1);
    strcat(result, str2);
    
    return result;
}

/* Trim whitespace from start */
void trim_start(char *str) {
    int i = 0, j = 0;
    while (str[i] && isspace((unsigned char)str[i])) {
        i++;
    }
    while (str[i]) {
        str[j++] = str[i++];
    }
    str[j] = '\0';
}

/* Trim whitespace from end */
void trim_end(char *str) {
    int n = strlen(str) - 1;
    while (n >= 0 && isspace((unsigned char)str[n])) {
        n--;
    }
    str[n + 1] = '\0';
}

/* Trim whitespace from both ends */
void trim(char *str) {
    trim_start(str);
    trim_end(str);
}

/* Duplicates string. The returned string has to be freed by the caller */
char *duplicate_string(const char *src) {
    char *str;
    char *p;
    int len = 0;

    /* Calculate the length of the source string */
    while (src[len])
        len++;
    
    /* Allocate memory for the new string, including space for the null terminator */
    str = (char*)malloc(len + 1);
    if (str == NULL) {
        return NULL; /* Return NULL if memory allocation fails */
    }
    
    p = str;
    
    while (*src)
        *p++ = *src++;
    
    /* Null-terminate the new string */
    *p = '\0';

    return str;
}

/* Identifies the type of assembler directive based on a given token, 
optionally including or excluding a leading dot in the comparison.*/
DirectiveType get_directive_type(const char* token, boolean include_dot) {
    if (strcmp(token, &DATA[!include_dot]) == 0) {
        return ENUM_DATA;
    } else if (strcmp(token, &STRING[!include_dot]) == 0) {
        return ENUM_STRING;
    } else if (strcmp(token, &ENTRY[!include_dot]) == 0) {
        return ENUM_ENTRY;
    } else if (strcmp(token, &EXTERN[!include_dot]) == 0) {
        return ENUM_EXTERN;
    } else {
        return ENUM_INVALID;
    }
}

/* Identifies the type of instruction opcode based on a given token */
Opcode get_opcode(const char* token) {
    if (strcmp(token, MOV) == 0) {
        return ENUM_MOV;
    } else if (strcmp(token, CMP) == 0) {
        return ENUM_CMP;
    } else if (strcmp(token, ADD) == 0) {
        return ENUM_ADD;
    } else if (strcmp(token, SUB) == 0) {
        return ENUM_SUB;
    } else if (strcmp(token, NOT) == 0) {
        return ENUM_NOT;
    } else if (strcmp(token, CLR) == 0) {
        return ENUM_CLR;
    } else if (strcmp(token, LEA) == 0) {
        return ENUM_LEA;
    } else if (strcmp(token, INC) == 0) {
        return ENUM_INC;
    } else if (strcmp(token, DEC) == 0) {
        return ENUM_DEC;
    } else if (strcmp(token, JMP) == 0) {
        return ENUM_JMP;
    } else if (strcmp(token, BNE) == 0) {
        return ENUM_BNE;
    } else if (strcmp(token, RED) == 0) {
        return ENUM_RED;
    } else if (strcmp(token, PRN) == 0) {
        return ENUM_PRN;
    } else if (strcmp(token, JSR) == 0) {
        return ENUM_JSR;
    } else if (strcmp(token, RTS) == 0) {
        return ENUM_RTS;
    } else if (strcmp(token, HLT) == 0) {
        return ENUM_HLT;
    } else {
        return ENUM_INVALID;
    }
}

/* Gets the number of the register based on the token. For example 'r2' would return 2 
and 'r9' would return ENUM_INVALID because it is out of bounds [0,7]*/
int get_register_num(const char* token) {
    if (strlen(token) == 2 &&
        token[0] == 'r' && 
        isdigit((unsigned char)token[1]) &&
        (token[1] - '0') >= 0 && 
        (token[1] - '0') <= 7) {
        return token[1] - '0';
    } else {
        return ENUM_INVALID;
    }
}

/* Returns wether a string is a number (integer) */
boolean is_number(const char* token) {
    int i = 0;
    if (token[i] == '-' || token[i] == '+') {
        i++;
    }
    for (; token[i] != '\0'; i++) {
        if (!isdigit(token[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Converts a string to a number (integer). This function assumes the input string consists of a number*/
int get_number(const char* token) {
    int i = 0;
    int sign = 1;
    int result = 0;
    if (token[i] == '-') {
        sign = -1;
        i++;
    } else if (token[i] == '+') {
        i++;
    }
    for (; token[i] != '\0'; i++) {
        result = result * 10 + (token[i] - '0');
    }
    return sign * result;
}

/* Checks if a string is a constant number or a constant defined in the constants table. */
boolean is_number_with_constants(const char* token, hashtable* constantsTable) {
    void *value = search(constantsTable, token);
    return value != NULL || is_number(token);
}

/* Converts a string to a constant number or a constant defined in the constants table. */
int get_number_with_constants(const char* token, hashtable* constants) {
    int* value;
    if (is_number(token)) {
        return get_number(token);
    }
    value = (int*) search(constants, token);
    if (value != NULL) {
        return *value;
    }
    return ENUM_INVALID;
}

/* Checks if a given operand type is allowed for a given instruction */
boolean is_operand_allowed(OperandType type, int allowed_types) {
    return (type & allowed_types) != 0;
}

/* Function to free a parsed syntax line */
void free_parsed_syntax_line(ParsedSyntaxLine* line) {
    int i;
    if (!line) return; /* Safety check */

    switch (line->type) {
        case ENUM_CONSTANT_DEFINITION:
            free(line->statement.constantDefinition.name); /* Free if dynamically allocated */
            break;

        case ENUM_INSTRUCTION:
            for (i = 0; i < line->statement.instruction.numOfOperands; ++i) {
                Operand operand = line->statement.instruction.operands[i];
                switch (operand.operandType) {
                    case OPERAND_TYPE_DIRECT:
                        free(operand.operandValue.directLabel); /* Free if dynamically allocated */
                        break;
                    case OPERAND_TYPE_INDEXED:
                        free(operand.operandValue.constantIndex.label); /* Free if dynamically allocated */
                        break;
                    /* No action needed for immediate and register types since they don't hold dynamic memory */
                    case OPERAND_TYPE_IMMEDIATE:
                    case OPERAND_TYPE_REGISTER:
                    default:
                        break;
                }
            }
            break;

        case ENUM_DIRECTIVE:
            switch (line->statement.directive.directiveType) {
                case ENUM_DATA:
                    free(line->statement.directive.directiveValue.data.values); /* Free if dynamically allocated */
                    break;
                case ENUM_STRING:
                    if (line->statement.directive.directiveValue.string != NULL)
                        free(line->statement.directive.directiveValue.string); /* Free if dynamically allocated */
                    break;
                case ENUM_ENTRY:
                    free(line->statement.directive.directiveValue.entryLabel); /* Free if dynamically allocated */
                    break;
                case ENUM_EXTERN:
                    free(line->statement.directive.directiveValue.externLabel); /* Free if dynamically allocated */
                    break;
            }
            break;

        case ENUM_COMMENT:
        case ENUM_EMPTY:
        default:
            /* These types don't hold dynamically allocated memory */
            break;
    }
    free(line);
}

/* Function to check if a string is an instruction keyword */
boolean is_instruction_keyword(const char* token) {
    return get_opcode(token) != ENUM_INVALID;
}

/* Function to check if a string is a directive keyword */
boolean is_directive_keyword(const char* token, boolean include_dot) {
    return get_directive_type(token, include_dot) != ENUM_INVALID;
}

/* Function to check if a string is a register name */
boolean is_register_keyword(const char* token) {
    return get_register_num(token) != ENUM_INVALID;
}

/* Function to check if a string is a constant definition keyword */
boolean is_const_defintion_keyword(const char* token, boolean include_dot) {
    return strcmp(token, &DEFINE[!include_dot]) == 0;
}

/* Function to check if a string is a macro keyword */
boolean is_macro_keyword(const char* token) {
    return strcmp(token, MACRO_START) == 0 || strcmp(token, MACRO_END) == 0;
}

/* Function to check if a string is a saved keyword */
boolean is_keyword(const char* token, boolean include_dot) {
    return is_instruction_keyword(token) || is_directive_keyword(token, include_dot) || 
    is_register_keyword(token) || is_const_defintion_keyword(token, include_dot) || 
    is_macro_keyword(token);
}

/* Function to check if a string is a label. 
Requirements of a valid label:
    - starts with a latin letter
    - consists only of latin letters and numbers
    - max length: 31
    - not a reserved keyword */
boolean is_label(const char* label) {
    int i, len;
    len = strlen(label);
    
    /* Check for the minimum length requirement for a label (at least 1 character) */
    if (len < 1 || len > MAX_LABEL_LENGTH) {
        return FALSE;
    }
    
    /* Check if the first character is alphabetic */
    if (!isalpha((unsigned char)label[0])) {
        return FALSE;
    }
    
    /* Check the rest of the label for valid characters (alphabetic or digit) */
    for (i = 1; i < len; i++) {
        if (!isalnum((unsigned char)label[i])) {
            return FALSE;
        }
    }
    
    return !is_keyword(label, FALSE);
}

/* Function to check if a token is a valid label (consists the : at the end)*/
boolean is_token_label(const char* token) {
    char tokenWithoutColon[MAX_LABEL_LENGTH + 1];
    int len;

    len = strlen(token);
    
    if (len < 2 || len > MAX_LABEL_LENGTH + 1) {
        return FALSE;
    }
    
    if (token[len - 1] != ':') {
        return FALSE;
    }
    
    /* Create a temporary copy of the token without the colon */
    strncpy(tokenWithoutColon, token, len - 1);
    tokenWithoutColon[len - 1] = '\0'; /* Null-terminate the string */

    return is_label(tokenWithoutColon);
}

/* Function to check if a string consists only of printable characters. */
boolean is_string_printable(const char* token) {
    int i;
    int len = strlen(token);
    for (i = 0; i < len; i++) {
        if (!isprint(token[i])) {
            return FALSE;
        }
    }

    return TRUE;
}

/* Function to check if convert the types integer to a list of strings consisting the names of the types seperated by a comma. */
void operand_types_to_string(int types, char* buffer) {
    buffer[0] = '\0'; /* Initialize buffer to empty string */

    if (types & OPERAND_TYPE_IMMEDIATE) strcat(buffer, "Immediate, ");
    if (types & OPERAND_TYPE_DIRECT) strcat(buffer, "Direct, ");
    if (types & OPERAND_TYPE_INDEXED) strcat(buffer, "Indexed, ");
    if (types & OPERAND_TYPE_REGISTER) strcat(buffer, "Register, ");

    if (strlen(buffer) > 0) {
        /* Remove the last comma and space */
        buffer[strlen(buffer) - 2] = '\0';
    }
}

/* Function to correctly inform the user of an error regarding an invalid operand.
It generates and copies a full error message to the error variable */
void sprint_operand_error(char error[], int opcode, int operandNumber, const char* token, OperandType invalidOperandType) {
    char sourceTypes[100];
    char destinationTypes[100];
    char invalidOperand[100];

    /* Convert operand types to string */
    operand_types_to_string(instructionRules[opcode].allowedSourceTypes, sourceTypes);
    operand_types_to_string(instructionRules[opcode].allowedDestinationTypes, destinationTypes);
    operand_types_to_string(invalidOperandType, invalidOperand);

    sprintf(error, "Error: Operand %d ('%s') of type %s is invalid for instruction '%s'. Allowed source types: [%s]. Allowed destination types: [%s].",
            operandNumber, token, invalidOperand, instructionRules[opcode].name, sourceTypes, destinationTypes);
}
