#include "parser.h"
#include <stdio.h>
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

// Exctract property from line
int extract_property(const char* line, const char* prop, char* value, size_t max_len) {
    // Copy line to temp buf
    char temp[1024];
    strncpy(temp, line, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // This line contains needed property
    char* ptr = strstr(temp, prop);
    if (!ptr) // return false if not
        return 0;
    
    // Trying to find =
    ptr = strchr(ptr, '=');
    if (!ptr)
        return 0;
    ptr++; // Skip '='

    // Skip spaces
    while (*ptr && isspace(*ptr))
        ptr++;
    if (!*ptr)
        return 0;

    // Copy value of this property
    char* end = ptr;
    while (*end && *end != ';' && *end != ' ' && *end != '\t' && *end != '\n')
        end++;
    
    // Calculate length of value
    size_t len = end - ptr;
    if (len >= max_len)
        len = max_len - 1;
    
    // Copy to value
    strncpy(value, ptr, len);
    value[len] = '\0';
    strip_quotes(value);

    return 1;
}
