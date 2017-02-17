// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <magenta/mdi.h>

struct Node {
    mdi_id_t id;
    uint64_t int_value;
    std::string string_value;
    mdi_type_t array_element_type;
    uint32_t length;
    uint32_t array_element_length;   // length of node when serialized as an array element
    std::vector<Node> children;

    Node(mdi_id_t id);

    void print_indent(int depth);
    void print_children(int depth);
    void print(int depth);

    void compute_array_length();
    void compute_node_length();

    bool serialize(std::ofstream& out_file);
};
