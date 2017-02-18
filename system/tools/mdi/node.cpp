// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "node.h"
#include "parser.h"

Node::Node(mdi_id_t i)
    : id(i) {
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
    const char* name = get_id_name(id);

    if (name && name[0]) {
        print_indent(depth);
        printf("%s = ", name);
    }

    switch (MDI_ID_TYPE(id)) {
        case MDI_INT32:
            printf("%d", (int)int_value);
            break;
        case MDI_UINT32:
            printf("%u", (unsigned int)int_value);
            break;         
        case MDI_UINT64:
            printf("%" PRIu64, int_value);
            break;
        case MDI_BOOLEAN:
            printf("%s", (int_value ? "true" : "false"));
            break;  
        case MDI_STRING:
            printf("%s", string_value.c_str());
            break;  
        case MDI_ARRAY:
            printf("[ ");
            print_children(depth + 1);
            printf("]");
            break;
        case MDI_LIST:
            printf("{\n");
            print_children(depth + 1);
            print_indent(depth);
            printf("}");
            break;
        case MDI_INVALID_TYPE:
            assert(0);
            break;
    }

    if (name && name[0]) {
        printf("\n");
    } else {
        // keep array elements on same line
        printf(" ");
    }
}

void Node::compute_array_length() {
    // recurse through our children first
    int children_length = 0;
    int child_count = children.size();

    if (array_element_type == MDI_STRING || array_element_type == MDI_LIST ||
            array_element_type == MDI_ARRAY) {
        for (auto iter = children.begin(); iter != children.end(); iter++) {
            iter->compute_node_length();
            children_length += iter->array_element_length;
        }
    }

    switch (array_element_type) {
        case MDI_INT32:
        case MDI_UINT32:
            length = MDI_ALIGN(sizeof(mdi_node_t) + child_count * sizeof(uint32_t));
            break;
        case MDI_UINT64:
            length = MDI_ALIGN(sizeof(mdi_node_t) + child_count * sizeof(uint64_t));
            break;
        case MDI_BOOLEAN:
            length = MDI_ALIGN(sizeof(mdi_node_t) + child_count * sizeof(uint8_t));
            break;
        case MDI_STRING:
        case MDI_LIST:
        case MDI_ARRAY:
            // mdi_node_t is followed by list of offsets and then element values
            length = MDI_ALIGN(sizeof(mdi_node_t) + child_count * sizeof(mdi_offset_t) + children_length);
            break;
        default:
            assert(0);
            break;
    }
}

void Node::compute_node_length() {
    switch (MDI_ID_TYPE(id)) {
        case MDI_INT32:
        case MDI_UINT32:
        case MDI_UINT64:
        case MDI_BOOLEAN:
            // primitive types are self contained
            length = sizeof(mdi_node_t);
            // array_element_length not used for these types
            array_element_length = 0;
            break;
        case MDI_STRING:
            // zero terminated string follows the mdi_node_t
            array_element_length = string_value.length() + 1;
            length = MDI_ALIGN(sizeof(mdi_node_t) + array_element_length);
            break;  
        case MDI_ARRAY:
            compute_array_length();
            array_element_length = length;
            break;
        case MDI_LIST:
            length = sizeof(mdi_node_t);
            // recurse through our children
            for (auto iter = children.begin(); iter != children.end(); iter++) {
                iter->compute_node_length();
                length += iter->length;
            }
            array_element_length = length;
            break;
        case MDI_INVALID_TYPE:
        default:
            printf("invalid type %d\n", MDI_ID_TYPE(id));
            assert(0);
            break;
    }
}

bool Node::serialize(std::ofstream& out_file) {
    mdi_node_t node;
    static_assert(sizeof(node) == MDI_ALIGN(sizeof(node)), "");

    memset(&node, 0, sizeof(node));
    node.id = id;
    node.length = length;
    mdi_type_t type = MDI_ID_TYPE(id);
    int child_count = children.size();

    switch (type) {
        case MDI_INT32:
        case MDI_UINT32:
            node.value.u32 = (uint32_t)int_value;
            break;
        case MDI_UINT64:
            node.value.u64 = int_value;
            break;
        case MDI_BOOLEAN:
            node.value.bool_value = int_value;
            break;
        case MDI_STRING:
            node.value.str_len = string_value.length() + 1;
            break;
        case MDI_ARRAY:
            node.value.array.type = array_element_type;
            node.value.array.count = children.size();
            break;
        case MDI_LIST:
            node.value.list.count = children.size();
            break;
        case MDI_INVALID_TYPE:
        default:
            assert(0);
            return false;
    }

    out_file.write((const char *)&node, sizeof(node));

    // length following node that may need padding
    int pad_length = 0;

    if (type == MDI_STRING) {
        // string values are written immediately after the mdi_node_t
        int strlen = string_value.length() + 1;
        out_file.write(string_value.c_str(), strlen);
        // may need to pad following string value
        pad_length = strlen;
    } else if (type == MDI_LIST) {
        // children are recursively written following node
        for (auto iter = children.begin(); iter != children.end(); iter++) {
            iter->serialize(out_file);
        }
    } else if (type == MDI_ARRAY) {
        // array element values are written immediately after the mdi_node_t
        // handled differently depending on element type
        switch (array_element_type) {
            case MDI_INT32:
            case MDI_UINT32:
                // raw values immediately follow node
                for (auto iter = children.begin(); iter != children.end(); iter++) {
                    uint32_t value = iter->int_value;
                    out_file.write((const char *)&value, sizeof(value));
                }
                pad_length = child_count * sizeof(uint32_t);
                break;
            case MDI_UINT64:
                // raw values immediately follow node
                for (auto iter = children.begin(); iter != children.end(); iter++) {
                    uint64_t value = iter->int_value;
                    out_file.write((const char *)&value, sizeof(value));
                }
                pad_length = child_count * sizeof(uint64_t);
                break;
            case MDI_BOOLEAN:
                // raw values immediately follow node
                for (auto iter = children.begin(); iter != children.end(); iter++) {
                    uint8_t value = iter->int_value;
                    out_file.write((const char *)&value, sizeof(value));
                }
                pad_length = child_count * sizeof(uint8_t);
                break;
            case MDI_STRING:
            case MDI_LIST:
            case MDI_ARRAY: {
                // here we write offsets to element values followed by the actual elements
                // compute offset of first element value relative to beginning of node
                mdi_offset_t offset = sizeof(node) + child_count * sizeof(mdi_offset_t);

                // write element offsets
                for (auto iter = children.begin(); iter != children.end(); iter++) {
                    out_file.write((const char *)&offset, sizeof(offset));
                    offset += iter->array_element_length;
                }
                // write element values
                if (array_element_type == MDI_STRING) {
                    // write raw strings
                    for (auto iter = children.begin(); iter != children.end(); iter++) {
                        int strlen = iter->string_value.length() + 1;
                        out_file.write(iter->string_value.c_str(), strlen);
                        pad_length += strlen;
                    }
                } else {
                    // recursively serialize list and array elements
                    for (auto iter = children.begin(); iter != children.end(); iter++) {
                        iter->serialize(out_file);
                    }
                    // list and array elements will be aligned, so no need to update pad_length
                }
                break;
            }                
            default:
                assert(0);
                break;
        }
    }

    // align to MDI_ALIGNMENT boundary
    int pad = MDI_ALIGN(pad_length) - pad_length;
    if (pad) {
        char zeros[MDI_ALIGNMENT] = { 0 };
        out_file.write(zeros, pad);
    }

// TODO - error handling
    return true;
}
