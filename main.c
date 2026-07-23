#include "parser.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    const char* dtsi_path = NULL;
    
    // Parse args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc)
                output_file = argv[++i];
            else {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
        } 
        else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc)
                dtsi_path = argv[++i]; 
            else {
                fprintf(stderr, "Error: -i requires an argument\n");
                return 1;
            }
        } 
        else if (input_file == NULL)
            input_file = argv[i]; 
        else {
            fprintf(stderr, "Error: Unexpected argument: %s\n", argv[i]);
            return 1;
        }
    }
    
    if (input_file == NULL) {
        fprintf(stderr, "Usage: %s [-i dtsi_path] [-o output_file] input_file\n", argv[0]);
        return 1;
    }
    
    struct gpio_table table = parse_dts_file(input_file, dtsi_path);
    print_table(&table, output_file);
    
    return 0;
}
