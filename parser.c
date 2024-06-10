#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "data_structures/node.h"
#include "data_structures/hashtable.h"
#include "globals.h"
#include "data_structures/node.h"

/* Takes a line and returns a tokenized array of strings which are the tokens of the line
 For example: "add r1, r2, r3" would return ["add", "r1", ",", "r2", "," "r3"]. 
 The seperating delimiters are
 - whitespace ( )
 - commas (,)
 - equal sign (=)

Note: it ignores these rules if it's inside quotes or square barckets.
For example, "  bne X[2 ]" would translate to ["bne", "X[2 ]"] */
node* tokenize_line(const char line[MAX_LINE_LENGTH + 1], int* allocationError) {
    const char* cursor;
    node* head = NULL;
    char buffer[MAX_LINE_LENGTH + 1], temp[2]; /* Temporary buffer for tokens */
    
    int bufferIndex = 0;
    int inBracket = 0, inString = 0; /* Track whether we're inside brackets or strings */
    *allocationError = 0;

    for (cursor = line; *cursor != '\0'; cursor++) {
        /* Handle entering and exiting strings */
        if (*cursor == '"') {
            inString = TRUE; /* We're inside a string now and it is supposed
             to cotninue until the end of the line
             NOTE: THIS CODE WILL IGNORE ANOTHER " AND WILL ADD IT TO THE STRING AS SPECIFIED IN
             https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3188445&p=7558112#p7558112*/
            if (bufferIndex < MAX_LINE_LENGTH) {
                buffer[bufferIndex++] = *cursor; /* Add '"' to buffer */
            }
            continue;
        }

        if (inString) {
            /* If we're inside a string, add the character to the buffer */
            if (bufferIndex < MAX_LINE_LENGTH) { /* Prevent buffer overflow */
                buffer[bufferIndex++] = *cursor;
            }
            continue;
        }

        /* Handle entering and exiting brackets */
        if (*cursor == '[') {
            inBracket = 1; /* We're now inside a bracket */
            buffer[bufferIndex++] = *cursor;
        } else if (*cursor == ']') {
            inBracket = 0; /* We've exited the bracket */
            buffer[bufferIndex++] = *cursor;
        }
        /* Check for ',' or '=' or whitespace (if not in brackets) */
        else if (*cursor == ',' || *cursor == '=' || (isspace((unsigned char)*cursor) && !inBracket)) {
            if (bufferIndex > 0) {
                buffer[bufferIndex] = '\0';
                head = insert_node(head, buffer); /* Insert the token into the list */
                if (head == NULL) { 
                    *allocationError = 1;
                    return NULL;
                }
                bufferIndex = 0; /* Reset buffer index for next token */
            }
            /* Insert ',' or '=' as separate tokens */
            if (*cursor == ',' || *cursor == '=') {
                temp[0] = *cursor; /* Assign the current character to the first element */
                temp[1] = '\0';    /* Null-terminate the string */
                head = insert_node(head, temp); /* Insert the token into the list */
                if (head == NULL) { 
                    *allocationError = 1;
                    return NULL;
                }
            }
        } else {
            if (bufferIndex < MAX_LINE_LENGTH) { /* Prevent buffer overflow */
                buffer[bufferIndex++] = *cursor; /* Add character to buffer */
            }
        }
    }

    /* Handle the last token if there is one */
    if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0'; /* Null-terminate the last token */
        head = insert_node(head, buffer); /* Insert the last token into the list */
        if (head == NULL) { 
            *allocationError = 1;
            return NULL;
        }
    }

    return head;
}

