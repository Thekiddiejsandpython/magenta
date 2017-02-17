// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <ctime>
#include <map>

#include "node.h"
#include "parser.h"

//#define PRINT_ID_DECLARATIONS 1

static bool parse_node(Tokenizer& tokenizer, Token& token, Node& parent);

// map of identifier names to mdi_id_t
static std::map<std::string, mdi_id_t> id_map;

// map of ID numbers to identifier names
static std::map<uint32_t, std::string> id_name_map;

// map of child types for arrays
static std::map<mdi_id_t, mdi_type_t> array_type_map;

const char* get_id_name(mdi_id_t id) {
    return id_name_map[id & MDI_MAX_ID].c_str();
}

static bool parse_id_declaration(Tokenizer& tokenizer, mdi_type_t type) {
    mdi_type_t child_type = MDI_INVALID_TYPE;

    if (type == MDI_ARRAY) {
        // array declarations are followed by child type
        Token token;
        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            tokenizer.print_err("end of file while parsing ID declaration\n");
            return false;
        }
        if (token.type != TOKEN_ARRAY_START) {
            tokenizer.print_err("expected \'[' after \"array\"\n");
            return false;
        }
        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            tokenizer.print_err("end of file while parsing ID declaration\n");
            return false;
        }

        child_type = token.get_type_name();
        switch (child_type) {
            case MDI_INT8:
            case MDI_UINT8:
            case MDI_INT16:
            case MDI_UINT16:
            case MDI_INT32:
            case MDI_UINT32:
            case MDI_INT64:
            case MDI_UINT64:
            case MDI_BOOLEAN:
                // these are OK
                break;
            default:
                tokenizer.print_err("unsupported array child type \"%s\". "
                                    "Only integer and boolean types are currently supported\n",
                                    token.string_value.c_str());
                return false;
        }

        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            tokenizer.print_err("end of file while parsing ID declaration\n");
            return false;
        }
        if (token.type != TOKEN_ARRAY_END) {
            tokenizer.print_err("expected \'[' after array child type\n");
            return false;
        }
    }

    Token name_token, number_token;

    if (!tokenizer.next_token(name_token)) {
        return false;
    }
    if (name_token.type == TOKEN_EOF) {
        tokenizer.print_err("end of file while parsing ID declaration\n");
        return false;
    }

    if (!tokenizer.next_token(number_token)) {
        return false;
    }
    if (number_token.type == TOKEN_EOF) {
        tokenizer.print_err("end of file while parsing ID declaration\n");
        return false;
    }

    if (name_token.type != TOKEN_IDENTIFIER) {
        tokenizer.print_err("expected identifier, got token \"%s\" in ID declaration\n",
                            name_token.string_value.c_str());
        return false;
    }
    const char* name = name_token.string_value.c_str();

    if (number_token.type != TOKEN_INT_LITERAL) {
        tokenizer.print_err("expected integer, got token \"%s\" in ID declaration for \"%s\"\n",
                            number_token.string_value.c_str(), name);
        return false;
    }

    if (id_map.find(name) != id_map.end()) {
        tokenizer.print_err("duplicate declaration for ID %s\n", name);
        return false;
    }

    if (number_token.int_value < 1 || number_token.int_value > MDI_MAX_ID) {
        tokenizer.print_err("ID number %" PRId64 " for ID %s out of range\n",
                            number_token.int_value, name);
    }
    uint64_t id_number = number_token.int_value;  
    auto duplicate = id_name_map.find(id_number);
    if (duplicate != id_name_map.end()) {
        tokenizer.print_err("ID number %" PRId64 " has already been assigned to ID %s\n",
                            id_number, duplicate->second.c_str());
        return false;
    }

    mdi_id_t id = MDI_ID(type, id_number);
    id_map[name] = id;
    id_name_map[id_number] = name;

    if (type == MDI_ARRAY) {
        array_type_map[id] = child_type;
    }

#if PRINT_ID_DECLARATIONS
    printf("ID %s : %08X\n", name, id);
