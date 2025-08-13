#pragma once

#include <cstddef> // For offsetof, size_t

// Forward declaration of the options struct to avoid circular dependencies
struct sysOptions;

// Enum to identify the data type of an option
enum OptionType {
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_UINT16,
    TYPE_INT16,
    TYPE_STRING,
};

// Struct to describe each option in sysOptions
struct OptionDescriptor {
    const char* name;     // The name of the option (as in the CSV file)
    OptionType type;      // The data type of the option
    size_t offset;        // The memory offset within the sysOptions struct
    size_t size;          // The size of the member (only for strings/arrays)
};

// The descriptor table that maps CSV names to struct members (defined in the .cpp)
extern const OptionDescriptor optionDescriptors[];
extern const size_t numOptionDescriptors;