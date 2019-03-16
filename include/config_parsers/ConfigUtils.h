#pragma once

// Header file that config parsers can include for convience
// Might not end up even using this

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;

typedef enum {
  CONFIG_PARSE_SUCCESSFUL,
  CONFIG_PARSE_FILE_NOT_FOUND,
  CONFIG_PARSE_INVALID_INPUT
} EParseReturn;