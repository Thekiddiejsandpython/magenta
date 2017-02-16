// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <map>

#include "parser.h"

// map of identifier names to mdi_id_t
static std::map<std::string, mdi_id_t> id_map;

// map of ID numbers to identifier names
static std::map<uint64_t, std::string> id_name_map;

// map of child types for arrays
static std::map<mdi_id_t, mdi_type_t> array_type_map;

const char* get_id_name(mdi_id_t id) {
    return id_name_map[id & MDI_MAX_ID].c_str();
}

Node::Node(mdi_id_t id) {
    node.id = id;
}

void Node::print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
}

void Node::print_children(int depth) {
    for (auto iter = children.begin(); iter != children.end(); iter++) {
        iter->print(depth);
    }
}

void Node::print(int depth) {
    print_indent(depth);

    printf("%s = ", get_id_name(node.id));
    switch (MDI_ID_TYPE(node.id)) {      
        case MDI_INT8:
            printf("%d\n", node.value.i8);
            break;
        case MDI_UINT8:
            printf("%u\n", node.value.u8);
            break;
        case MDI_INT16:
            printf("%d\n", node.value.i16);
            break;
        case MDI_UINT16:
            printf("%u\n", node.value.u16);
            break;
        case MDI_INT32:
            printf("%d\n", node.value.i32);
            break;
        case MDI_UINT32:
            printf("%u\n", node.value.u32);
            break;
        case MDI_INT64:
            printf("%" PRId64 "\n", node.value.i64);
            break;
        case MDI_UINT64:
            printf("%" PRIu64 "\n", node.value.u64);
            break;
        case MDI_BOOLEAN:
            printf("%s\n", (node.value.b ? "true" : "false"));
            break;  
        case MDI_STRING:
            printf("%s\n", string_value.c_str());
            break;  
        case MDI_ARRAY:
            printf("[\n");
            print_children(depth + 1);
            print_indent(depth);
            printf("]\n");
            break;
        case MDI_LIST:
            printf("{\n");
            print_children(depth + 1);
            print_indent(depth);
            printf("}\n");
            break;
        case MDI_RANGE32:
            printf("MDI_RANGE32\n");
            break;
        case MDI_RANGE64:
            printf("MDI_RANGE64\n");
            break;
        case MDI_INVALID_TYPE:
            assert(0);
            break;
    }
}

int parse_id_declaration(Tokenizer& tokenizer, mdi_type_t type) {
    mdi_type_t child_type = MDI_INVALID_TYPE;

    if (type == MDI_ARRAY) {
        // array declarations are followed by child type
        Token token;
        if (!tokenizer.next_token(token)) {
            return -1;
        }
        if (token.type == TOKEN_EOF) {
            fprintf(stderr, "end of file while parsing ID declaration\n");
            return -1;
        }
        if (token.type != TOKEN_ARRAY_START) {
            fprintf(stderr, "expected \'[' after \"array\"\n");
            return -1;
        }
        if (!tokenizer.next_token(token)) {
            return -1;
        }
        if (token.type == TOKEN_EOF) {
            fprintf(stderr, "end of file while parsing ID declaration\n");
            return -1;
        }

        child_type = token.get_type_name();
        if (child_type == MDI_INVALID_TYPE) {
            fprintf(stderr, "bad array child type \"%s\"\n", token.string_value.c_str());
        }

        if (!tokenizer.next_token(token)) {
            return -1;
        }
        if (token.type == TOKEN_EOF) {
            fprintf(stderr, "end of file while parsing ID declaration\n");
            return -1;
        }
        if (token.type != TOKEN_ARRAY_END) {
            fprintf(stderr, "expected \'[' after array child type\n");
            return -1;
        }
    }

    Token name_token, number_token;

    if (!tokenizer.next_token(name_token)) {
        return -1;
    }
    if (name_token.type == TOKEN_EOF) {
        fprintf(stderr, "end of file while parsing ID declaration\n");
        return -1;
    }

    if (!tokenizer.next_token(number_token)) {
        return -1;
    }
    if (number_token.type == TOKEN_EOF) {
        fprintf(stderr, "end of file while parsing ID declaration\n");
        return -1;
    }

    if (name_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "expected identifier, got token \"%s\" in ID declaration\n",
                name_token.string_value.c_str());
        return -1;
    }
    const char* name = name_token.string_value.c_str();

    if (number_token.type != TOKEN_INT_LITERAL) {
        fprintf(stderr, "expected integer, got token \"%s\" in ID declaration for \"%s\"\n",
                number_token.string_value.c_str(), name);
        return -1;
    }

    if (id_map.find(name) != id_map.end()) {
        fprintf(stderr, "Duplicate declaration for ID %s\n", name);
        return -1;
    }

    if (number_token.int_value < 1 || number_token.int_value > MDI_MAX_ID) {
        fprintf(stderr, "ID number %" PRId64 " for ID %s out of range\n", number_token.int_value,
                name);
    }
    uint64_t id_number = number_token.int_value;  
    auto duplicate = id_name_map.find(id_number);
    if (duplicate != id_name_map.end()) {
        fprintf(stderr, "ID number %" PRId64 " has already been assigned to ID %s\n",
                id_number, duplicate->second.c_str());
        return -1;
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
    return 0;
}

