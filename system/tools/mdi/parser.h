// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <magenta/mdi.h>

#include "tokens.h"

struct Node {
    mdi_id_t id;
    uint64_t int_value;
    bool bool_value;
    std::string string_value;
    mdi_type_t array_element_type;
    uint32_t length;
    std::vector<Node> children;

    Node(mdi_id_t id);

    void print_indent(int depth);
    void print_children(int depth);
    void print(int depth);

    void compute_node_length();

    bool serialize(std::ofstream& out_file);
};

bool parse_id_declaration(Tokenizer& tokenizer, mdi_type_t type);
bool parse_node(Tokenizer& tokenizer, Token& token, Node& parent);
bool print_header_file(std::ofstream& out_file);
