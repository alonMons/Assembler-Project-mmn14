#ifndef STRUCTS_FILE_H
#define STRUCTS_FILE_H

#include "constants.h"
#include "data_structures/node.h"

#define ENUM_INVALID -1

typedef enum {
    PREPROCESS_SUCCESS, /* Preprocessing succeeded */
    PREPROCESS_WARNING, /* Preprocessing found issues but allows continuation */
    PREPROCESS_FAIL /* Critical issues found, halting further processing */
} PreprocessStatus; /* The return value after the preprocessing stage */

typedef struct {
    char* name;
    int value;
} ConstantDefintionStatement;

typedef enum { 
    ENUM_CONSTANT_DEFINITION,
    ENUM_INSTRUCTION,
    ENUM_DIRECTIVE,
    ENUM_COMMENT,
    ENUM_EMPTY
} InstructionType;

typedef enum {
    ENUM_DATA,
    ENUM_STRING,
    ENUM_ENTRY,
    ENUM_EXTERN
} DirectiveType;


typedef enum {
    OPERAND_TYPE_IMMEDIATE = 1 << 0,   /* 0001 */ 
    OPERAND_TYPE_DIRECT = 1 << 1, /* 0010 */
    OPERAND_TYPE_INDEXED = 1 << 2, /* 0100 */
    OPERAND_TYPE_REGISTER = 1 << 3  /* 1000 */
} OperandType;

/* A structure that holds information about an operand; it's type and value */
typedef struct {
    OperandType operandType;
    union {
        int immediate;
        char* directLabel;
        struct {
            char* label;
            int value;
        } constantIndex;
        int directRegisterNum;
    } operandValue;
} Operand;

typedef enum { 
    ENUM_MOV = 0, 
    ENUM_CMP = 1, 
    ENUM_ADD = 2, 
    ENUM_SUB = 3, 
    ENUM_NOT = 4, 
    ENUM_CLR = 5, 
    ENUM_LEA = 6, 
    ENUM_INC = 7, 
    ENUM_DEC = 8, 
    ENUM_JMP = 9, 
    ENUM_BNE = 10, 
    ENUM_RED = 11, 
    ENUM_PRN = 12, 
    ENUM_JSR = 13, 
    ENUM_RTS = 14, 
    ENUM_HLT = 15 
} Opcode;

/* A structure that holds information about an instruction statement; it's opcode and operands */
typedef struct {
    Opcode opcode;
    int numOfOperands;
    Operand operands[2];
} InstructionStatement;

/* A structure that holds information about a directive statement; it's type and value */
typedef struct {
    DirectiveType directiveType;
    union {
        struct {
            int* values;
            int count;
        } data;
        char* string;
        char* entryLabel;
        char* externLabel;
    } directiveValue;
} DirectiveStatement;

/* A structure that represents a single parsed line of the input file. It contains all the information needed about a line. */
typedef struct ParsedSyntaxLine { 
    char error[250];
    char labelName[MAX_LABEL_LENGTH + 1];
    InstructionType type;
    union {
        ConstantDefintionStatement constantDefinition;
        InstructionStatement instruction;
        DirectiveStatement directive;
    } statement;
} ParsedSyntaxLine;

typedef enum {
    FALSE = 0,
    TRUE = 1
} boolean;


/* A structure that represents a rule for an instruction's operands */
typedef struct {
    const char* name;
    int allowedSourceTypes;
    int allowedDestinationTypes;
    int numberOfOperandsRequired;
} InstructionRule;

/* A structure that represents a symbol in the symbol table */
typedef struct Symbol {
    char name [MAX_LABEL_LENGTH + 1];
    enum {
        ENUM_SYMBOL_ENTRY,
        ENUM_SYMBOL_EXTERN,
        ENUM_SYMBOL_CODE,
        ENUM_SYMBOL_DATA,
        ENUM_SYMBOL_STRING,
        ENUM_SYMBOL_ENTRY_DATA,
        ENUM_SYMBOL_ENTRY_CODE,
        ENUM_SYMBOL_ENTRY_STRING,
        ENUM_SYMBOL_CONSTANT_MACRO
    } type;
    int dataLength; /* for symbols that are data or list */
    int address;
} Symbol;

/* A structure that represents an external symbol in the externals table */
typedef struct External {
    char * name;
    int addresses [MEMORY_SIZE];
    int numOfUse;
} External;

/* A structure that we build after going through the program.
 it represents all of the aspects of a program, and we use it to build the final output files */
typedef struct translation {
   int code_image [MEMORY_SIZE];
   int data_image [MEMORY_SIZE];
   int IC;
   int DC;
   struct Symbol_Node * symbol_table_head;
   struct External_Node * external_table_head;
} translation;


#endif