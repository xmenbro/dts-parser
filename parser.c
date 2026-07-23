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
    if (len >= 32)
        return 0;
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

// Parse dts file
struct gpio_table parse_dts_file(const char* dts_path, const char* dtsi_path) {
    // Trying to parse dtsi file
    int gpio_base = -1;
    if (dtsi_path)
        gpio_base = parse_dtsi_file(dtsi_path);

    // Open dts file
    struct gpio_table table = {0};
    FILE* dts_file = fopen(dts_path, "r");
    if (!dts_file) {
        fprintf(stderr, "ERROR: Can't open dts file '%s' : %s\n", dts_path, strerror(errno));
        return table;
    }
    
    // Create variables
    char line[MAX_LINE]; // Current file line
    char parent_node[MAX_NAME] = ""; // Name of parent node
    char current_label[MAX_NAME] = ""; // Label
    char current_node[MAX_NAME] = ""; // Name of current node
    int in_leds = 0; // If we're in gpio-leds then 1
    int in_keys = 0; // If we're in gpio-keys then 1
    int in_gpio_node = 0; // If we're in gpio section then 1
    int brace_level = 0; // Current node depth
    int parent_brace_level = 0; // Parent node depth
 
    // Read dts file
    while (fgets(line, sizeof(line), dts_file)) {
        char* trimmed = trim(line); // Remove spaces
        
        // Skip empty lines and comments
        if (trimmed[0] == '\0' || trimmed[0] == '/') 
            continue;
        if (trimmed[0] == '&')
            continue;
        
        // Check {
        if (strstr(trimmed, "{")) {
            char node_name[MAX_NAME] = "";
            sscanf(trimmed, "%63s", node_name);
            
            // Remove '{' from name
            char* brace = strchr(node_name, '{');
            if (brace) *brace = '\0';
            
            // Check parent sections
            if (strcmp(node_name, "gpio-keys") == 0 || strstr(node_name, "keys")) {
                in_keys = 1;
                in_leds = 0;
                strcpy(parent_node, "gpio-keys");
                current_label[0] = '\0';
                parent_brace_level = brace_level + 1;
            } 
            else if (strcmp(node_name, "leds") == 0 || strstr(node_name, "gpio-leds")) {
                in_leds = 1;
                in_keys = 0;
                strcpy(parent_node, "leds");
                current_label[0] = '\0';
                parent_brace_level = brace_level + 1;
            } 
            else if (in_keys || in_leds) {
                // It's a child node
                in_gpio_node = 1;
                strcpy(current_node, node_name);
                strncpy(current_label, node_name, MAX_NAME - 1);
            }
            
            brace_level++;
            continue;
        }
        
        // Check }
        if (strstr(trimmed, "}")) {
            brace_level--;
            
            // If child node is closing
            if (in_gpio_node && brace_level < parent_brace_level) {
                in_gpio_node = 0;
                current_label[0] = '\0';
                current_node[0] = '\0';
            }
            
            // If parent section is closing
            if (brace_level < parent_brace_level && (in_leds || in_keys)) {
                in_leds = 0;
                in_keys = 0;
                parent_node[0] = '\0';
                current_label[0] = '\0';
                current_node[0] = '\0';
                in_gpio_node = 0;
                parent_brace_level = 0;
            }
            continue;
        }
        
        // If we found label
        if (in_gpio_node && strstr(trimmed, "label =")) {
            char label[MAX_NAME] = "";
            if (extract_property(trimmed, "label", label, sizeof(label)))
                strncpy(current_label, label, MAX_NAME - 1);
            continue;
        }
        
        // If we found gpios
        if (in_gpio_node && strstr(trimmed, "gpios = <")) {
            struct gpio_entry entry;
            memset(&entry, 0, sizeof(entry));
            
            // Use saved label
            if (current_label[0] != '\0')
                strncpy(entry.label, current_label, MAX_NAME - 1); 
            else
                strncpy(entry.label, current_node, MAX_NAME - 1);
            
            // Parse gpios
            int offset, flags;
            char controller[MAX_NAME];
            if (parse_gpio(trimmed, controller, &offset, &flags)) {
                entry.gpio_offset = offset;
                entry.active_low = flags;
                
                // If we red dtsi file
                if (gpio_base != -1) {
                    entry.gpio_index = gpio_base + entry.gpio_offset;
                    entry.gpio_block = entry.gpio_index / 32;
                }
                // If we didn't read, then try to extract from controller name
                else {
                    entry.gpio_block = extract_gpio_block(controller);
                    entry.gpio_index = entry.gpio_block * 32 + entry.gpio_offset;
                }

                // Define state
                if (flags == 1)
                    strcpy(entry.state, "low"); 
                else 
                    strcpy(entry.state, "high");
                
                // Define type
                if (in_keys)
                    entry.type = TYPE_BUTTON; 
                else if (in_leds)
                    entry.type = TYPE_LED;
                
                // Copy controller
                strncpy(entry.controller, controller, MAX_NAME - 1);
                
                // Add entry to table
                if (table.count < MAX_DEVICES) {
                    table.entries[table.count] = entry;
                    table.count++;
                }
            }
        }
    }
    
    fclose(dts_file);
    return table;
}

// Print table
void print_table(const struct gpio_table* table, const char* output_file) {
    // Define the output, default = stdout
    FILE* out = stdout;
    if (output_file) {
        out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot create output file '%s'\n", output_file);
            return;
        }
    }

    fprintf(out, "-----------------------------------------------------------------------\n");
    
    // Print buttons
    int has_buttons = 0;
    for (int i = 0; i < table->count; i++) {
        if (table->entries[i].type == TYPE_BUTTON) {
            if (!has_buttons) {
                fprintf(out, "button:\n");
                has_buttons = 1;
            }
            fprintf(out, "%d %s %s\n", table->entries[i].gpio_index, table->entries[i].label, table->entries[i].state);
        }
    }
    
    if (!has_buttons) {
        fprintf(out, "button:\n");
        fprintf(out, "(no buttons found)\n");
    }
    
    fprintf(out, "\n");
    
    // Print LED
    int has_leds = 0;
    for (int i = 0; i < table->count; i++) {
        if (table->entries[i].type == TYPE_LED) {
            if (!has_leds) {
                fprintf(out, "led:\n");
                has_leds = 1;
            }
            fprintf(out, "%d %s %s\n", table->entries[i].gpio_index, table->entries[i].label, table->entries[i].state);
        }
    }
    
    if (!has_leds) {
        fprintf(out, "led:\n");
        fprintf(out, "(no leds found)\n");
    }
    
    fprintf(out, "-----------------------------------------------------------------------\n");
    
    if (output_file) {
        printf("Table exported to '%s'\n", output_file);
        fclose(out);
    }
}
