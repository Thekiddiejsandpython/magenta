// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/compiler.h>
#include <stdint.h>

__BEGIN_CDECLS;

// MDI Nodes are aligned to 4 byte boundaries
#define MDI_ALIGNMENT   4
#define MDI_ALIGN(x)    (((x) + MDI_ALIGNMENT - 1) & ~(MDI_ALIGNMENT - 1))

// MDI node type
typedef enum {
    MDI_INVALID_TYPE = -1,
    MDI_LIST = 0,   // node is a list of children
    MDI_INT8,       // signed 8-bit integer type
    MDI_UINT8,      // unsigned 8-bit integer type
    MDI_INT16,      // signed 16-bit integer type
    MDI_UINT16,     // unsigned 16-bit integer type
    MDI_INT32,      // signed 32-bit integer type
    MDI_UINT32,     // unsigned 32-bit integer type
    MDI_INT64,      // signed 64-bit integer type
    MDI_UINT64,     // unsigned 64-bit integer type
    MDI_BOOLEAN,    // boolean type
    MDI_STRING,     // zero terminated char string
    MDI_ARRAY,      // list of children with same type
} mdi_type_t;

// MDI node identifier
// encodes both name and type of the node
typedef uint32_t mdi_id_t;
#define MDI_ID_TYPE_SHIFT   24
#define MDI_ID_TYPE(id)     (mdi_type_t)(id >> MDI_ID_TYPE_SHIFT)
#define MDI_MAX_ID          ((1 << MDI_ID_TYPE_SHIFT) - 1)
#define MDI_ID(type, num)   ((type << MDI_ID_TYPE_SHIFT) | num)

typedef struct {
    mdi_id_t    id;
    uint32_t    length; // total length of the node, including subtree
    union {
        uint32_t    list_count; // number of children following this struct
        int8_t      i8;
        uint8_t     u8;         // also used for boolean
        int16_t     i16;
        uint16_t    u16;
        int32_t     i32;
        uint32_t    u32;
        int64_t     i64;
        uint64_t    u64;
        uint32_t    str_len;    // length of zero terminated string following this struct
        struct {
            mdi_type_t type;    // type of array elements following this struct
            uint32_t count;     // number of array elements following this struct
        } array;
    } value;
} __attribute__ ((packed)) mdi_node_t;


__END_CDECLS;
