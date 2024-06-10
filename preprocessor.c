#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "data_structures/hashtable.h"
#include "data_structures/node.h"
#include "constants.h"
#include "globals.h"
#include "utils.h"
#include "parser.h"
#include "data_structures/node.h"

/* Function to check if a macro name is valid. Requirements for a valid macro name:
    - The first character must be a latin letter.
    - It is of length 1 to 31 (including).
    - It does not contain any whitespace characters.
    - It is not a reserved keyword.
    - It contains only printable characters.
 */
boolean is_macro_name_valid(const char* macroName) {
    int i;
    if (strlen(macroName) > MAX_LABEL_LENGTH) {
        return FALSE;
    }
    if (!isalpha(macroName[0])) {
        return FALSE;
    }
    for (i = 1; i < strlen(macroName); i++) {
        if (isspace(macroName[i]) || !isprint(macroName[i])) {
            return FALSE;
        }
    }

    return !is_keyword(macroName, FALSE); /* false because we don't check for : in a macro name*/
}

/**
 * create_preprocessed_file - Processes an assembly source file to expand macros and prepare it for assembly.
 * The filename is the base name of the file to process, i.e. excluding an extension.
 * The symbol_table_head is a double pointer to the head of a linked list for symbol management.
 * 
 * This function reads an assembly file (.as), expands macros defined within it, and writes the result to a new file (.am).
 * It updates the symbol table with macro definitions.
 * It returns a PreprocessStatus indicating the success of the preprocessing, or a warning/error status if issues are encountered.
 */
