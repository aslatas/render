#ifndef CONFIG_UTILS_H

// Header file that config parsers can include for convience
// Might not end up even using this

typedef enum {
  CONFIG_PARSE_SUCCESSFUL,
  CONFIG_PARSE_FILE_NOT_FOUND,
  CONFIG_PARSE_INVALID_INPUT
} EParseReturn;

#define CONFIG_UTILS_H
#endif