# mx_object_wait_one

## NAME

object_wait_one - wait for signals on an object

## SYNOPSIS

```
#include <magenta/syscalls.h>

mx_status_t mx_object_wait_one(mx_handle_t handle,
                               mx_signals_t signals,
                               mx_time timeout,
                               mx_signals_t* observed);
```

## DESCRIPTION

**object_wait_one**() is a blocking syscall which causes the caller to
wait until at least one of the specified *signals* has been observed on
the object *handle* refers to or *timeout* elapses.

Upon return, if non-NULL, *observed* is a bitmap of *all* of the
signals which were observed asserted on that object while waiting.

The *observed* signals may not reflect the actual state of the object's
signals if the state of the object was modified by another thread or
process.  (For example, a Channel ceases asserting **MX_CHANNEL_READABLE**
once the last message in its queue is read).

The *timeout* parameter specifies a relative timeout (from now) in nanoseconds,
ranging from **0** (do not wait at all) to **MX_TIME_INFINITE** (wait forever).

## RETURN VALUE

**object_wait_one**() returns **NO_ERROR** if any of *signals* were observed
on the object before *timeout* expires.

In the event of **ERR_TIMED_OUT**, *observed* may reflect state changes
that occurred after the timeout expired, but before the syscall returned.

For any other return value, *observed* is undefined.

## ERRORS

**ERR_INVALID_ARGS**  *pending* is an invalid pointer.

**ERR_BAD_HANDLE**  *handle* is not a valid handle.

**ERR_ACCESS_DENIED**  *handle* does not have **MX_RIGHT_READ** and may
not be waited upon.

**ERR_HANDLE_CLOSED**  *handle* was invalidated (e.g., closed) during the wait.

**ERR_TIMED_OUT**  The specified timeout elapsed (or was 0 to begin
with) before any of the specified *signals* are observed on
*handle*.

**ERR_NOT_SUPPORTED**  *handle* is a handle that cannot be waited on
(for example, a Port handle).

## BUGS

Currently *timeout* is rounded down to the nearest millisecond interval.
