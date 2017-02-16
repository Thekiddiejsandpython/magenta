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

int mdi_init(void) {
#if EMBED_MDI
    uint32_t* ptr = (uint32_t *)embedded_mdi;
    printf("embedded_mdi: %08X %08X %08X %08X length: %u\n",
            ptr[0], ptr[1], ptr[2], ptr[3], embedded_mdi_len);
#endif

    return 0;
}