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
    out_ref->remaining_siblings = 0; 
    return NO_ERROR;
}

#if EMBED_MDI
// returns reference to MDI root node from MDI embedded in kernel image
mx_status_t mdi_init_embedded(mdi_node_ref_t* out_ref) {
    out_ref->node = (mdi_node_t *)embedded_mdi;
    out_ref->remaining_siblings = 0; 
    return NO_ERROR;
}
#endif

mx_status_t mdi_node_get_int32(const mdi_node_ref_t* ref, int32_t* out_value) {
    if (mdi_get_type(ref) != MDI_INT32) {
        printf("bad node type for mdi_node_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.i32;
    return NO_ERROR;
}

mx_status_t mdi_node_get_uint32(const mdi_node_ref_t* ref, uint32_t* out_value) {
    if (mdi_get_type(ref) != MDI_UINT32) {
        printf("bad node type for mdi_node_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.u32;
    return NO_ERROR;
}

mx_status_t mdi_node_get_uint64(const mdi_node_ref_t* ref, uint64_t* out_value) {
    if (mdi_get_type(ref) != MDI_UINT64) {
        printf("bad node type for mdi_node_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.u64;
    return NO_ERROR;
}

mx_status_t mdi_node_get_boolean(const mdi_node_ref_t* ref, bool* out_value) {
    if (mdi_get_type(ref) != MDI_BOOLEAN) {
        printf("bad node type for mdi_node_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    *out_value = ref->node->value.bool_value;
    return NO_ERROR;
}

const char* mdi_node_get_string(const mdi_node_ref_t* ref) {
    if (mdi_get_type(ref) != MDI_STRING) {
        printf("bad node type for mdi_get_string_value\n");
        return NULL;
    }
    return (const char *)ref->node + sizeof(ref->node);
}

uint32_t mdi_array_get_length(const mdi_node_ref_t* ref) {
    if (mdi_get_type(ref) == MDI_ARRAY) {
        return ref->node->value.array.count;
    } else {
        printf("bad node type for mdi_array_get_length\n");
        return -1;
    }
}

mx_status_t mdi_array_get_int32(const mdi_node_ref_t* ref, uint32_t index, int32_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    const mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_INT32) {
        printf("bad array element type for mdi_array_get_int32\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_int32\n");
        return ERR_OUT_OF_RANGE;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((int32_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_uint32(const mdi_node_ref_t* ref, uint32_t index, uint32_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    const mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_UINT32) {
        printf("bad array element type for mdi_array_get_uint32\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_uint32\n");
        return ERR_OUT_OF_RANGE;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((uint32_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_uint64(const mdi_node_ref_t* ref, uint32_t index, uint64_t* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    const mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_UINT64) {
        printf("bad array element type for mdi_array_get_uint64\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_uint64\n");
        return ERR_OUT_OF_RANGE;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((uint64_t *)array_data)[index];
    return NO_ERROR;
}

mx_status_t mdi_array_get_boolean(const mdi_node_ref_t* ref, uint32_t index, bool* out_value) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    const mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_BOOLEAN) {
        printf("bad array element type for mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_boolean\n");
        return ERR_OUT_OF_RANGE;
    }
    void* array_data = (char *)node + sizeof(*node);
    *out_value = ((bool *)array_data)[index];
    return NO_ERROR;
}

const char* mdi_array_get_string(const mdi_node_ref_t* ref, uint32_t index) {
    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return NULL;
    }
    const mdi_node_t* node = ref->node;
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

mx_status_t mdi_array_get_node(const mdi_node_ref_t* ref, uint32_t index, mdi_node_ref_t* out_ref) {
    out_ref->node = NULL;
    out_ref->remaining_siblings = 0;

    if (mdi_get_type(ref) != MDI_ARRAY) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    const mdi_node_t* node = ref->node;
    if (node->value.array.type != MDI_LIST && node->value.array.type != MDI_ARRAY) {
        printf("bad array element type for mdi_array_get_node\n");
        return ERR_WRONG_TYPE;
    }
    if (index >= node->value.array.count) {
        printf("array index out of range in mdi_array_get_boolean\n");
        return ERR_OUT_OF_RANGE;
    }
    mdi_offset_t* offsets = (mdi_offset_t *)((char *)node + sizeof(*node));
    out_ref->node = (mdi_node_t *)((const char *)node + offsets[index]);
    return NO_ERROR;
}

mx_status_t mdi_list_get_first_child(const mdi_node_ref_t* ref, mdi_node_ref_t* out_ref) {
    out_ref->node = NULL;
    out_ref->remaining_siblings = 0;

   if (mdi_get_type(ref) != MDI_LIST) {
        printf("ref not an array in mdi_array_get_boolean\n");
        return ERR_WRONG_TYPE;
    }
    if (ref->node->value.list.count == 0) {
        return ERR_NOT_FOUND;
    }
    // first child immediately follows list node
    out_ref->node = &ref->node[1];
    out_ref->remaining_siblings = ref->node->value.list.count - 1;
    return NO_ERROR;
}

mx_status_t mdi_list_get_next_child(const mdi_node_ref_t* ref, mdi_node_ref_t* out_ref) {
    if (ref->remaining_siblings == 0) {
        out_ref->node = NULL;
        out_ref->remaining_siblings = 0;
        return ERR_NOT_FOUND;
    }
    
    out_ref->node = (mdi_node_t *)((char *)ref->node + ref->node->length);
    out_ref->remaining_siblings = ref->remaining_siblings - 1;
    return NO_ERROR;
}

uint32_t mdi_node_get_child_count(const mdi_node_ref_t* ref) {
    if (mdi_get_type(ref) == MDI_LIST) {
        return ref->node->value.list.count;
    } else {
        printf("bad node type for mdi_node_get_child_count\n");
        return -1;
    }
}
