// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>

enum TokenType {
    TOKEN_INVALID = 0,
    TOKEN_EOF,              // returned at end of input
    TOKEN_INT_LITERAL,      // non-negative integer
    TOKEN_NEG_INT_LITERAL,  // negative integer
    TOKEN_STRING_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_LIST_START,       // '{'
    TOKEN_LIST_END,         // '{'
    TOKEN_ARRAY_START,      // '['
    TOKEN_ARRAY_END,        // ']'
    TOKEN_RANGE_START,      // '('
    TOKEN_RANGE_END,        // ')'
    TOKEN_CHILD_SEPARATOR,  // ','
    TOKEN_EQUALS,           // '='

    // type names
    TOKEN_INT8_TYPE,        // "int8"
    TOKEN_UINT8_TYPE,       // "uint8"
    TOKEN_INT16_TYPE,       // "int16"
    TOKEN_UINT16_TYPE,      // "uint16"
    TOKEN_INT32_TYPE,       // "int32"
    TOKEN_UINT32_TYPE,      // "uint32"
    TOKEN_INT64_TYPE,       // "int64"
    TOKEN_UINT64_TYPE,      // "uint64"
    TOKEN_BOOLEAN_TYPE,     // "boolean"
    TOKEN_STRING_TYPE,      // "string"
    TOKEN_ARRAY_TYPE,       // "array"
    TOKEN_RANGE32_TYPE,     // "range32"
    TOKEN_RANGE64_TYPE,     // "range64"
    TOKEN_LIST_TYPE,        // "list"

    // special values
    TOKEN_TRUE,             // "true"
    TOKEN_FALSE,            // "false"
};

TokenType find_reserved_word(std::string& string);
