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
