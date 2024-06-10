#include <stdio.h>
#include "parser.h"
#include "utils.h"
#include "assemble_file.h"

/**
 * The main function of the assembler program. 
 * It reads a list of files from args and assembles those files.
*/
int main(int argc, char **argv) {
    int i;
    
    if (argc <= 1) {
        fprintf(stderr, "No files specified, exiting program.\n");
        return 1;
    }
    
    for (i = 1; i < argc; i++) {
        assemble_file(argv[i]);
    }

    return 0;
}