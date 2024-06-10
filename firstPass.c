#include <stdio.h>
#include <string.h>
#include "data_structures/node.h"
#include "structs.h"
#include "constants.h"
#include "data_structures/node.h" 
#include "firstPass.h"
#include "secondPass.h"
#include <stdio.h>
#include "globals.h"

/* the firstPass goes through the parsed lines for the first time and creates the symbol table */
int firstPass (const char* fileName, translation * translation, ParsedSyntaxLine **lines,int lineCount) {
  int IC = START_POSITION, DC = 0;
  int error = 0; /* a flag to indicate if there's an error */
  int i;
  Symbol_Node * found;
  Symbol_Node * current;
  ParsedSyntaxLine line;
  int operandNum = 0;
  for (i = 0; i < lineCount; i++) {
    line = *(lines[i]);
    if (*line.error != '\0') {
      fprintf(stderr, "Error in file \"%s\" on line %d: %s\n", fileName, i + 1, line.error);
      error = 1;
      continue;
    } if (*line.labelName != '\0' && (line.type == ENUM_INSTRUCTION ||
     (line.type == ENUM_DIRECTIVE && 
     (line.statement.directive.directiveType == ENUM_DATA || line.statement.directive.directiveType == ENUM_STRING)))) {
        /* if there's a label that declares of data or string */
        found = symbol_contains(translation->symbol_table_head, line.labelName);
        if (found) { /* if the symbol is already in the symbol table */
          if (found->symbol->type == ENUM_SYMBOL_ENTRY) {
            /* if the current type of the symbol is entry, 
            it means that it was already declared, and now it is initialized */
            found->symbol->type = line.type == ENUM_INSTRUCTION ? ENUM_SYMBOL_ENTRY_CODE : (
              line.statement.directive.directiveType == ENUM_DATA ? ENUM_SYMBOL_ENTRY_DATA : ENUM_SYMBOL_ENTRY_STRING
            ) ;
            found->symbol->address = line.type == ENUM_INSTRUCTION ? IC : DC;

            /* keep track of the length of the of the string/data so that we can identify an out of bounds index */
            if (line.type == ENUM_DIRECTIVE && line.statement.directive.directiveType == ENUM_DATA) {
              found->symbol->dataLength = line.statement.directive.directiveValue.data.count;
            } else if (line.type == ENUM_DIRECTIVE && line.statement.directive.directiveType == ENUM_STRING) {
              found->symbol->dataLength = strlen(line.statement.directive.directiveValue.string) + 1;
            }
          }
          else { 
            /* the symbol is present in the symbol table and it is not a
             entry which means that it was already initialized */
            fprintf(stderr, "Error in file \"%s\" on line %d: Trying to redefine the symbol: \"%s\"\n", fileName, i + 1, found->symbol->name);
            error = 1;
          }
        }
        else { /* the symbol is not present in the sumbol table, which means that it needs to be added */
          current = insert_symbol(&translation->symbol_table_head, line.labelName);
          current->symbol->type = line.type == ENUM_INSTRUCTION ? ENUM_SYMBOL_CODE : 
          (line.statement.directive.directiveType == ENUM_DATA ? ENUM_SYMBOL_DATA : ENUM_SYMBOL_STRING);
          current->symbol->address = line.type == ENUM_INSTRUCTION ? IC : DC; 

          /* keep track of the length of the of the string/data so that we can identify an out of bounds index */
          if (line.type == ENUM_DIRECTIVE && line.statement.directive.directiveType == ENUM_DATA) {
            current->symbol->dataLength = line.statement.directive.directiveValue.data.count;
          }
          else if (line.type == ENUM_DIRECTIVE && line.statement.directive.directiveType == ENUM_STRING) {
            current->symbol->dataLength = strlen(line.statement.directive.directiveValue.string) + 1;
          }
        }
    }
    /* if the line is an instruction we should increase the IC so that the addresses of the symbols are correct */
    if (line.type == ENUM_INSTRUCTION) {
      error |= checkNumOfOperands(fileName, i + 1, line.statement.instruction.numOfOperands, line.statement.instruction.opcode);
      IC++; /* the first word which describes the instruction itself */
      if (line.statement.instruction.operands[0].operandType == OPERAND_TYPE_REGISTER &&
      line.statement.instruction.operands[1].operandType == OPERAND_TYPE_REGISTER) {
        /* if both operands are registers, then only one additional word is needed to store them */
        IC++;
      }
      else {
        for (operandNum = 0; operandNum < line.statement.instruction.numOfOperands; operandNum++) {
          if (line.statement.instruction.operands[operandNum].operandType == OPERAND_TYPE_IMMEDIATE) {
            /* if the operand is a number, then only one additional word is needed to store the number itself */
            IC++;
          } else if (line.statement.instruction.operands[operandNum].operandType == OPERAND_TYPE_DIRECT) {
            /* if the operand is a label, then only one additional word is needed to store the address of the label */
            IC++;
          } else if (line.statement.instruction.operands[operandNum].operandType == OPERAND_TYPE_INDEXED) {
            /* if the operand is a indexed label, then two additional words are 
            needed to store the adress of the label, and the index itself */
            IC += 2;
          } else IC++; /* if the operand is a register, then only one additional word is needed to store the register number */
        }
      }
    } else if (line.type == ENUM_CONSTANT_DEFINITION && isNumTooLarge(line.statement.constantDefinition.value, 14)) {
      /* if the constant can't fit in 14 bits it has no use */
      printf("Warning in file \"%s\" on line %d: the constant \"%s\" is too %s and not useable\n", fileName, i +1, line.statement.constantDefinition.name, line.statement.constantDefinition.value > 0 ? "large" : "small");
    } else if (line.type == ENUM_DIRECTIVE && (line.statement.directive.directiveType == ENUM_DATA || line.statement.directive.directiveType == ENUM_STRING)) {
      /* if the line is a directive, then we should increase the DC so that the addresses of the symbols are correct */
      if (line.statement.directive.directiveType == ENUM_DATA) {
        /* the number of words needed to store the data. each word stores one number */
        DC += line.statement.directive.directiveValue.data.count; 
      } else {
        /* the number of words needed to store the string. each word stores one character, 
          and we need an additional one to store the \0 */
        DC += strlen(line.statement.directive.directiveValue.string) + 1;
      }
    } else if (line.type == ENUM_DIRECTIVE && (line.statement.directive.directiveType == ENUM_ENTRY || line.statement.directive.directiveType == ENUM_EXTERN)) {
      /* if the line declares a synbol as entry, or as external */
      handle_symbol_definition(fileName, i + 1, translation, line, &error);
    }
  }
  for (current = translation->symbol_table_head; current != NULL; current = current->next) {
    if (current->symbol->type == ENUM_SYMBOL_ENTRY) {
      /* no symbol can remain as entry, it needs to be initialized */
      fprintf(stderr, "Error in file \"%s\": Symbol \"%s\" was declared as entry but was never defined\n", fileName, current->symbol->name);
      error = 1;
    } if (current->symbol->type == ENUM_SYMBOL_ENTRY_DATA || current->symbol->type == ENUM_SYMBOL_DATA 
    || current->symbol->type == ENUM_SYMBOL_STRING || current->symbol->type == ENUM_SYMBOL_ENTRY_STRING) {
      /* currently, the address is storing the dc. so, we need to increase it 
      by the IC so that the data goes after the code in the output */
      current->symbol->address += IC;
    }
  }
  if (IC + DC > MEMORY_SIZE) {
    /* if the final size of the program exceeds the memory size of the computer */
    printf("Error in file \"%s\": Program is too large\n", fileName);
    error = 1;
  }
  return error;
}