/* This function checks wether a token represents an indexed constant e.g. X[2]. 
If the token is indeed an indexed label it will store the name of the label in result_label and will store the constant index in the index variable.
Otherwise the contents of result_label and index variable will be undefined */
boolean is_indexed(const char* token, hashtable* constantsTable, int* index, char** result_label) {
    int temp;
    size_t label_length, num_length;
    char *label, *num;
    /* Find the position of the opening and closing brackets */
    const char *open_bracket = strchr(token, '[');
    const char *close_bracket = strchr(token, ']');

    /* Check that both brackets are present and in the correct order */
    if (!open_bracket || !close_bracket || close_bracket < open_bracket) {
        return FALSE;
    }

    /* Check that there are characters before the '[' (the label) */
    if (open_bracket == token) {
        return FALSE;
    }

    /* Check that there are no characters after the ']' */
    if (*(close_bracket + 1) != '\0') {
        return FALSE;
    }

    /* Check that the label is valid
     Create a substring for the label part */
    label_length = open_bracket - token;
    label = (char*)malloc(label_length + 1);
    strncpy(label, token, label_length);
    label[label_length] = '\0';

    if (!is_label(label)) {
        free(label);
        return FALSE;
    }

    /* Check that the number/constant part is valid
     Create a substring for the number/constant part */
    num_length = close_bracket - open_bracket - 1;
    num = (char*)malloc(num_length + 1);
    strncpy(num, open_bracket + 1, num_length);
    num[num_length] = '\0';
    trim(num); /* Ignore whitespaces in index according to https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3181019&p=7536805#p7536805 */

    if (!is_number_with_constants(num, constantsTable)) {
        free(label);
        free(num);
        return FALSE;
    }

    temp = get_number_with_constants(num, constantsTable);
    if (temp < 0) {
        free(label);
        free(num);
        return FALSE;
    }

    free(num);
    *index = temp;
    *result_label = label;
    return TRUE;
}

/* This function parses a line into a DirectiveStatement struct and stores the result in result */
void parse_directive(node* tokens, DirectiveType type, hashtable* constantsTable, ParsedSyntaxLine* result) {
    char* token;
    int i, need_comma, len;
    if (tokens == NULL) {
        strcpy(result->error, "Invalid directive, no tokens found");
        return;
    }
    switch (type) {
        case ENUM_DATA:
            /* Allocate memory for the values of the .data declaration */
            result->statement.directive.directiveValue.data.values = malloc(sizeof(int) * MAX_DATA_LENGTH);
            i = 0;
            need_comma = 0; /* A flag to indicate if we need */
            while (tokens != NULL) {
                token = tokens->token;
                if (!need_comma && is_number_with_constants(token, constantsTable)) {
                    result->statement.directive.directiveValue.data.values[i] = get_number_with_constants(token, constantsTable);
                    i++;
                    need_comma = TRUE;
                } else if (need_comma && strcmp(token, ",") == 0) {
                    need_comma = FALSE;
                } else if (need_comma) {
                    sprintf(result->error, "Invalid data directive, expected comma before %s", token);
                    return;
                } else {
                    sprintf(result->error, "Invalid data value: '%s'", token);
                    return;
                }
                tokens = tokens->next;
            }
            if (need_comma == FALSE) {
                strcpy(result->error, "Invalid data directive, unexpected comma");
                return;
            }
            result->statement.directive.directiveValue.data.count = i;
            result->statement.directive.directiveValue.data.values = realloc(result->statement.directive.directiveValue.data.values, sizeof(int) * i);
            break;
        case ENUM_STRING:
            token = tokens->token;
            len = strlen(token);
            if (len < 2) { /* this checks needs to be here becaus if the length is less than 2 then " could be seen as a valid string*/
                sprintf(result->error, "Invalid string value: %s", token);
                return;
            }

            if (token[0] == '"' && token[strlen(token) - 1] == '"') {
                size_t newLength = strlen(token) - 2; 
                char* newString = malloc(newLength + 1); /* (+1 for null terminator). */
                if (newString == NULL) {
                    return;
                }
                strncpy(newString, token + 1, newLength);
                newString[newLength] = '\0';
                if (!is_string_printable(newString)) {
                    sprintf(result->error, "Invalid string value, string has unprintable characters");
                    free(newString);
                    return;
                }
                result->statement.directive.directiveValue.string = newString;
            } else {
                sprintf(result->error, "Invalid string value: %s", token);
                return;
            }
            if (tokens->next != NULL) {
                sprintf(result->error, "Invalid string directive, unexpected token: '%s'", tokens->next->token);
                return;
            }
            break;
        case ENUM_ENTRY:
            token = tokens->token;
            if (is_label(token)) {
                result->statement.directive.directiveValue.entryLabel = duplicate_string(token);
            } else {
                sprintf(result->error, "Invalid entry label: %s", token);
                return;
            }
            if (tokens->next != NULL) {
                sprintf(result->error, "Invalid entry directive, unexpected token %s after label", tokens->next->token);
                return;
            }
            break;
        case ENUM_EXTERN:
            token = tokens->token;
            if (is_label(token)) {
                result->statement.directive.directiveValue.externLabel = duplicate_string(token);
            } else {
                sprintf(result->error, "Invalid extern label: %s", token);
                return;
            }
            if (tokens->next != NULL) {
                sprintf(result->error, "Invalid extern directive, unexpected token %s after label", tokens->next->token);
                return;
            }
            break;
        default:
            strcpy(result->error, "Invalid directive type");
            return;
            
    }
    return;
}

