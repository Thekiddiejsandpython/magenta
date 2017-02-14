// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>
#include <fstream>

#include <magenta/mdi.h>

#include "tokens.h"

//#define PRINT_TOKENS            1
//#define PRINT_ID_DECLARATIONS   1
#define PRINT_PARSE_TREE        1

// map of identifier names to mdi_id_t
static std::map<std::string, mdi_id_t> id_map;

// map of ID numbers to identifier names
static std::map<uint64_t, std::string> id_name_map;

// map of child types for arrays
static std::map<mdi_id_t, mdi_type_t> array_type_map;

static const char* get_id_name(mdi_id_t id) {
    return id_name_map[id & MDI_MAX_ID].c_str();
}

struct Token {
    TokenType   type;
    uint64_t    int_value;
    std::string string_value;   // raw string value

    // returns type for type name tokens
    mdi_type_t get_type_name() {
        switch (type) {
            case TOKEN_INT8_TYPE:
                return MDI_INT8;
            case TOKEN_UINT8_TYPE:
                return MDI_UINT8;
            case TOKEN_INT16_TYPE:
                return MDI_INT16;
            case TOKEN_UINT16_TYPE:
                return MDI_UINT16;
            case TOKEN_INT32_TYPE:
                return MDI_INT32;
            case TOKEN_UINT32_TYPE:
                return MDI_UINT32;
            case TOKEN_INT64_TYPE:
                return MDI_INT64;
            case TOKEN_UINT64_TYPE:
                return MDI_UINT64;
            case TOKEN_BOOLEAN_TYPE:
                return MDI_BOOLEAN;
            case TOKEN_STRING_TYPE:
                return MDI_STRING;
            case TOKEN_ARRAY_TYPE:
                return MDI_ARRAY;
            case TOKEN_RANGE32_TYPE:
                return MDI_RANGE32;
            case TOKEN_RANGE64_TYPE:
                return MDI_RANGE64;
            case TOKEN_LIST_TYPE:
                return MDI_LIST;
            default:
                return MDI_INVALID_TYPE;
        }
    }

    void print() {
        switch (type) {
            case TOKEN_INVALID:
                printf("TOKEN_INVALID\n");
                break;
            case TOKEN_EOF:
                printf("TOKEN_EOF\n");
                break;
            case TOKEN_INT_LITERAL:
                printf("TOKEN_INT_LITERAL %" PRId64 "\n", int_value);
                break;
            case TOKEN_NEG_INT_LITERAL:
                printf("TOKEN_NEG_INT_LITERAL %" PRId64 "\n", int_value);
                break;
            case TOKEN_STRING_LITERAL:
                printf("TOKEN_STRING_LITERAL %s\n", string_value.c_str());
                break;
            case TOKEN_IDENTIFIER:
                printf("TOKEN_IDENTIFIER %s\n", string_value.c_str());
                break;
            case TOKEN_LIST_START:
                printf("TOKEN_LIST_START\n");
                break;
            case TOKEN_LIST_END:
                printf("TOKEN_LIST_END\n");
                break;
            case TOKEN_ARRAY_START:
                printf("TOKEN_ARRAY_START\n");
                break;
            case TOKEN_ARRAY_END:
                printf("TOKEN_ARRAY_END\n");
                break;
            case TOKEN_RANGE_START:
                printf("TOKEN_RANGE_START\n");
                break;
            case TOKEN_RANGE_END:
                printf("TOKEN_RANGE_END\n");
                break;
            case TOKEN_EQUALS:
                printf("TOKEN_EQUALS\n");
                break;
            case TOKEN_INT8_TYPE:
                printf("TOKEN_INT8_TYPE\n");
                break;
            case TOKEN_UINT8_TYPE:
                printf("TOKEN_UINT8_TYPE\n");
                break;
            case TOKEN_INT16_TYPE:
                printf("TOKEN_INT16_TYPE\n");
                break;
            case TOKEN_UINT16_TYPE:
                printf("TOKEN_UINT16_TYPE\n");
                break;
            case TOKEN_INT32_TYPE:
                printf("TOKEN_INT32_TYPE\n");
                break;
            case TOKEN_UINT32_TYPE:
                printf("TOKEN_UINT32_TYPE\n");
                break;
            case TOKEN_INT64_TYPE:
                printf("TOKEN_INT64_TYPE\n");
                break;
            case TOKEN_UINT64_TYPE:
                printf("TOKEN_UINT64_TYPE\n");
                break;
            case TOKEN_BOOLEAN_TYPE:
                printf("TOKEN_BOOLEAN_TYPE\n");
                break;
            case TOKEN_STRING_TYPE:
                printf("TOKEN_STRING_TYPE\n");
                break;
            case TOKEN_ARRAY_TYPE:
                printf("TOKEN_ARRAY_TYPE\n");
                break;
            case TOKEN_RANGE32_TYPE:
                printf("TOKEN_RANGE32_TYPE\n");
                break;
            case TOKEN_RANGE64_TYPE:
                printf("TOKEN_RANGE64_TYPE\n");
                break;
            case TOKEN_LIST_TYPE:
                printf("TOKEN_LIST_TYPE\n");
                break;
            case TOKEN_TRUE:
                printf("TOKEN_TRUE\n");
                break;
            case TOKEN_FALSE:
                printf("TOKEN_FALSE\n");
                break;
            default:
                printf("unknown token %d\n", type);
                break;
        }
    }
};

