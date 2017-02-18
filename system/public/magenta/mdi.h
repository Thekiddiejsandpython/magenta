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

// 32-bit offset used for array element offsets
typedef uint32_t mdi_offset_t;

// MDI node type
typedef enum {
    MDI_INVALID_TYPE,
    MDI_INT32,      // signed 32-bit integer type
    MDI_UINT32,     // unsigned 32-bit integer type
    MDI_UINT64,     // unsigned 64-bit integer type
    MDI_BOOLEAN,    // boolean type
    MDI_STRING,     // zero terminated char string
    MDI_LIST,       // node is a list of children
    MDI_ARRAY,      // array of elements with same type laid out for fast random access
} mdi_type_t;

// MDI node identifier
// encodes both name and type of the node
typedef uint32_t mdi_id_t;
#define MDI_ID_TYPE_SHIFT   24
#define MDI_MAX_ID          ((1 << MDI_ID_TYPE_SHIFT) - 1)
#define MDI_ID_TYPE(id)     (mdi_type_t)((id) >> MDI_ID_TYPE_SHIFT)
#define MDI_ID_NUM(id)      ((id) & MDI_MAX_ID)
#define MDI_ID(type, num)   ((type << MDI_ID_TYPE_SHIFT) | num)

static const mdi_id_t mdi_root_id = MDI_ID(MDI_LIST, 0);

// mdi_node_t represents a node in the device index.
// For integer and boolean types, the mdi_node_t is self contained and
// mdi_node_t.length is sizeof(mdi_node_t).
// Nodes of type MDI_STRING are immediately followed by a zero terminated char string.
// Nodes of type MDI_LIST are followed by the list's child nodes.
// Nodes of type MDI_ARRAY are followed by the raw array element values.
// For arrays with integer or boolean element type, the node is followed by an array array of values.
// For arrays with element type MDI_STRING, the mdi_node_t is followed by an array of uint32_t
// offsets to the zero terminated string values (relative to the address of the mdi_node_t),
// and then the actual string values.
// For arrays with element type MDI_LIST or MDI_ARRAY, the mdi_node_t is followed by an array of
// uint32_t offsets to the element nodes, followed by the child element themselves.
typedef struct {
    mdi_id_t    id;
    uint32_t    length;         // total length of the node, including subtree
    union {
        int32_t     i32;
        uint32_t    u32;
        uint64_t    u64;
        uint8_t     bool_value;
        uint32_t    str_len;    // length of zero terminated string following this struct
        struct {
            uint32_t count;     // number of list elements following this struct
        } list;
        struct {
            mdi_type_t type;    // type of array elements following this struct
            uint32_t count;     // number of array elements following this struct
        } array;
    } value;
} __attribute__ ((packed)) mdi_node_t;

__END_CDECLS;
