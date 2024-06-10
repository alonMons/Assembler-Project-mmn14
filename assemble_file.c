#include <stdio.h>
#include <stdlib.h>
#include "preprocessor.h"
#include "parser.h"
#include "globals.h"
#include "firstPass.h"
#include "constants.h"
#include "utils.h"
#include "secondPass.h"
#include "writeOutputFiles.h"

/* the assemble_file function, recives the name of the file to assemble from the assenbler.
    then it goes through all the steps to create and assemble the output of the file  */
void assemble_file(const char* filename) {
    ParsedSyntaxLine **lines = NULL;
    translation *output = NULL;
    int lineCount = 0;
    char *amName = NULL, *asName = NULL;
    int i;
    int error = 0, allocationError = 0; /* a flag to indicate if there's an error */

    /* Initialize output values */
    output = malloc(sizeof(translation));
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory for translation struct\n");
        goto end;
    }
    output->symbol_table_head = NULL;
    output->external_table_head = NULL;
    output->IC = START_POSITION;
    output->DC = 0;
    
    for (i = 0; i < MEMORY_SIZE; i++) {
        output->code_image[i] = 0;
        output->data_image[i] = 0;
    }

    amName = concatenate_strings(filename, ".am");

    if (amName == NULL) {
        fprintf(stderr, "Failed to allocate memory for am file name\n");
        goto end;
    }

    asName = concatenate_strings(filename, ".as");

    if (asName == NULL) {
        fprintf(stderr, "Failed to allocate memory for as file name\n");
        goto end;
    }

    printf("Processing file \"%s\"\n", asName);
    printf("Creating .am file for file \"%s\"\n", asName);
    /* Perform preprocessing */
    error = create_preprocessed_file(filename, &output->symbol_table_head, asName, amName);
    /* If the preprocessing failed, end the assemble process because an am file can't be created'*/
    if (error == PREPROCESS_FAIL) {
        goto end;
    }

    printf("Parsing file \"%s\"\n", amName);
    /* Parse the file into a linked list of parsed lines */
    lines = parse_file(amName, &lineCount, &output->symbol_table_head, &allocationError);

    if (allocationError) {
        goto end;
    }

    error |= firstPass(amName, output, lines, lineCount);

    /* we go into the secondPass phase even if there's an error, so that we can find additional errors */
    error |= secondPass(lines, output, lineCount, amName);

    if (error == 0) {
        /* only create the output files if there is no error */
        write_output_files(filename, output);
    }

    end:
    printf("Finished assembling file \"%s\" with %s\n\n", filename, error ? "errors" : "success");
    
    /* free all assigned memory */
    i = 0;
    while (i < lineCount) {
        if (lines[i] != NULL)
            free_parsed_syntax_line(lines[i]);
        i++;
    }
    if (amName != NULL) free(amName);
    if (asName != NULL) free(asName);
    if (lines != NULL) free(lines);
    if (output != NULL && output->external_table_head != NULL)
        free_externals(output->external_table_head);
    if (output != NULL && output->symbol_table_head != NULL) 
        free_symbols(output->symbol_table_head);
    if (output != NULL)
        free(output);
}