struct Tokenizer {
    std::ifstream& in_file;
    char peek[2];

    Tokenizer(std::ifstream& in_file)
        : in_file(in_file)
    {
        memset(peek, 0, sizeof(peek));
    }

    char next_char() {
        char ch;

        if (peek[0]) {
            ch = peek[0];
            peek[0] = peek[1];
            peek[1] = 0;
        } else if (in_file.eof()) {
            ch = EOF;
        } else {
            in_file.get(ch);
        }
        return ch;
    }

    char peek_char() {
        if (!peek[0]) {
             in_file.get(peek[0]);
        }
        return peek[0];
    }

    void eat_whitespace() {
        while (1) {
            while (isspace(peek_char())) {
                next_char();
            }
            // handle C style comments
            if (peek_char() == '/') {
                // consume the '/'
                next_char();
                char ch = peek_char();
                if (ch == '/') {
                    // read until end of line
                    while ((ch = next_char()) != EOF && ch != '\n' && ch != '\r') {}
                    if (ch == EOF) {
                        break;
                    }
                } else if (ch == '*') {
                    next_char();    // consume '*'

                    // look for "*/"
                    while (1) {
                        while ((ch = next_char()) != EOF && ch != '*') {}
                        if (ch == EOF) {
                            return;
                        }
                        if (peek_char() == '/') {
                            // consume '/'
                            next_char();
                            break;
                        }
                    }
                } else {
                    // end of whitespace
                    // put characters we read into peek
                    peek[0] = '/';
                    peek[1] = ch;
                    return;
                }
            } else {
                break;
            }
        }
    }

    bool parse_identifier(Token& token, char ch) {
        std::string string;
        string.append(1, ch);

        ch = peek_char();
        while (isalnum(ch) || ch == '-' || ch == '_') {
            next_char();
            string.append(1, ch);
            ch = peek_char();
        }

        token.type = find_reserved_word(string);
        token.string_value = string;
        return true;
    }

    bool parse_integer(Token& token, char ch) {
        bool negative = false;
        bool hexadecimal = false;
        uint64_t value = 0;

        token.string_value.append(1, ch);

        if (ch == '-') {
            negative = true;
            ch = next_char();
            token.string_value.append(1, ch);
        } else if (ch == '0') {
            ch = next_char();
            token.string_value.append(1, ch);
            if (ch == 'x' || ch == 'X') {
                hexadecimal = true;
                ch = next_char();
                token.string_value.append(1, ch);
            }
        }

        // ch now contains highest order digit to parse
        int digit_count = 0;
        while (1) {
            int digit = -1;

            if (ch >= '0' && ch <= '9') {
                digit = ch - '0';
            } else if (hexadecimal) {
                if (ch >= 'A' && ch <= 'F') {
                    digit = ch - 'A' + 10;
                } else if (ch >= 'a' && ch <= 'f') {
                    digit = ch - 'a' + 10;
                }
            }

            if (digit < 0) {
                break;
            }

            if (hexadecimal) {
                value = 16 * value + digit;
            } else {
                value = 10 * value + digit;
            }

            if (++digit_count > 16) {
                fprintf(stderr, "integer value too large\n");
                return false;
            }

            ch = peek_char();
            if (!isdigit(ch) && !(hexadecimal &&
                                  ((ch >= 'A' && ch <= 'F') ||
                                   (ch >= 'a' && ch <= 'f')))) {
                break;
            }
            token.string_value.append(1, ch);
            next_char();
        }

        token.type = (negative ? TOKEN_NEG_INT_LITERAL : TOKEN_INT_LITERAL);
        if (negative && value > -INT64_MIN) {
            fprintf(stderr, "integer value too small\n");
            return false;
        }
        token.int_value = value;
        return true;
    }

    bool parse_string(Token& token) {
        std::string string;
        char ch = next_char();

        while (ch != EOF) {
            if (ch == '\\') {
                ch = next_char();
                if (ch == EOF) {
                    break;
                }
                switch (ch) {
                    case 'a':
                        ch = '\a';
                        break;
                    case 'b':
                        ch = '\b';
                        break;
                    case 'f':
                        ch = '\f';
                        break;
                    case 'n':
                        ch = '\n';
                        break;
                    case 'r':
                        ch = '\r';
                        break;
                    case 't':
                        ch = '\t';
                        break;
                    case 'v':
                        ch = '\v';
                        break;
                    case '\\':
                        ch = '\\';
                        break;
                    case '\'':
                        ch = '\'';
                        break;
                    case '\"':
                        ch = '\"';
                        break;
                    case '?':
                        ch = '?';
                        break;
                    default:
                        fprintf(stderr, "Unsupported escape sequence \\%c in string literal\n", ch);
                        return false;
                }
            } else if (ch == '\"') {
                token.type = TOKEN_STRING_LITERAL;
                token.string_value = string;
                return true;
            }
            string.append(1, ch);
            ch = next_char();
        }

        fprintf(stderr, "end of file during unterminated string\n");
        return false;
    }

