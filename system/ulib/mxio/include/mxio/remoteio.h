// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/compiler.h>
#include <magenta/types.h>
#include <mxio/io.h>

#include <assert.h>
#include <limits.h>
#include <stdint.h>

__BEGIN_CDECLS

// clang-format off

#define MXRIO_HDR_SZ       (__builtin_offsetof(mxrio_msg_t, data))

#define MXRIO_ONE_HANDLE   0x00000100

#define MXRIO_STATUS       0x00000000
#define MXRIO_CLOSE        0x00000001
#define MXRIO_CLONE       (0x00000002 | MXRIO_ONE_HANDLE)
#define MXRIO_OPEN        (0x00000003 | MXRIO_ONE_HANDLE)
#define MXRIO_MISC         0x00000004
#define MXRIO_READ         0x00000005
#define MXRIO_WRITE        0x00000006
#define MXRIO_SEEK         0x00000007
#define MXRIO_STAT         0x00000008
#define MXRIO_READDIR      0x00000009
#define MXRIO_IOCTL        0x0000000a
#define MXRIO_IOCTL_1H    (0x0000000a | MXRIO_ONE_HANDLE)
#define MXRIO_UNLINK       0x0000000b
#define MXRIO_READ_AT      0x0000000c
#define MXRIO_WRITE_AT     0x0000000d
#define MXRIO_TRUNCATE     0x0000000e
#define MXRIO_RENAME      (0x0000000f | MXRIO_ONE_HANDLE)
#define MXRIO_CONNECT      0x00000010
#define MXRIO_BIND         0x00000011
#define MXRIO_LISTEN       0x00000012
#define MXRIO_GETSOCKNAME  0x00000013
#define MXRIO_GETPEERNAME  0x00000014
#define MXRIO_GETSOCKOPT   0x00000015
#define MXRIO_SETSOCKOPT   0x00000016
#define MXRIO_GETADDRINFO  0x00000017
#define MXRIO_SETATTR      0x00000018
#define MXRIO_SYNC         0x00000019
#define MXRIO_NUM_OPS      26

#define MXRIO_OP(n)        ((n) & 0x3FF) // opcode
#define MXRIO_HC(n)        (((n) >> 8) & 3) // handle count
#define MXRIO_OPNAME(n)    ((n) & 0xFF) // opcode, "name" part only

#define MXRIO_OPNAMES { \
    "status", "close", "clone", "open", \
    "misc", "read", "write", "seek", \
    "stat", "readdir", "ioctl", "unlink", \
    "read_at", "write_at", "truncate", "rename", \
    "connect", "bind", "listen", "getsockname", \
    "getpeername", "getsockopt", "setsockopt", "getaddrinfo", \
    "setattr", "sync" }

const char* mxio_opname(uint32_t op);

typedef struct mxrio_msg mxrio_msg_t;

typedef mx_status_t (*mxrio_cb_t)(mxrio_msg_t* msg, mx_handle_t rh, void* cookie);
// callback to process a mxrio_msg
// - on entry datalen indicates how much valid data is in msg.data[]
// - return value will be placed in msg.arg, negative is an error,
//   positive values are opcode-specific
// - on non-error return msg.len indicates how much valid data to
//   send.  On error return msg.len will be set to 0.
// - if rh is non-zero it is a reply handle which may be used for
//   deferred replies.  In which case the callback must return
//   ERR_DISPATCHER_INDIRECT to differentiate this from an immediate
//   reply or error

// a mxio_dispatcher_handler suitable for use with a mxio_dispatcher
mx_status_t mxrio_handler(mx_handle_t h, void* cb, void* cookie);


// OPEN and CLOSE messages, can be forwarded to another remoteio server,
// without any need to wait for a reply.  The reply channel from the initial
// request is passed along to the new server.
// If the write to the server fails, an error reply is sent to the reply channel.
void mxrio_txn_handoff(mx_handle_t server, mx_handle_t reply, mxrio_msg_t* msg);


// OPEN and CLONE ops do not return a reply
// Instead they receive a channel handle that they write their status
// and (if successful) type, extra data, and handles to.