/* This function parses a token into an Operand struct and stores the result in result */
Operand* parse_operand(const char* token, hashtable* constantsTable, ParsedSyntaxLine* result) {
    int len;
    int immediateIndex = 0;
    char* label;
    Operand* operand = malloc(sizeof(Operand));
    len = strlen(token);
    if (len == 0) {
        free(operand);
        return NULL;
    };
    /* Parse the operand */
    if (token[0] == '#' && len > 1) {
        if (!is_number_with_constants(token + 1, constantsTable)) {
            sprintf(result->error, "Invalid immediate operand: %s", token);
            free(operand);
            return NULL;
        }
        operand->operandType = OPERAND_TYPE_IMMEDIATE;
        operand->operandValue.immediate = get_number_with_constants(token + 1, constantsTable);
    } else if (is_label(token)) {
        if (is_number_with_constants(token, constantsTable)) {
            sprintf(result->error, "Uncompatible operand: %s is a constant. However, it is used as a label. Perhaps you forgot a #?", token);
            free(operand);
            return NULL;
        }
        operand->operandType = OPERAND_TYPE_DIRECT;
        operand->operandValue.directLabel = duplicate_string(token);
    } else if (is_register_keyword(token)) {
        operand->operandType = OPERAND_TYPE_REGISTER;
        operand->operandValue.directRegisterNum = get_register_num(token);
    } else if (is_indexed(token, constantsTable, &immediateIndex, &label)) {
        operand->operandType = OPERAND_TYPE_INDEXED;
        operand->operandValue.constantIndex.label = duplicate_string(label); /* we duplicate it because we will need the variable later*/
        operand->operandValue.constantIndex.value = immediateIndex;
        free(label); /* the is_indexed funtion allocates memory for label */
    }
    else {
        sprintf(result->error, "Uncompatible operand: %s", token);
        free(operand);
        return NULL;
    }
    return operand;
}

/* This function checks if an operand type is allowed by the instruction, and the postion of the operand. */
boolean is_operand_allowed_by_index(Operand* operand, Opcode opcode, int index) {
    int numOfOperandsRequired = instructionRules[opcode].numberOfOperandsRequired;
    if (numOfOperandsRequired == 0) {
        return FALSE;
    }
    if (numOfOperandsRequired == 1) { /* If there is one operand
    in the instruction then it is always the destination*/
        return is_operand_allowed(operand->operandType, instructionRules[opcode].allowedDestinationTypes);
    }
    if (numOfOperandsRequired == 2) {
        if (index == 0) { /* The first operand is the source */
            return is_operand_allowed(operand->operandType, instructionRules[opcode].allowedSourceTypes);
        } else { /* The second operand is the destination */
            return is_operand_allowed(operand->operandType, instructionRules[opcode].allowedDestinationTypes);
        }
    }
    return FALSE;
}

/* This function parses a line into an InstructionStatement struct and stores the result in result */
void parse_instruction(node* tokens, Opcode opcode, hashtable* constantsTable, ParsedSyntaxLine* result) {
    Operand* currentOperand = NULL;
    char* token;
    int need_comma = 0; /* A flag indicating whether a comma is required now as a token */
    int i, maxOperands = instructionRules[opcode].numberOfOperandsRequired;
    result->statement.instruction.numOfOperands = 0;
    
    if (tokens == NULL && maxOperands > 0) { /* There are no operands */
        strcpy(result->error, "Invalid instruction, no operands found");
        return;
    }
    i = 0;
    while (tokens != NULL) {
        if (i >= maxOperands) {
            sprintf(result->error, "Too many operands. The maximum number of operands for \"%s\" is %d", 
            instructionRules[opcode].name, maxOperands);
            goto end;
        }
        token = tokens->token;
        if (!need_comma) {
            currentOperand = parse_operand(token, constantsTable, result);
            if (currentOperand == NULL) {
                /* No need to put in error because parse_operand takes care of that*/
                goto end;
            }
            
            if (!is_operand_allowed_by_index(currentOperand, opcode, i)) {
                /* Format a correct error message */
                sprint_operand_error(result->error, opcode, i + 1, token, currentOperand->operandType);
                free(currentOperand);
                goto end;
            }
            result->statement.instruction.operands[i] = *currentOperand;
            result->statement.instruction.numOfOperands++;
            need_comma = TRUE;
            i++;
            free(currentOperand);
        } else if (strcmp(token, ",") == 0) {
            need_comma = FALSE;
        } else {
            sprintf(result->error, "Expected comma but has: %s", token);
            goto end;
        }
        
        tokens = tokens->next;
    }
    end:
    return;
}