static int parse_int_node(Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_INT_LITERAL && token.type != TOKEN_NEG_INT_LITERAL) {
        fprintf(stderr, "Expected integer value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }

    Node node(id);
    mdi_type_t type = MDI_ID_TYPE(id);

    if (type == MDI_UINT64) {
        node.node.value.u64 = token.int_value;
    } else if (type == MDI_INT64) {
        uint64_t value = token.int_value;
        if (value > INT64_MAX || -value < INT64_MIN) goto out_of_range;
        if (token.type == TOKEN_NEG_INT_LITERAL) {
            node.node.value.i64 = -value;
        } else {
            node.node.value.i64 = value;
        }
    } else {
        // signed version of our value
        int64_t value = (int64_t)token.int_value;
        if (token.type == TOKEN_NEG_INT_LITERAL) {
            value = -value;
        }

        switch (type) {
            case MDI_INT8:
                if (value > INT8_MAX || value < INT8_MIN) goto out_of_range;
                node.node.value.i8 = (int8_t)value;
                break;
            case MDI_UINT8:
                if (value > UINT8_MAX || value < 0) goto out_of_range;
                node.node.value.u8 = (uint8_t)value;
                break;
            case MDI_INT16:
                if (value > INT16_MAX || value < INT16_MIN) goto out_of_range;
                node.node.value.i16 = (int16_t)value;
                break;
            case MDI_UINT16:
                if (value > UINT16_MAX || value < 0) goto out_of_range;
                node.node.value.u16 = (uint16_t)value;
                break;
            case MDI_INT32:
                if (value > INT32_MAX || value < INT32_MIN) goto out_of_range;
                node.node.value.i32 = (int32_t)value;
                break;
            case MDI_UINT32:
                if (value > UINT32_MAX || value < 0) goto out_of_range;
                node.node.value.u32 = (uint32_t)value;
                break;
            default:
                assert(0);
                return -1;
        }
    }

    parent.children.push_back(node);
    return 0;

out_of_range:
    fprintf(stderr, "Integer value %s%" PRId64 " out of range for \"%s\"\n",
            (token.type == TOKEN_NEG_INT_LITERAL ? "-" : ""),
            token.int_value, get_id_name(id));
    return -1;
}

static int parse_string_node(Token& token, mdi_id_t id, Node& parent) {
     if (token.type != TOKEN_STRING_LITERAL) {
        fprintf(stderr, "Expected string value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }

    Node node(id);
    node.string_value = token.string_value;
    parent.children.push_back(node);

    return 0;
}

static int parse_boolean_node(Token& token, mdi_id_t id, Node& parent) {
    Node node(id);

    if (token.type == TOKEN_TRUE) {
        node.node.value.b = true;
    } else if (token.type == TOKEN_TRUE) {
        node.node.value.b = false;
    } else {
        fprintf(stderr, "Expected boolean value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }

    parent.children.push_back(node);
    return 0;
}

static int parse_list_node(Tokenizer& tokenizer, Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_LIST_START) {
        fprintf(stderr, "Expected list value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }
     
    Node node(id);

    while (1) {
        Token token;
        if (!tokenizer.next_token(token)) {
            return -1;
        }
        if (token.type == TOKEN_EOF) {
            fprintf(stderr, "end of file while parsing list children\n");
            return -1;
        } else if (token.type == TOKEN_LIST_END) {
            break;
        }

        if (parse_node(tokenizer, token, node)) {
            return -1;
        }
    }

    parent.children.push_back(node);
    return 0;
}

static int parse_array_node(Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_ARRAY_START) {
        fprintf(stderr, "Expected array value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }

    return 0;
}

static int parse_range_node(Token& token, mdi_id_t id, Node& parent) {
    if (token.type != TOKEN_RANGE_START) {
        fprintf(stderr, "Expected range value for node \"%s\", got \"%s\"\n", get_id_name(id),
                token.string_value.c_str());
        return -1;
    }

    return 0;
}

int parse_node(Tokenizer& tokenizer, Token& token, Node& parent) {
    auto iter = id_map.find(token.string_value);
    if (iter == id_map.end()) {
        fprintf(stderr, "undefined identifier \"%s\"\n", token.string_value.c_str());
        return -1;
    }
    mdi_id_t id = iter->second;

    Token equals_token;
    if (!tokenizer.next_token(equals_token)) {
        return -1;
    }
    if (equals_token.type != TOKEN_EQUALS) {
        fprintf(stderr, "Expected \'=\' after identifier %s\n", token.string_value.c_str());
        return -1;
    }

    Token value;
    if (!tokenizer.next_token(value)) {
        return -1;
    }
    if (value.type == TOKEN_EOF) {
        fprintf(stderr, "end of file while parsing node\n");
        return -1;
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
            return parse_int_node(value, id, parent);
        case MDI_BOOLEAN:
            return parse_boolean_node(value, id, parent);
        case MDI_STRING:
            return parse_string_node(value, id, parent);
        case MDI_ARRAY:
            return parse_array_node(value, id, parent);
        case MDI_RANGE32:
        case MDI_RANGE64:
            return parse_range_node(value, id, parent);
        default:
            fprintf(stderr, "Internal error: Unknown type %d\n", MDI_ID_TYPE(id));
            return -1;
    }
}