#define MXRIO_OBJECT_EXTRA 32
#define MXRIO_OBJECT_MINSIZE (2 * sizeof(uint32_t))
#define MXRIO_OBJECT_MAXSIZE (MXRIO_OBJECT_MINSIZE + MXRIO_OBJECT_EXTRA)

typedef struct {
    // Required Header
    mx_status_t status;
    uint32_t type;

    // Optional Extra Data
    uint8_t extra[MXRIO_OBJECT_EXTRA];

    // OOB Data
    uint32_t esize;
    uint32_t hcount;
    mx_handle_t handle[MXIO_MAX_HANDLES];
} mxrio_object_t;


struct mxrio_msg {
    uint32_t txid;                     // transaction id
    uint32_t op;                       // opcode
    uint32_t datalen;                  // size of data[]
    int32_t arg;                       // tx: argument, rx: return value
    union {
        int64_t off;                   // tx/rx: offset where needed
        uint32_t mode;                 // tx: Open
        uint32_t protocol;             // rx: Open
        uint32_t op;                   // tx: Ioctl
    } arg2;
    int32_t reserved;
    uint32_t hcount;                   // number of valid handles
    mx_handle_t handle[4];             // up to 3 handles + reply channel handle
    uint8_t data[MXIO_CHUNK_SIZE];     // payload
};

static_assert(MXIO_CHUNK_SIZE >= PATH_MAX, "MXIO_CHUNK_SIZE must be large enough to contain paths");

// - msg.datalen is the size of data sent or received and must be <= MXIO_CHUNK_SIZE
// - msg.arg is the return code on replies

// request---------------------------------------    response------------------------------
// op          arg        arg2     data              arg2        data            handle[]
// ----------- ---------- -------  --------------    ----------- --------------------------
// CLOSE       0          0        -                 0           -               -
// CLONE       0          0        -                 objtype     -               handle(s)
// OPEN        flags      mode     <name>            objtype     -               handle(s)
// READ        maxread    0        -                 newoffset   <bytes>         -
// READ_AT     maxread    offset   -                 0           <bytes>         -
// WRITE       0          0        <bytes>           newoffset   -               -
// WRITE_AT    0          offset   <bytes>           0           -               -
// SEEK        whence     offset   -                 offset      -               -
// STAT        maxreply   0        -                 0           <vnattr_t>      -
// READDIR     maxreply   0        -                 0           <vndirent_t[]>  -
// IOCTL       out_len    opcode   <in_bytes>        0           <out_bytes>     -
// UNLINK      0          0        <name>            0           -               -
// TRUNCATE    0          offset   -                 0           -               -
// RENAME      0          0        <name1>0<name2>0  0           -               -
// CONNECT     0          0        <sockaddr>        0           -               -
// BIND        0          0        <sockaddr>        0           -               -
// LISTEN      0          0        <backlog>         0           -               -
// GETSOCKNAME maxreply   0        -                 0           <sockaddr>      -
// GETPEERNAME maxreply   0        -                 0           <sockaddr>      -
// GETSOCKOPT  maxreply   0        <sockopt>         0           <sockopt>       -
// SETSOCKOPT  0          0        <sockopt>         0           <sockopt>       -
// GETADDRINFO maxreply   0        <getaddrinfo>     0           <getaddrinfo>   -
// SETATTR     0          0        <vnattr>          0           -               -
// SYNC        0          0        0                 0           -               -
//
// proposed:
//
// LSTAT       maxreply   0        -                 0           <vnattr_t>      -
// MKDIR       0          0        <name>            0           -               -
// SYMLINK     namelen    0        <name><path>      0           -               -
// READLINK    maxreply   0        -                 0           <path>          -
// MMAP        flags      offset   <uint64:len>      offset      -               vmohandle
// FLUSH       0          0        -                 0           -               -
// LINK*       0          0        <name>            0           -               -
//
// on response arg32 is always mx_status, and may be positive for read/write calls
// * handle[0] used to pass reference to target object

__END_CDECLS
