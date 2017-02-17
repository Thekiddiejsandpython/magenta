// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <fstream>

#include <magenta/mdi.h>

#include "tokens.h"

struct Node;

const char* get_id_name(mdi_id_t id);
bool parse_id_declaration(Tokenizer& tokenizer, mdi_type_t type);
bool parse_node(Tokenizer& tokenizer, Token& token, Node& parent);
bool print_header_file(std::ofstream& out_file);