#endif
    return true;
}

static bool parse_include(Tokenizer& tokenizer, Node& root) {
    Token token;

    if (!tokenizer.next_token(token)) {
        return false;
    }
    if (token.type == TOKEN_EOF) {
        tokenizer.print_err("end of file while parsing ID declaration\n");
        return false;
    }
    if (token.type != TOKEN_STRING_LITERAL) {
        tokenizer.print_err("expected string file path after include, got \"%s\"\n",
                token.string_value.c_str());
        return false;
    }

    return process_file(&tokenizer, token.string_value.c_str(), root);
}

static bool parse_int_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_INT_LITERAL && token.type != TOKEN_NEG_INT_LITERAL) {
        tokenizer.print_err("expected integer value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return false;
    }

    Node node(id);
    mdi_type_t type = MDI_ID_TYPE(id);

    // signed version of our value
    int64_t value = (int64_t)token.int_value;

    switch (type) {
        case MDI_INT8:
            if (value > INT8_MAX || value < INT8_MIN) goto out_of_range;
            break;
        case MDI_UINT8:
            if (value > UINT8_MAX || value < 0) goto out_of_range;
            break;
        case MDI_INT16:
            if (value > INT16_MAX || value < INT16_MIN) goto out_of_range;
            break;
        case MDI_UINT16:
            if (value > UINT16_MAX || value < 0) goto out_of_range;
            break;
        case MDI_INT32:
            if (value > INT32_MAX || value < INT32_MIN) goto out_of_range;
            break;
        case MDI_UINT32:
            if (value > UINT32_MAX || value < 0) goto out_of_range;
            break;
        case MDI_INT64:
            if (value > INT64_MAX || -value < INT64_MIN) goto out_of_range;
        case MDI_UINT64:
        break;
        default:
            assert(0);
            return false;
    }    
    node.int_value = token.int_value;

    parent.children.push_back(node);
    return true;

out_of_range:
    tokenizer.print_err("integer value %s%" PRId64 " out of range for \"%s\"\n",
                        (token.type == TOKEN_NEG_INT_LITERAL ? "-" : ""),
                        token.int_value, get_id_name(id));
    return false;
}

static bool parse_string_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
     if (token.type != TOKEN_STRING_LITERAL) {
        tokenizer.print_err("expected string value for node \"%s\", got \"%s\"\n", get_id_name(id),
                            token.string_value.c_str());
        return false;
    }

    Node node(id);
    node.string_value = token.string_value;
    parent.children.push_back(node);

    return true;
}

static bool parse_boolean_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
    Node node(id);

    if (token.type == TOKEN_TRUE) {
        node.bool_value = true;
    } else if (token.type == TOKEN_FALSE) {
        node.bool_value = false;
    } else {
        tokenizer.print_err("expected boolean value for node \"%s\", got \"%s\"\n", get_id_name(id),
                            token.string_value.c_str());
        return false;
    }

    parent.children.push_back(node);
    return true;
}

static bool parse_list_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_LIST_START) {
        tokenizer.print_err("expected list value for node \"%s\", got \"%s\"\n", get_id_name(id),
                            token.string_value.c_str());
        return false;
    }
     
    Node node(id);

    while (1) {
        Token token;
        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            tokenizer.print_err("end of file while parsing list children\n");
            return false;
        } else if (token.type == TOKEN_LIST_END) {
            break;
        }

        if (!parse_node(tokenizer, token, node)) {
            return false;
        }
    }

    parent.children.push_back(node);
    return true;
}

