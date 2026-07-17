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
    char temp[MAX_LINE];
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

// Parse gpio expression : <&gpio N FLAGS>
int parse_gpio(const char* line, char* controller, int* offset, int* flags) {
    // Copy line to buffer
    char buffer[MAX_LINE];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Find gpio
    char* gpio_ptr = strstr(buffer, "&gpio");
    if (!gpio_ptr)
        return 0;

    // Find space after gpio
    char* space_ptr = strchr(gpio_ptr, ' ');
    if (!space_ptr)
        return 0;

    // Calculate length of gpio and copy to controller
    size_t len = space_ptr - gpio_ptr;
    strncpy(controller, gpio_ptr, len);
    controller[len] = '\0';

    // Find offset
    char* offset_ptr = space_ptr;
    while (*offset_ptr && isspace(*offset_ptr))
        offset_ptr++;
    if (!*offset_ptr)
        return 0;
    *offset = atoi(offset_ptr);

    // Find flags
    char* flags_ptr = offset_ptr;
    // Skip offset e.g - 23
    while(*flags_ptr && !isspace(*flags_ptr))
        flags_ptr++;
    // Skip spaces
    while(*flags_ptr && isspace(*flags_ptr))
        flags_ptr++;
    if (!*flags_ptr)
        return 0;
    
    // Convert GPIO_ACTIVE_X to number
    char temp[32];
    char* bracket_ptr = strchr(flags_ptr, '>'); // Find end of this string
    if (!bracket_ptr)
        return 0;
    
    // Calculate length and copy to temp
    len = bracket_ptr - flags_ptr;
    strncpy(temp, flags_ptr, len);
    temp[len] = '\0';
    
    if (strcmp(temp, "GPIO_ACTIVE_LOW") == 0)
        *flags = 1;
    else
        *flags = 0;

    return 1;
}

// Extract gpio block number
int extract_gpio_block(const char* controller) {
    if (!controller || strlen(controller) <= 4)
        return -1;
    // Find number at the end of the string
    const char* ptr = controller + 4; // gpio = 4 chars, then number
    // Skip non-digit
    while(*ptr && !isdigit(*ptr))
        ptr++;

    if (*ptr)
        return atoi(ptr);

    return -1;
}

// Extract gpio_base
int extract_gpio_base(const char* line) {
    // Find '<'
    char* ptr = strchr(line, '<');
    if (!ptr)
        return 0;
    ptr++; // Skip '<'
    
    // Skip &pinctrl
    while(*ptr && !isdigit(*ptr))
        ptr++;
    if (!*ptr)
        return 0;

    return atoi(ptr);
}

// Parse dtsi file
int parse_dtsi_file(const char* path) {
    // Open file
    FILE* dtsi_file = fopen(path, "r");
    int gpio_base = -1;
    
    if (dtsi_file) {
        char line[MAX_LINE];
        // Read line and find gpio-ranges
        while(fgets(line, sizeof(line), dtsi_file)) {
            if (strstr(line, "gpio-ranges")) {
                gpio_base = extract_gpio_base(line);
                break;
            }
        }
        fclose(dtsi_file);
    }

    return gpio_base;
}
