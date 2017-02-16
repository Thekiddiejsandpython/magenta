// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <magenta/mdi.h>

static void dump_node(int fd, int level) {
    mdi_node_t node;

    if (read(fd, &node, sizeof(node)) != sizeof(node)) {
        fprintf(stderr, "read failed!\n");
        abort();
    }

    mdi_type_t type = MDI_ID_TYPE(node.id);
    uint32_t id_num = MDI_ID_NUM(node.id);
    int child_count = 0;

    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    switch (type) {
        case MDI_LIST:
            printf("list(%u)", id_num);
            child_count = node.value.list_count;
            break;
        case MDI_INT8:
            printf("int8(%u) = %d", id_num, node.value.i8);
            break;
        case MDI_UINT8:
            printf("uint8(%u) = %u", id_num, node.value.u8);
            break;
        case MDI_INT16:
            printf("int16(%u) = %d", id_num, node.value.i16);
            break;
        case MDI_UINT16:
            printf("uint16(%u) = %u", id_num, node.value.u16);
            break;
        case MDI_INT32:
            printf("int32(%u) = %d", id_num, node.value.i32);
            break;
        case MDI_UINT32:
            printf("uint32(%u) = %u", id_num, node.value.u32);
            break;
        case MDI_INT64:
            printf("int64(%u) = %" PRId64, id_num, node.value.i64);
            break;
        case MDI_UINT64:
            printf("uint64(%u) = %" PRIu64, id_num, node.value.u64);
            break;
        case MDI_BOOLEAN:
            printf("boolean(%u) = %s", id_num, (node.value.u8 ? "true" : "false"));
            break;
        case MDI_STRING: {
            printf("string(%u)", id_num);
            char* buffer = new char[node.length - sizeof(node)];
            read(fd, buffer, node.length - sizeof(node));
            printf(" %s", buffer);
            delete[] buffer;
            break;
        }
        case MDI_ARRAY:
            printf("array(%u)", id_num);
            child_count = node.value.array.count;
            break;
        default:
            fprintf(stderr, "unknown type %d\n", type);
    }
    printf("\n");

    for (int i = 0; i < child_count; i++) {
        dump_node(fd, level + 1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: mdidump <mdi-file-path>\n");
        return -1;
    }

    const char* path = argv[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "error: unable to open %s\n", path);
        return -1;
    }

    dump_node(fd, 0);

    close(fd);

    return 0;
}