static bool parse_array_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_ARRAY_START) {
        tokenizer.print_err("expected array value for node \"%s\", got \"%s\"\n", get_id_name(id),
                            token.string_value.c_str());
        return false;
    }
    mdi_type_t element_type = array_type_map[id];
    mdi_id_t element_id = MDI_ID(element_type, 0);

    Node node(id);

    while (1) {
        Token token;
        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            tokenizer.print_err("end of file while parsing list children\n");
            return false;
        } else if (token.type == TOKEN_ARRAY_END) {
            break;
        }

        switch (element_type) {
            case MDI_INT8:
            case MDI_UINT8:
            case MDI_INT16:
            case MDI_UINT16:
            case MDI_INT32:
            case MDI_UINT32:
            case MDI_INT64:
            case MDI_UINT64:
                if (!parse_int_node(tokenizer, token, element_id, node)) {
                    return false;
                }
                break;
            case MDI_BOOLEAN:
                if (!parse_boolean_node(tokenizer, token, element_id, node)) {
                    return false;
                }
                break;
            default:
                assert(0);
                break;
        }
    }

    parent.children.push_back(node);
    return true;
}

static bool parse_node(Tokenizer& tokenizer, Token& token, Node& parent) {
    auto iter = id_map.find(token.string_value);
    if (iter == id_map.end()) {
        tokenizer.print_err("undefined identifier \"%s\"\n", token.string_value.c_str());
        return false;
    }
    mdi_id_t id = iter->second;

    Token equals_token;
    if (!tokenizer.next_token(equals_token)) {
        return false;
    }
    if (equals_token.type != TOKEN_EQUALS) {
        tokenizer.print_err("expected \'=\' after identifier %s\n", token.string_value.c_str());
        return false;
    }

    Token value;
    if (!tokenizer.next_token(value)) {
        return false;
    }
    if (value.type == TOKEN_EOF) {
        tokenizer.print_err("end of file while parsing node\n");
        return false;
    }

    switch (MDI_ID_TYPE(id)) {
        case MDI_LIST:
            return parse_list_node(tokenizer, value, id, parent);
        case MDI_INT8:
        case MDI_UINT8:
        case MDI_INT16:
        case MDI_UINT16:
        case MDI_INT32:
        case MDI_UINT32:
        case MDI_INT64:
        case MDI_UINT64:
            return parse_int_node(tokenizer, value, id, parent);
        case MDI_BOOLEAN:
            return parse_boolean_node(tokenizer, value, id, parent);
        case MDI_STRING:
            return parse_string_node(tokenizer, value, id, parent);
        case MDI_ARRAY:
            return parse_array_node(tokenizer, value, id, parent);
        default:
            tokenizer.print_err("internal error: Unknown type %d\n", MDI_ID_TYPE(id));
            return false;
    }
}

bool process_file(Tokenizer* container, const char* in_path, Node& root) {
    Tokenizer tokenizer;
    if (!tokenizer.open_file(container, in_path)) {
        return false;
    }

    while (1) {
        Token token;

        if (!tokenizer.next_token(token)) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            // on to the next input file
            break;
        }

        mdi_type_t type = token.get_type_name();
        if (type != MDI_INVALID_TYPE) {
            // handle ID declarations
            if (!parse_id_declaration(tokenizer, type)) {
                return false;
            }
        } else if (token.type == TOKEN_INCLUDE) {
            if (!parse_include(tokenizer, root)) {
                return false;
            }
        } else if (token.type == TOKEN_IDENTIFIER) {
            if (!parse_node(tokenizer, token, root)) {
                return false;
            }
        } else {
            tokenizer.print_err("unexpected token \"%s\" at top level\n",
                                token.string_value.c_str());
            return false;
        }
    }

    return true;
}

constexpr char kAuthors[] = "The Fuchsia Authors";

bool generate_file_header(std::ofstream& os) {
    auto t = std::time(nullptr);
    auto ltime = std::localtime(&t);

    os << "// Copyright " << ltime->tm_year + 1900
       << " " << kAuthors << ". All rights reserved.\n";
    os << "// This is a GENERATED file. The license governing this file can be ";
    os << "found in the LICENSE file.\n\n";
    return os.good();
}

bool print_header_file(std::ofstream& os) {
    generate_file_header(os);
    for (auto iter = id_map.begin(); iter != id_map.end(); iter++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "#define %-50s 0x%08X\n", iter->first.c_str(), iter->second);
        os << buffer;
    }

    return true;
}
