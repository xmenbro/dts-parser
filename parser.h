#ifndef PARSER_H
#define PARSER_H

#include "device_type.h"
#include <stdlib.h>

// Remove unwanted spaces
char* trim(char* str);
// Remove quotes from string
void strip_quotes(char* str);
// Exctract property from line
int extract_property(const char* line, const char* prop, char* value, size_t max_len);
// Parse gpio expression : <&gpio N FLAGS>
int parse_gpio(const char* line, char* controller, int* offset, int* flags);
// Extract gpio block number
int extract_gpio_block(const char* controller);
// Extract gpio_base
int extract_gpio_base(const char* line);

#endif
