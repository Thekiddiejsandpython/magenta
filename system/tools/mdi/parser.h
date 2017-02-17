// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <fstream>

#include <magenta/mdi.h>

#include "tokens.h"

struct Node;

const char* get_id_name(mdi_id_t id);
bool process_file(const char* in_path, Node& root);
bool print_header_file(std::ofstream& out_file);