/* This function parses a constant defintion statement into a ConstantDefintionStatement and stores it in result */
void parse_constant_defintion(node* tokens, hashtable* constantsTable, ParsedSyntaxLine* result, Symbol_Node ** symbol_table_head) {
    char* token;
    char* name;
    Symbol_Node * symbolJ;
    int len, value;
    if (strlen(result->labelName) > 0) {
        strcpy(result->error, "Invalid constant definition. Cannot have a label in a constant defintion statement.");
        return;
    }

    if (tokens == NULL) {
        strcpy(result->error, "Invalid constant definition, no tokens found");
        return;
    }
    token = tokens->token;
    len = strlen(token);
    if (len == 0) {
        strcpy(result->error, "Invalid constant definition, empty token");
        return;
    }
    if (!is_label(token)) {
        sprintf(result->error, "Invalid constant definition, invalid name: %s", token);
        return;
    }
    if (search(constantsTable, token) != NULL) {
        sprintf(result->error, "Invalid constant definition, constant already defined: %s", token);
        return;
    } else if (symbol_contains(*symbol_table_head, token) != NULL) {
        sprintf(result->error, "Invalid constant definition, label already defined: %s", token);
        return;
    }
    name = token;
    tokens = tokens->next;
    if (tokens == NULL) {
        strcpy(result->error, "Invalid constant definition, no tokens found after label");
        return;
    }
    token = tokens->token;
    if (strcmp(token, "=") != 0) {
        sprintf(result->error, "Invalid constant definition, expected '=' but has: %s", token);
        return;
    }
    tokens = tokens->next;
    if (tokens == NULL) {
        strcpy(result->error, "Invalid constant definition, no tokens found after '='");
        return;
    }
    token = tokens->token;
    if (!is_number_with_constants(token, constantsTable)) {
        sprintf(result->error, "Invalid constant definition, invalid value: %s", token);
        return;
    }
    tokens = tokens->next;
    if (tokens != NULL) {
        strcpy(result->error, "Invalid constant definition, unexpected token after value");
        return;
    }
    /* Add the constant name to the constant table and symbols list */
    value = get_number_with_constants(token, constantsTable);
    insert(constantsTable, name, &value, sizeof(int));
    symbolJ = insert_symbol(symbol_table_head, name);
    symbolJ->symbol->type = ENUM_SYMBOL_CONSTANT_MACRO;
    result->statement.constantDefinition.name = duplicate_string(name);
    result->statement.constantDefinition.value = value;
    return;
}

void initializeParsedMemory (ParsedSyntaxLine* parsed_line) {
    parsed_line->error[0] = '\0';
    parsed_line->labelName[0] = '\0';
    parsed_line->type = ENUM_INVALID;
    parsed_line->statement.directive.directiveValue.string = NULL;
}