    // returns false if we cannot parse the next token
    // EOF is not considered an error
    bool next_token(Token& token) {
        eat_whitespace();
        char ch = next_char();
        bool result = true;

        if (isalpha(ch)) {
           result = parse_identifier(token, ch);
        } else if (isdigit(ch) || ch == '-') {
            result = parse_integer(token, ch);
        } else if (ch == '\"') {
            result = parse_string(token);
        } else {
            switch (ch) {
                case EOF:
                    token.type = TOKEN_EOF;
                    return true;
                case '{':
                    token.type = TOKEN_LIST_START;
                    break;
                case '}':
                    token.type = TOKEN_LIST_END;
                    break;
                case '[':
                    token.type = TOKEN_ARRAY_START;
                    break;
                case ']':
                    token.type = TOKEN_ARRAY_END;
                    break;
                case '(':
                    token.type = TOKEN_RANGE_START;
                    break;
                case ')':
                    token.type = TOKEN_RANGE_END;
                    break;
                case '=':
                    token.type = TOKEN_EQUALS;
                    break;
                default:
                    fprintf(stderr, "invalid token \'%c\'\n", ch);
                    result = false;
            }
            token.string_value.clear();
            token.string_value.append(1, ch);
        }

#if PRINT_TOKENS
        if (result) {
            token.print();
        }
#endif

        return result;
    }
};

struct Node {
    mdi_node_t  node;
    std::string string_value;
    std::vector<Node> children;

    Node(mdi_id_t id) {
        node.id = id;
    }

    void print_indent(int depth) {
        for (int i = 0; i < depth; i++) {
            printf("    ");
        }
    }

    void print_children(int depth) {
        for (auto iter = children.begin(); iter != children.end(); iter++) {
            iter->print(depth);
        }
    }

    void print(int depth) {
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
};

static int parse_id_declaration(Tokenizer& tokenizer, mdi_type_t type) {
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

static int parse_node(Tokenizer& tokenizer, Token& token, Node& parent);

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

static int parse_node(Tokenizer& tokenizer, Token& token, Node& parent) {
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

static int run(std::vector<std::string>& in_paths, const char* out_path) {
    std::ofstream out_file;
    out_file.open(out_path, std::ofstream::out);
    if (!out_file.good()) {
        fprintf(stderr, "error: unable to open %s\n", out_path);
        return -1;
    }

    // add name for root node
    id_name_map[0] = "mdi_root";

    // root of our tree
    Node    root(MDI_ID(MDI_LIST, 0));

    // iterate through our input files
    for (auto iter = in_paths.begin(); iter != in_paths.end(); iter++) {
        const char* in_path = iter->c_str();
        std::ifstream in_file;
        in_file.open(in_path, std::ifstream::in);
    
        if (!in_file.good()) {
            fprintf(stderr, "error: unable to open %s\n", in_path);
            return -1;
        }

        Tokenizer tokenizer(in_file);
        while (1) {
            Token token;

            if (!tokenizer.next_token(token)) {
                return -1;
            }
            if (token.type == TOKEN_EOF) {
                // on to the next input file
                break;
            }

            // handle ID declarations        
            mdi_type_t type = token.get_type_name();
            if (type != MDI_INVALID_TYPE) {
                if (parse_id_declaration(tokenizer, type)) {
                    return -1;
                }
            } else if (token.type == TOKEN_IDENTIFIER) {
                if (parse_node(tokenizer, token, root)) {
                    return -1;
                }
            } else {
                fprintf(stderr, "unexpected token \"%s\" at top level\n", token.string_value.c_str());
                return -1;
            }
        }
    }

#if PRINT_PARSE_TREE
    root.print(0);
#endif

    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> in_paths;
    const char* out_path = nullptr;
 
    argc--;
    argv++;
 
    while (argc > 0) {
        const char *arg = argv[0];
        if (arg[0] != '-') {
            in_paths.push_back(arg);
        } else if (!strcmp(arg, "-o")) {
            if (argc < 2) {
              fprintf(stderr, "no output file given\n");
              return -1;
            } else if (out_path) {
                fprintf(stderr, "duplicate output file\n");
                return 1;
            }
            out_path = argv[1];
            argc--;
            argv++;
        } else {
            fprintf(stderr, "unknown argument \"%s\"\n", arg);
            return -1;
        }
        argc--;
        argv++;
    }

    if (in_paths.size() == 0) {
        fprintf(stderr, "no input files specified\n");
        return -1;
    }
    if (!out_path) {
        fprintf(stderr, "no output file specified\n");
        return -1;
    }

    return run(in_paths, out_path);
}