PreprocessStatus create_preprocessed_file(const char* filename, Symbol_Node ** symbol_table_head, char* origialFileName, char* newFileName) {
    Symbol_Node * symbolJ = NULL;
    node* tokens = NULL, *firstToken = NULL;
    char *currentMacroName = NULL, *currentMacroCode = NULL,
     *tempMacroName = NULL, currentChar = 0,
    *newMacroCode, *macroCode;
    char line[MAX_LINE_LENGTH + 2];
    char error[250];
    int inMacro, lineTooLong, allocationError = 0;
    int lineNumber = 0;
    FILE *originalFp = NULL, *newFp = NULL;
    hashtable* macros = NULL;

    error[0] = '\0';
    originalFp = fopen(origialFileName, "r");
    newFp = fopen(newFileName, "w+");

    if (originalFp == NULL || newFp == NULL) {
        sprintf(error, "File %s could not be opened.\n", (originalFp == NULL) ? origialFileName : newFileName);
        goto end;
    }

    macros = create_hashtable();
    if (macros == NULL) {
        sprintf(error, "Failed to allocate memory for macros hashtable\n");
        goto end;
    }
    /* Flag to check if currently reading a macro definition */
    inMacro = 0;
    /* Flag to check if a line over the MAX_LINE_LENGTH have been found in the file */
    lineTooLong = 0;
    
    while (fgets(line, MAX_LINE_LENGTH + 2, originalFp) != NULL) {
        lineNumber++;
        /* Check if line is too long */
        if (strstr(line, "\n") == NULL && !feof(originalFp)) {
            lineTooLong = 1;
            fprintf(stderr, "Error in file \"%s\", line %d is too long, maximum length is %d\n", 
            origialFileName, lineNumber, MAX_LINE_LENGTH);
            do {
                currentChar = fgetc(originalFp);
            } while (currentChar != '\n' && currentChar != EOF);
            line[MAX_LINE_LENGTH] = '\n'; /* Clear the rest of the line */
        }

        /* Tokenize the line */
        tokens = tokenize_line(line, &allocationError);
        if (allocationError) {
            sprintf(error, "Error in file \"%s\", line %d: Failed to allocate memory\n", origialFileName, lineNumber);
            goto end;
        }
        firstToken = tokens; /* Keep track of the first node to free it later*/

        if (firstToken == NULL) {
            continue;
        }

        /* Check if line is start of a macro defintion */
        if (strcmp(firstToken->token, MACRO_START) == 0) {
            if (firstToken->next == NULL) {
                sprintf(error, "Error in file \"%s\", line %d: Macro name is missing\n", origialFileName, lineNumber);
                break;
            }

            if (!is_macro_name_valid(firstToken->next->token)) {
                sprintf(error, "Error in file \"%s\", line %d: Invalid macro name %s\n", origialFileName, lineNumber, firstToken->next->token);
                break;
            }

            if (firstToken->next->next != NULL) {
                sprintf(error, "Error in file \"%s\", line %d: Unexpected token %s after macro name\n", origialFileName, lineNumber, firstToken->next->next->token);
                break;
            }
            tempMacroName = firstToken->next->token;
            symbolJ = insert_symbol(symbol_table_head, tempMacroName); /* Insert the new macro to the symbols table */
            if (symbolJ == NULL) {
                sprintf(error, "Error: Failed to allocate memory for macro symbol\n");
                allocationError = 1;
                goto end;
            }
            symbolJ->symbol->type = ENUM_SYMBOL_CONSTANT_MACRO;
            inMacro = 1;
            insert(macros, tempMacroName, "", 1); /* empty string because the macro hasn't been capturedm */
            if (macros == NULL) {
                sprintf(error, "Error: Failed to allocate memory for macro hashtable\n");
                allocationError = 1;
                goto end;
            }
            if (currentMacroName != NULL) {
                free(currentMacroName);
            }
            currentMacroName = duplicate_string(tempMacroName);
            if (currentMacroName == NULL) {
                sprintf(error, "Error: Failed to allocate memory for macro name\n");
                allocationError = 1;
                goto end;
            }
        }
        /* Check if line is end of a macro definition */
        else if (strcmp(tokens->token, MACRO_END) == 0 && inMacro) {
            if (tokens->next != NULL) {
                sprintf(error, "Error in file \"%s\", line %d: Unexpected token %s after macro end: %s\n", origialFileName, lineNumber, tokens->next->token, MACRO_END);
                break;
            }
            inMacro = 0;
        }
        /* If in macro, append line to macro's code */
        else if (inMacro) {
            currentMacroCode = (char*)search(macros, currentMacroName);
            newMacroCode = concatenate_strings(currentMacroCode, line);
            if (newMacroCode == NULL) {
                sprintf(error, "Error: Failed to allocate memory for macro\n");
                allocationError = 1;
                goto end;
            }
            /* Update the macro defintion with the new code */
            replace(macros, currentMacroName, newMacroCode, strlen(newMacroCode) + 1, &allocationError);
            if (allocationError) {
                sprintf(error, "Error: Failed to allocate memory for macro\n");
                goto end;
            }
            free(newMacroCode);
        /* If not in macro, then the line is a normal line */
        } else {
            if (firstToken != NULL) {
                tempMacroName = firstToken->token;
            } else {
                tempMacroName = NULL;
            }
            /* If a macro is called here, seach it */
            macroCode = (char*)search(macros, tempMacroName);
            if (macroCode != NULL) {
                fputs(macroCode, newFp);
            } else {
                fputs(line, newFp);
            }
        }

        if (firstToken != NULL) {
            free_nodes(firstToken);
            firstToken = NULL;
        }
    }


    end:
    /* Free the last tokens if not freed by the loop (early break)*/
    if (firstToken != NULL) {
        free_nodes(firstToken);
        firstToken = NULL;
    }

    if (originalFp != NULL)
        fclose(originalFp);
    if (newFp != NULL)
        fclose(newFp); /* Ensure the file is closed before trying to remove it */
    if (error[0] != '\0' || allocationError) { 
        if (error[0] != '\0') 
            fprintf(stderr, "%s", error);
        if (remove(newFileName) != 0) {
            perror("Error deleting file");
        }
    }
    /* Free variables */
    if (currentMacroName != NULL) {
        free(currentMacroName);
    }
    if (macros != NULL) {
        free_hashtable(macros);
    }

    if (error[0] != '\0' || allocationError) return PREPROCESS_FAIL;
    if (lineTooLong) return PREPROCESS_WARNING;
    return PREPROCESS_SUCCESS;

}