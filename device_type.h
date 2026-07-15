#ifndef DEVICE_TYPE_H
#define DEVICE_TYPE_H

// Constants
#define MAX_NAME 64
#define MAX_DEVICES 256

// GPIO type in this case it can be led or button
enum gpio_type {
    TYPE_LED,
    TYPE_BUTTON
};

// This structure represents needed information about gpio
struct gpio_entry {
    char name[MAX_NAME]; // Node's name in DTS
    char label[MAX_NAME]; // Label from DTS
    char controller[MAX_NAME]; // Contoller name ("gpio1")
    int gpio_block; // GPIO block number (1,2...)
    int gpio_offset; // Offset inside block
    int gpio_index; // gpio_block*32 + gpio_offset
    int active_low; // If active_low = 1 then ACTIVE_LOW else ACTIVE_HIGH
    gpio_type type; // Type - LED or button
    char state[8]; // Text representation of active_low field
};

// Array of gpio_entry
struct gpio_table {
    gpio_entry entries[MAX_DEVICES];
    int count;
};

#endif