/* the handle_symbol_definition function, handles the decleration of a entry/external symbol */
void handle_symbol_definition(const char* fileName, int lineNumber, translation* translation, ParsedSyntaxLine line, int* error) {
    Symbol_Node* found = symbol_contains(translation->symbol_table_head, line.statement.directive.directiveValue.entryLabel);
    Symbol_Node* current;

    if (found) {
      /* if the symbol is present in the symbol table */

      /* if the user is decalring a symbol entry/extenal twice, a warning should be issued */
      if (line.statement.directive.directiveType == ENUM_EXTERN && found->symbol->type == ENUM_SYMBOL_EXTERN) {
        printf("Warning in file \"%s\" on line %d: Redefining the symbol \"%s\" as extern again\n", fileName, lineNumber, found->symbol->name);
      } else if (line.statement.directive.directiveType == ENUM_ENTRY && found->symbol->type == ENUM_SYMBOL_ENTRY) {
        printf("Warning in file \"%s\" on line %d: Redefining the symbol \"%s\" as entry again\n", fileName, lineNumber, found->symbol->name);
      } 
      
      /* update the type of the symbol so that it is a entry symbol  */
      else if (line.statement.directive.directiveType == ENUM_ENTRY) {
        switch (found->symbol->type) {
          case ENUM_SYMBOL_CODE:
            found->symbol->type = ENUM_SYMBOL_ENTRY_CODE;
            break;
          case ENUM_SYMBOL_DATA:
            found->symbol->type = ENUM_SYMBOL_ENTRY_DATA;
            break;
          case ENUM_SYMBOL_STRING:
            found->symbol->type = ENUM_SYMBOL_ENTRY_STRING;
            break;
          default: /* if the type of the symbol is external */
            fprintf(stderr, "Error in file \"%s\" on line %d: Trying to redefine the symbol \"%s\"\n", fileName, lineNumber, found->symbol->name);
            *error = 1;
            break;
        }
      } else { /* if the symbol is currently entry, and the line is declaring it as external */
        fprintf(stderr, "Error in file \"%s\" on line %d: Trying to redefine the symbol \"%s\"\n", fileName, lineNumber, found->symbol->name);
        *error = 1;
      }
    } else { /* if the symbol isn't present in the symbol table it needs to be added to it */
        if (line.statement.directive.directiveType == ENUM_ENTRY) { /* if the symbol is entry */
            current = insert_symbol(&translation->symbol_table_head, line.statement.directive.directiveValue.entryLabel);
            current->symbol->type = ENUM_SYMBOL_ENTRY;
        } else { /* if the symbol is external */
            current = insert_symbol(&translation->symbol_table_head, line.statement.directive.directiveValue.externLabel);
            current->symbol->type = ENUM_SYMBOL_EXTERN;
        }
    }
}

/* the checkNumOfOperands function checks if the number of operands given to an instruction is correct */
int checkNumOfOperands(const char * fileName,int lineNum, int numOfOperands, Opcode instruction) {
  int required = instructionRules[instruction].numberOfOperandsRequired;
  if (numOfOperands != instructionRules[instruction].numberOfOperandsRequired) {
    fprintf(stderr, "Error in file \"%s\" on line %d: The instruction \"%s\" requires %d %s, but %d %s given\n", fileName, lineNum, instructionRules[instruction].name, required, required > 1 ? "operands" : "operand" , numOfOperands, numOfOperands > 1 ? "were" : "was");
    return 1;
  } else return 0;
}