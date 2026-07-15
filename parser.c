#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

// Remove unwanted spaces
char* trim(char* str) {
    // Skip space chars at the beginning of a string
    while (isspace((unsigned char)*str))
        str++;
    // Is the string empty now?
    if (*str == 0)
        return str;

    // Skip space chars at the end of a string
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';
    return str;
}

// Remove quotes from string
void strip_quotes(char* str) {
    char* ptr = str; // Current pointer
    char* result = str; // Result pointer
    
    while (*ptr) {
        // If ptr doesn't contain these chars then add to result pointer
        if (*ptr != '"' && *ptr != '\'' && *ptr != ';' && *ptr != ',')
            *result++ = *ptr;
        ptr++; // Step to the next char
    }
    *result = '\0';
}
