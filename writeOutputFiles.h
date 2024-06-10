#include "structs.h"

#ifndef WRITE_OUTPUT_FILES_H
#define WRITE_OUTPUT_FILES_H

/* the write_output_files function creates the output files that describe the whole program */
void write_output_files (const char * filename, translation * output);

/* the encrypt function translates the binary word into an encrypted base-4 word */
char * encrypt (int code);

#endif