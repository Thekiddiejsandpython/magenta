// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <lib/mdi.h>
#include <magenta/mdi.h>
#include <stdio.h>

#if EMBED_MDI
extern const uint8_t embedded_mdi[];
extern const uint32_t embedded_mdi_len;
#endif


// takes pointer to MDI header and returns reference to MDI root node
mx_status_t mdi_init(void* mdi_header, mdi_node_ref_t* out_node_ref) {
    // TODO - check header once we define one
    *out_node_ref = (mdi_node_ref_t)mdi_header;
    return NO_ERROR;
}

#if EMBED_MDI
// returns reference to MDI root node from MDI embedded in kernel image
mx_status_t mdi_init_embedded(mdi_node_ref_t* out_node_ref) {
    *out_node_ref = (mdi_node_ref_t)embedded_mdi;
    return NO_ERROR;
}
#endif
