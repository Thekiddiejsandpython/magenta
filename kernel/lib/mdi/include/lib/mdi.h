// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <magenta/types.h>

// opaque reference to an MDI node
typedef struct mdi_node_ref* mdi_node_ref_t;

// takes pointer to MDI header and returns reference to MDI root node
mx_status_t mdi_init(void* mdi_header, mdi_node_ref_t* out_node_ref);

#if EMBED_MDI
// returns reference to MDI root node from MDI embedded in kernel image
mx_status_t mdi_init_embedded(mdi_node_ref_t* out_node_ref);
#endif
