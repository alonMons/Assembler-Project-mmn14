#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "utils.h"
#include "constants.h"
#include "writeOutputFiles.h"
#include "data_structures/node.h"

/* the write_output_files function creates the output files that describe the whole program */
void write_output_files (const char * filename, translation * output) {
  char * obName;
  char * entName;
  char * extName;
  FILE * obFile;
  FILE * entFile = NULL;
  FILE * extFile = NULL;
  int i, j;
  char * encrypted;
  Symbol_Node * current;
  External_Node * currentExt;

  obName = concatenate_strings(filename, ".ob");
  entName = concatenate_strings(filename, ".ent");
  extName = concatenate_strings(filename, ".ext");

  obFile = fopen(obName, "w");
  if (obFile) {
    printf("Creating .ob file for file \"%s\"\n", filename);
    fprintf(obFile, "%4d %d\n", output->IC - START_POSITION, output->DC); /* the tile of the file */
    for (i = START_POSITION; i < output->IC; i++) { /* for each word in the code image */
      encrypted = encrypt(output->code_image[i]);
      fprintf(obFile, "%04d %s\n", i, encrypted);
      free(encrypted);
    }
    for (i = output->IC; i < output->IC + output->DC; i++) { /* for each word in the data image */
      encrypted = encrypt(output->data_image[i - output->IC]);
      fprintf(obFile, "%04d %s\n", i, encrypted);
      free(encrypted);
    }
    for (current = output->symbol_table_head; current != NULL; current = current->next) {
      /* for each symbol in the symbol table */
      if (current->symbol->type == ENUM_SYMBOL_ENTRY_DATA ||
        current->symbol->type == ENUM_SYMBOL_ENTRY_CODE ||
        current->symbol->type == ENUM_SYMBOL_ENTRY_STRING) {
          /* if the symbol is an entry */
          if (entFile == NULL) {
            entFile = fopen(entName, "w");
            if (entFile) {
              printf("Creating .ent file for file \"%s\"\n", filename);
              /* write the name of the symbol, and it's address */
              fprintf(entFile, "%-10s\t%04d\n", current->symbol->name, current->symbol->address);
            } else {
              printf("Error creating .ent file for file \"%s\"\n", filename);
            }
          } else {
            /* write the name of the symbol, and it's address */
            fprintf(entFile, "%-10s\t%04d\n", current->symbol->name, current->symbol->address);
          }
        }
    }
    for (currentExt = output->external_table_head; currentExt != NULL; currentExt = currentExt->next) {
      /* for each external in the external table. 
        if there is no use of an external, the loop will be skipped */
      if (extFile == NULL) {
        extFile = fopen(extName, "w");
        if (extFile) {
          printf("Creating .ext file for file \"%s\"\n", filename);
          for (j = 0; j < currentExt->external->numOfUse; j++) {
            /* for each use of the external, write the name of the external,
                and the address it was used in */
            fprintf(extFile, "%-10s\t%04d\n", currentExt->external->name, currentExt->external->addresses[j]);
          }
        } else {
          printf("Error creating .ext file for file \"%s\"\n", filename);
        }
      } else {
        for (j = 0; j < currentExt->external->numOfUse; j++) {
          /* for each use of the external, write the name of the external,
              and the address it was used in */
          fprintf(extFile, "%-10s\t%04d\n", currentExt->external->name, currentExt->external->addresses[j]);
        }
      }
    }
    fclose(obFile);
    if (entFile) {
      fclose(entFile);
    }
    if (extFile) {
      fclose(extFile);
    }
  } else {
    fprintf(stderr, "Error creating .ob file for file \"%s\"\n", filename);
  }

  free(entName);
  free(extName);
  free(obName);
}

/* the encrypt function translates the binary word into an encrypted base-4 word */
char * encrypt (int code) {
  char * result = calloc(ENCRYPTED_WORD_LENGTH + 1, sizeof(char));
  int i, k;
  int two_bits;
  for (i = 0, k = 3 /* 3 = 11 */; i < ENCRYPTED_WORD_LENGTH; i++, k = 3) {
    /* for each char in the encryped word */
    k <<= (i * 2); /* the two bits that are needed */
    two_bits = code & k; /* get the two bits */
    two_bits >>= (i * 2); /* move the two bits to the right */
    if (two_bits == 0) {
      result[ENCRYPTED_WORD_LENGTH - i - 1] = '*';
    } else if (two_bits == 1) {
      result[ENCRYPTED_WORD_LENGTH - i - 1] = '#';
    } else if (two_bits == 2) {
      result[ENCRYPTED_WORD_LENGTH - i - 1] = '%';
    } else {
      result[ENCRYPTED_WORD_LENGTH - i - 1] = '!';
    }
  }
  result[ENCRYPTED_WORD_LENGTH] = '\0';
  return result;
}