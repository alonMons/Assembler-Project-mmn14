#include <stdio.h>
#include "structs.h"
#include "secondPass.h"
#include <string.h>
#include "data_structures/node.h"

/* the secondPass function's purpose is to build the translation of the program
 as binary, and ready it for file creation */
int secondPass(ParsedSyntaxLine **lines, translation *output, int lineCount, char * filename) {
    int i, j, k;
    int error = 0;
    int value;
    int index; 
    ParsedSyntaxLine line;
    Symbol_Node * found;
    for (i = 0; i < lineCount; i++) {
        line = *(lines[i]);
        if (line.error[0] != '\0') { /* if the line has an error, it should be skipped */
            continue;
        }
        if (line.type == ENUM_INSTRUCTION) {
          output->code_image[output->IC] = line.statement.instruction.opcode << 6; /* insert the opcode of the function */
          if (line.statement.instruction.numOfOperands == 1) {
            /* if there's only on operand, it is the destination, and not the source  */
            output->code_image[output->IC] |= operandType(line.statement.instruction.operands[0].operandType) << 2;
          }
          else if (line.statement.instruction.numOfOperands == 2) {
            /* if there are two operands, the first is the source, and the second is the destination */
            output->code_image[output->IC] |= operandType(line.statement.instruction.operands[0].operandType) << 4;
            output->code_image[output->IC] |= operandType(line.statement.instruction.operands[1].operandType) << 2;
          }
          output->IC++; /* the first word, that describes the instruction itself is built */
          if (line.statement.instruction.numOfOperands == 2 && 
          line.statement.instruction.operands[0].operandType == OPERAND_TYPE_REGISTER && 
          line.statement.instruction.operands[1].operandType == OPERAND_TYPE_REGISTER) {
            /* if both operands are registers, then only one additional word is needed to store them */
            output->code_image[output->IC] |= line.statement.instruction.operands[0].operandValue.directRegisterNum << 5;
            output->code_image[output->IC] |= line.statement.instruction.operands[1].operandValue.directRegisterNum << 2;
            output->IC++;
          } else {
            for (j = 0; j < line.statement.instruction.numOfOperands; j++) { /* for each operand */
                if (line.statement.instruction.operands[j].operandType == OPERAND_TYPE_IMMEDIATE) {
                    /* if the operand is a number */
                    value = line.statement.instruction.operands[j].operandValue.immediate; 
                    if (isNumTooLarge(value, 12)) {
                        /* if the number can't fit in 12 bits */
                        fprintf(stderr, "Error in file \"%s\" on line %d: value \"%d\" is too %s\n", filename, i + 1, value, value < 0 ? "small" : "large");
                        error = 1;
                    } else output->code_image[output->IC] |= value << 2;
                    value = 0;
                } else if (line.statement.instruction.operands[j].operandType == OPERAND_TYPE_DIRECT) {
                     /* if the operand is a label */
                    error = directAddress(output, line, i, j, filename, line.statement.instruction.operands[j].operandValue.directLabel) ? error : 1;
                } else if (line.statement.instruction.operands[j].operandType == OPERAND_TYPE_INDEXED) {
                    /* if the operand is a indexed label */
                   if((found = directAddress(output, line, i, j, filename, line.statement.instruction.operands[j].operandValue.constantIndex.label))) {
                    /* if the label is a real symbol */
                    if (found->symbol->type == ENUM_SYMBOL_EXTERN || found->symbol->type == ENUM_SYMBOL_DATA || found->symbol->type == ENUM_SYMBOL_ENTRY_DATA ||
                    found->symbol->type == ENUM_SYMBOL_STRING || found->symbol->type == ENUM_SYMBOL_ENTRY_STRING) {
                        /* if the type of the symbol is string, data, or external */
                        index = line.statement.instruction.operands[j].operandValue.constantIndex.value;
                        if (found->symbol->type == ENUM_SYMBOL_EXTERN || index < found->symbol->dataLength) {
                            /* if the index is in bounds of the symbol's data, or if it's an external and can't be checked */
                            output->IC++;
                            output->code_image[output->IC] |= index << 2;
                        } else {
                            /* if the index is out of bounds of the symbol's data */
                            fprintf(stderr, "Error in file \"%s\" on line %d: Index %d is out of bounds\n", filename, i + 1, index);
                            error = 1;
                        }
                    } else {
                        /* if the type of the symbol isn't string, data, or external it can't be indexed */
                        fprintf(stderr, "Error in file \"%s\" on line %d: Symbol is not indexable\n", filename, i + 1);
                        error = 1;
                    }
                   } else {
                    /* if the symbol doesn't exist */
                    error = 1;
                   }
                } else { /* if the operand is a register */
                    output->code_image[output->IC] |= line.statement.instruction.operands[j].operandValue.directRegisterNum << (j == 1 || line.statement.instruction.numOfOperands == 1 ? 2 : 5);
                }
                output->IC++; /* the additional word is built */
            }
          }
        } else if (line.type == ENUM_DIRECTIVE && (line.statement.directive.directiveType == ENUM_DATA || line.statement.directive.directiveType == ENUM_STRING)) {
            /* if the line is a decleration of a string, or an array of data */
            if (line.statement.directive.directiveType == ENUM_DATA) {
                /* if the line is declaring an array of data */
                for (k = 0; k < line.statement.directive.directiveValue.data.count; k++) { 
                    /* for each value in the array */
                    value = line.statement.directive.directiveValue.data.values[k];
                    if (isNumTooLarge(value, 14)) {
                        /* if the value can't fit in 14 bits */
                        fprintf(stderr, "Error in file \"%s\" on line %d: value \"%d\" is too %s\n", filename, i + 1, value, value < 0 ? "small" : "large");
                        error = 1;
                    } else {
                        output->data_image[output->DC] = value;
                        output->DC++;

                    }
                }
            } else { /* if the line is declaring a string */
                for (k = 0; k < strlen(line.statement.directive.directiveValue.string); k++) { 
                    /* for each character in the string */
                    output->data_image[output->DC] = (int)line.statement.directive.directiveValue.string[k];
                    output->DC++;
                }
                output->DC++; /* one additional word is needed to store the \0 */
            }
        }
    }
    return error;
}

