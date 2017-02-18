// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <stdio.h>

#include <lib/mdi.h>

#if EMBED_MDI
extern const uint8_t embedded_mdi[];
extern const uint32_t embedded_mdi_len;
#endif

// takes pointer to MDI header and returns reference to MDI root node
mx_status_t mdi_init(void* mdi_header, mdi_node_ref_t* out_ref) {
    // TODO - check header once we define one
    out_ref->node = (mdi_node_t *)mdi_header;
    return NO_ERROR;
}

#if EMBED_MDI
// returns reference to MDI root node from MDI embedded in kernel image
mx_status_t mdi_init_embedded(mdi_node_ref_t* out_ref) {
    out_ref->node = (mdi_node_t *)embedded_mdi;
    return NO_ERROR;
}
#endif

mx_status_t mdi_node_get_int32(mdi_node_ref_t* ref, int32_t* out_value) {
    if (mdi_get_type(ref) != MDI_INT32) {
        printf("bad node type for mdi_node_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.i32;
    return NO_ERROR;
}

mx_status_t mdi_node_get_uint32(mdi_node_ref_t* ref, uint32_t* out_value) {
    if (mdi_get_type(ref) != MDI_UINT32) {
        printf("bad node type for mdi_node_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.u32;
    return NO_ERROR;
}

mx_status_t mdi_node_get_uint64(mdi_node_ref_t* ref, uint64_t* out_value) {
    if (mdi_get_type(ref) != MDI_UINT64) {
        printf("bad node type for mdi_node_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.u64;
    return NO_ERROR;
}

mx_status_t mdi_node_get_boolean(mdi_node_ref_t* ref, bool* out_value) {
    if (mdi_get_type(ref) != MDI_BOOLEAN) {
        printf("bad node type for mdi_node_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.bool_value;
    return NO_ERROR;
}

const char* mdi_node_get_string(mdi_node_ref_t* ref) {
    if (mdi_get_type(ref) != MDI_STRING) {
        printf("bad node type for mdi_get_string_value\n");
        return NULL;
    }
    return (const char *)ref->node + sizeof(ref->node);
}

mx_status_t mdi_array_get_int32(mdi_node_ref_t* ref, uint32_t index, int32_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_INT32) {
        printf("bad array element type for mdi_array_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_int32\n");
        return ERR_INVALID_ARGS;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((int32_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_uint32(mdi_node_ref_t* ref, uint32_t index, uint32_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_UINT32) {
        printf("bad array element type for mdi_array_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_uint32\n");
        return ERR_INVALID_ARGS;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((uint32_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_uint64(mdi_node_ref_t* ref, uint32_t index, uint64_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_UINT64) {
        printf("bad array element type for mdi_array_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_uint64\n");
        return ERR_INVALID_ARGS;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((uint64_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_boolean(mdi_node_ref_t* ref, uint32_t index, bool* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_BOOLEAN) {
        printf("bad array element type for mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_boolean\n");
        return ERR_INVALID_ARGS;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((bool *)array_data)[index];
    return NO_ERROR;
}

const char* mdi_array_get_string(mdi_node_ref_t* ref, uint32_t index) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return NULL;
    }
    mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_STRING) {
        printf("bad array element type for mdi_array_get_boolean\n");
        return NULL;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_boolean\n");
        return NULL;
    }
    mdi_offset_t* offsets = (mdi_offset_t *)((char *)node + sizeof(*node));
    return (const char *)node + offsets[index];
}

uint32_t mdi_node_get_child_count(mdi_node_ref_t* ref) {
    switch (mdi_get_type(ref)) {
        case MDI_LIST:
            return ref->node->value.list.count;
        case MDI_ARRAY:
            return ref->node->value.array.count;
        default:
            return 0;
    }
}
