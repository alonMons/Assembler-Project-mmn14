#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MEMORY_SIZE 4096
#define MAX_LINE_LENGTH 80
#define MAX_DATA_LENGTH 40 /* Because each number is seperated by comma it can't be more than 40*/
#define MAX_STRING_LENGTH 80
#define MAX_LABEL_LENGTH 31

#define MAX_TOKENS_PER_LINE 80
#define MAX_TOKEN_LENGTH 80
#define MAX_OPERANDS 2

#define ENCRYPTED_WORD_LENGTH 7
#define START_POSITION 100

#define MOV "mov"
#define CMP "cmp"
#define ADD "add"
#define SUB "sub"
#define NOT "not"
#define CLR "clr"
#define LEA "lea"
#define INC "inc"
#define DEC "dec"
#define JMP "jmp"
#define BNE "bne"
#define RED "red"
#define PRN "prn"
#define JSR "jsr"
#define RTS "rts"
#define HLT "hlt"
#define DATA ".data"
#define STRING ".string"
#define ENTRY ".entry"
#define EXTERN ".extern"
#define DEFINE ".define"
#define COMMENT ';'
#define MACRO_START "mcr"
#define MACRO_END "endmcr"

#endif