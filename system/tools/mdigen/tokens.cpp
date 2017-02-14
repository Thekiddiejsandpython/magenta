// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "tokens.h"

struct ReservedWord {
    TokenType   token;
    const char* word;
} reserved_words [] = {
    { TOKEN_INT8_TYPE,      "int8" },
    { TOKEN_UINT8_TYPE,     "uint8" },
    { TOKEN_INT16_TYPE,     "int16" },
    { TOKEN_UINT16_TYPE,    "uint16" },
    { TOKEN_INT32_TYPE,     "int32" },
    { TOKEN_UINT32_TYPE,    "uint32" },
    { TOKEN_INT64_TYPE,     "int64" },
    { TOKEN_UINT64_TYPE,    "uint64" },
    { TOKEN_BOOLEAN_TYPE,   "boolean" },
    { TOKEN_STRING_TYPE,    "string" },
    { TOKEN_ARRAY_TYPE,     "array" },
    { TOKEN_RANGE32_TYPE,   "range32" },
    { TOKEN_RANGE64_TYPE,   "range64" },
    { TOKEN_LIST_TYPE,      "list" },
    { TOKEN_TRUE,           "true" },
    { TOKEN_FALSE,          "false" },
    { TOKEN_INVALID,        nullptr },
};

TokenType find_reserved_word(std::string& string) {
    const char* str = string.c_str();
    ReservedWord* test = reserved_words;

    while (test->word) {
        if (!strcmp(str, test->word)) {
            return test->token;
        }
        test++;
    }
    return TOKEN_IDENTIFIER;
}
