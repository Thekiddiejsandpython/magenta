// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
#include <vector>

#include <magenta/mdi.h>

#include "node.h"
#include "parser.h"
#include "tokens.h"

#define PRINT_PARSE_TREE 1

static bool run(std::vector<std::string>& in_paths, const char* out_path,
               const char* out_header_path) {
    std::ofstream out_file;
    out_file.open(out_path, std::ofstream::binary);
    if (!out_file.good()) {
        fprintf(stderr, "error: unable to open %s\n", out_path);
        return false;
    }

    // root of our tree
    Node    root(MDI_ID(MDI_LIST, 0));

    // iterate through our input files
    for (auto iter = in_paths.begin(); iter != in_paths.end(); iter++) {
        const char* in_path = iter->c_str();
        if (!process_file(in_path, root)) {
            return false;
        }
     }

    root.compute_node_length();
    root.serialize(out_file);

    if (out_header_path) {
        std::ofstream out_file;
        out_file.open(out_header_path, std::ofstream::binary);
        if (!out_file.good()) {
            fprintf(stderr, "error: unable to open %s\n", out_header_path);
            return false;
        }
        print_header_file(out_file);
    }

#if PRINT_PARSE_TREE
    root.print(0);
#endif

    return true;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> in_paths;
    const char* out_path = nullptr;
    const char* out_header_path = nullptr;
 
    argc--;
    argv++;
 
    while (argc > 0) {
        const char *arg = argv[0];
        if (arg[0] != '-') {
            in_paths.push_back(arg);
        } else if (!strcmp(arg, "-o") && argc >= 2) {
            if (out_path) {
                fprintf(stderr, "duplicate output file\n");
                return 1;
            }
            out_path = argv[1];
            argc--;
            argv++;
        } else if (!strcmp(arg, "-h") && argc >= 2) {
            if (out_header_path) {
                fprintf(stderr, "duplicate header output file\n");
                return 1;
            }
            out_header_path = argv[1];
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

    return run(in_paths, out_path, out_header_path) ? 0 : -1;
}
