// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fs-management/mount.h>
#include <fs/vfs.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <magenta/compiler.h>
#include <magenta/device/devmgr.h>
#include <magenta/processargs.h>
#include <magenta/syscalls.h>
#include <mxio/util.h>

static mx_status_t fsck_minfs(const char* devicepath, LaunchCallback cb) {
    mx_handle_t hnd[MXIO_MAX_HANDLES * 2];
    uint32_t ids[MXIO_MAX_HANDLES * 2];
    size_t n = 0;
    int device_fd;
    if ((device_fd = open(devicepath, O_RDWR)) < 0) {
        fprintf(stderr, "Failed to open device\n");
        return ERR_BAD_STATE;
    }
    mx_status_t status;
    if ((status = mxio_transfer_fd(device_fd, FS_FD_BLOCKDEVICE, hnd + n, ids + n)) <= 0) {
        fprintf(stderr, "Failed to access device handle\n");
        return status != 0 ? status : ERR_BAD_STATE;
    }
    n += status;

    const char* argv[] = {"/boot/bin/minfs", "fsck"};
    return cb(countof(argv), argv, hnd, ids, n);
}

static mx_status_t fsck_fat(const char* devicepath, LaunchCallback cb) {
    const char* argv[] = {"/boot/bin/fsck-msdosfs", devicepath};
    return cb(countof(argv), argv, NULL, NULL, 0);
}

mx_status_t fsck(const char* devicepath, disk_format_t df, LaunchCallback cb) {
    switch (df) {
    case DISK_FORMAT_MINFS:
        return fsck_minfs(devicepath, cb);
    case DISK_FORMAT_FAT:
        return fsck_fat(devicepath, cb);
    default:
        return ERR_NOT_SUPPORTED;
    }
}