/* the operandType function mathes the type of the operand (which is defined in structs.h) 
    to the num of the type */
int operandType (int originalOperandType) {
    if (originalOperandType == 1 << 0) return 0;
    else if (originalOperandType == 1 << 1) return 1;
    else if (originalOperandType == 1 << 2) return 2;
    else if (originalOperandType == 1 << 3) return 3;
    return 0;
}

/* the directAddress function creates the word that deribes the adress of the symbol, 
    and returns a pointer to the symbol */
Symbol_Node * directAddress (translation *output, ParsedSyntaxLine line, int i,int j, char * filename, char * label) {
    Symbol_Node * found;
    found = symbol_contains(output->symbol_table_head, label);
    if (found) {
        /* if the label is a real symbol */
        if (found->symbol->type == ENUM_SYMBOL_EXTERN) {
            output->code_image[output->IC] |= 1;
            /* add the use of the external to the externals table */
            output->external_table_head = insert_external(output->external_table_head, label, output->IC);
        } else {
            output->code_image[output->IC] |= found->symbol->address << 2;
            output->code_image[output->IC] |= 2;
        }
        return found;
    } else {
        /* if the symbol doesn't exist */
        fprintf(stderr, "Error in file \"%s\" on line %d: Symbol \"%s\" not found\n", filename, i, label);
        return NULL;
    }
}

/* the isNumTooLarge function checks if a number can be represented by the given bits, in the two's complement method */
int isNumTooLarge(int num, int bits) {
    int max = (1 << (bits - 1)) - 1;
    int min = -(1 << (bits - 1));
    
    if (num > max || num < min) {
        return 1; /* Number is too large to be represented in the given bits */
    }
    return 0; /* Number can be represented in the given bits */
}