/* Function to parse a line and return a ParsedSyntaxLine struct representin the parsed symbols of the line. 
If an error has occured then the return's value error property will contain a different character than a '\0'. In that case the statement data inside the return value is undefined*/
ParsedSyntaxLine* parse_line(char line[MAX_LINE_LENGTH + 1], hashtable* constantsTable, Symbol_Node ** symbol_table_head) {
    node *tokens = NULL, *firstToken = NULL, *symbolNames = NULL;
    int allocationError = 0;
    ParsedSyntaxLine* parsed_line =  (ParsedSyntaxLine *)calloc(1, sizeof(ParsedSyntaxLine));
    if (parsed_line == NULL) {
        return NULL;
    }
    initializeParsedMemory(parsed_line);

    /* A line is a comment if and only if its first character is ; */
    if (line[0] == COMMENT) {
        parsed_line->type = ENUM_COMMENT;
        goto end;
    }

    /* remove leading and trailing whitespace */
    trim(line);

    if (line[0] == '\0' || line[0] == '\n') {
        parsed_line->type = ENUM_EMPTY;
        goto end;
    }

    /* It's easier to parse a list of tokens in a line instead of one string */
    tokens = tokenize_line(line, &allocationError);
    if (allocationError) {
        strcpy(parsed_line->error, "Failed to tokenize line");
        goto end;
    }
    firstToken = tokens;
    if (tokens == NULL) {
        parsed_line->type = ENUM_EMPTY;
        goto end;
    }
    /* Check for label as the first token of the line */
    if (is_token_label(tokens[0].token)) {
        tokens[0].token[strlen(tokens[0].token) - 1] = '\0'; /* Remove the colon from the label */
        if (contains(symbolNames, tokens[0].token)) { /* Check if the label is already defined */
            sprintf(parsed_line->error, "Trying to redefine the symbol \"%s\"", tokens[0].token);
            goto end;
        }
        symbolNames = insert_node(symbolNames, tokens[0].token); /* Add the label to the symbols list */
        if (symbolNames == NULL) {
            allocationError = 1;
            goto end;
        }
        strcpy(parsed_line->labelName, tokens[0].token);
        tokens = tokens->next;
    }
    /* According to https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3192253 a label
     for an empty line is defined as an error here*/
    if (tokens == NULL) {
        sprintf(parsed_line->error, "Unexpected end of line after label: %s", parsed_line->labelName);
        goto end;
    }

    /* These conditinals are used to determine the line's type */
    if (is_directive_keyword(tokens[0].token, TRUE)) {
        parsed_line->type = ENUM_DIRECTIVE;
        parsed_line->statement.directive.directiveType = get_directive_type(tokens[0].token, TRUE);
        tokens = tokens->next;
        parse_directive(tokens, parsed_line->statement.directive.directiveType, constantsTable, parsed_line);
    } else if (is_instruction_keyword(tokens[0].token)) {
        parsed_line->type = ENUM_INSTRUCTION;
        parsed_line->statement.instruction.opcode = get_opcode(tokens[0].token);
        tokens = tokens->next;
        parse_instruction(tokens, parsed_line->statement.instruction.opcode, constantsTable, parsed_line);
    } else if (is_const_defintion_keyword(tokens[0].token, TRUE)) {
        parsed_line->type = ENUM_CONSTANT_DEFINITION;
        tokens = tokens->next;
        parse_constant_defintion(tokens, constantsTable, parsed_line, symbol_table_head);
    } else {
        /* Can't have whitepsaces before comment sign https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3191487&p=7560784#p7560784*/
        if (tokens[0].token[0] == COMMENT) {
            sprintf(parsed_line->error, "Comments can't have whitespaces before ';'");
        } else {
            sprintf(parsed_line->error, "Unexpected token: %s", tokens[0].token);
        }
    }

    end:
    if (symbolNames != NULL) free_nodes(symbolNames);
    if (firstToken != NULL) free_nodes(firstToken);
    if (allocationError) {
        if (parsed_line != NULL) {
            free_parsed_syntax_line(parsed_line);
        }
        return NULL;
    }
    return parsed_line;
}

/* Function to parse a file and return an array of ParsedSyntaxLine structs consisting of the parsed data of the file as AST */
ParsedSyntaxLine** parse_file(const char* file_name, int* lineCount, Symbol_Node ** symbol_table_head, int* error) {
    FILE* file = NULL;
    char line[MAX_LINE_LENGTH + 2];
    /* Hashtable to store constants */
    hashtable* constantsTable = NULL;
    ParsedSyntaxLine** parsedLines = NULL;
    file = fopen(file_name, "r");
    *lineCount = 0;

    if (file == NULL) {
        fprintf(stderr, "File could not be opened.\n");
        return NULL;
    }

    /* Count the lines to correctly allocate memory for parsedLines */
    while (fgets(line, MAX_LINE_LENGTH + 2, file) != NULL) {
        (*lineCount)++;
    }

    rewind(file); 
    /* parsedLines = calloc((*lineCount), sizeof(ParsedSyntaxLine*)); */
    parsedLines = malloc((*lineCount) * sizeof(ParsedSyntaxLine*));
    if (parsedLines == NULL) {
        fprintf(stderr, "Failed to allocate memory for parsedLines\n");
        goto end;
    }

    *lineCount = 0;
    constantsTable = create_hashtable(); 
    if (constantsTable == NULL) {
        fprintf(stderr, "Failed to allocate memory for constantsTable\n");
        free(constantsTable);
        goto end;
    }

    while (fgets(line, MAX_LINE_LENGTH + 2, file) != NULL) {
        parsedLines[*lineCount] = parse_line(line, constantsTable, symbol_table_head);
        if (parsedLines[*lineCount] == NULL) {
            fprintf(stderr, "Memory allocation failed");
            goto end;
        }
        (*lineCount)++;
    }
    end:
    fclose(file);
    free_hashtable(constantsTable);
    return parsedLines;
}
