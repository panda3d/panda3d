// Filename: pprerror.cxx
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "pprerror.h"
#include "config_net.h"

#include <prerror.h>
#include <prprf.h>
#include <prmem.h>

#include <stdarg.h>
#include <stdio.h>

static const char *get_error_message(PRErrorCode code);

////////////////////////////////////////////////////////////////////
//     Function: pprerror
//  Description: A handy function like perror().  It outputs the
//               printf-style format string and its arguments,
//               followed by a colon and the NSPR-given description of
//               its current error state.
////////////////////////////////////////////////////////////////////
void
pprerror(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  char *str = PR_vsprintf_append(NULL, format, ap);
  if (str == (char *)NULL) {
    net_cat.error()
      << string("Invalid format string: ") + format + "\n";
    return;
  }

  PRErrorCode code = PR_GetError();
  const char *error_message = get_error_message(code);

  if (error_message == (const char *)NULL) {
    str = PR_sprintf_append(str, ": (%d)\n", code);
  } else {
    str = PR_sprintf_append(str, ": %s (%d)\n", error_message, code);
  }

  net_cat.error() << str;

  PR_Free(str);
  if (get_net_error_abort()) {
    abort();
  }
  va_end(ap);
}

////////////////////////////////////////////////////////////////////
//     Function: get_error_message
//  Description: A local function to this module, it returns a
//               sensibly formatted string to describe the given NSPR
//               error code.  The strings in this function were
//               extracted from the NSPR documentation web site.
////////////////////////////////////////////////////////////////////
static const char *
get_error_message(PRErrorCode code) {
  switch (code) {
  case PR_OUT_OF_MEMORY_ERROR:
    return "PR_OUT_OF_MEMORY_ERROR--Insufficient memory to perform request.";
  case PR_BAD_DESCRIPTOR_ERROR:
    return "PR_BAD_DESCRIPTOR_ERROR--The file descriptor used as an argument in the preceding function is invalid.";
  case PR_WOULD_BLOCK_ERROR:
    return "PR_WOULD_BLOCK_ERROR--The operation would have blocked, which conflicts with the semantics that have been established.";
  case PR_ACCESS_FAULT_ERROR:
    return "PR_ACCESS_FAULT_ERROR--One of the arguments of the preceding function specified an invalid memory address.";
  case PR_INVALID_METHOD_ERROR:
    return "PR_INVALID_METHOD_ERROR--The preceding function is invalid for the type of file descriptor used.";
  case PR_ILLEGAL_ACCESS_ERROR:
    return "PR_ILLEGAL_ACCESS_ERROR--One of the arguments of the preceding function specified an invalid memory address.";
  case PR_UNKNOWN_ERROR:
    return "PR_UNKNOWN_ERROR--Some unknown error has occurred.";
  case PR_PENDING_INTERRUPT_ERROR:
    return "PR_PENDING_INTERRUPT_ERROR--The operation terminated because another thread has interrupted it with PR_Interrupt.";
  case PR_NOT_IMPLEMENTED_ERROR:
    return "PR_NOT_IMPLEMENTED_ERROR--The preceding function has not been implemented.";
  case PR_IO_ERROR:
    return "PR_IO_ERROR--The preceding I/O function encountered some sort of an error, perhaps an invalid device.";
  case PR_IO_TIMEOUT_ERROR:
    return "PR_IO_TIMEOUT_ERROR--The I/O operation has not completed in the time specified for the preceding function.";
  case PR_IO_PENDING_ERROR:
    return "PR_IO_PENDING_ERROR--An I/O operation has been attempted on a file descriptor that is currently busy with another operation.";
  case PR_DIRECTORY_OPEN_ERROR:
    return "PR_DIRECTORY_OPEN_ERROR--The directory could not be opened.";
  case PR_INVALID_ARGUMENT_ERROR:
    return "PR_INVALID_ARGUMENT_ERROR--One or more of the arguments to the function is invalid.";
  case PR_ADDRESS_NOT_AVAILABLE_ERROR:
    return "PR_ADDRESS_NOT_AVAILABLE_ERROR--The network address (PRNetAddr) is not available (probably in use).";
  case PR_ADDRESS_NOT_SUPPORTED_ERROR:
    return "PR_ADDRESS_NOT_SUPPORTED_ERROR--The type of network address specified is not supported.";
  case PR_IS_CONNECTED_ERROR:
    return "PR_IS_CONNECTED_ERROR--An attempt to connect on an already connected network file descriptor.";
  case PR_BAD_ADDRESS_ERROR:
    return "PR_BAD_ADDRESS_ERROR--The network address specified is invalid (as reported by the network).";
  case PR_ADDRESS_IN_USE_ERROR:
    return "PR_ADDRESS_IN_USE_ERROR--Network address specified (PRNetAddr) is in use.";
  case PR_CONNECT_REFUSED_ERROR:
    return "PR_CONNECT_REFUSED_ERROR--The peer has refused to allow the connection to be established.";
  case PR_NETWORK_UNREACHABLE_ERROR:
    return "PR_NETWORK_UNREACHABLE_ERROR--The network address specifies a host that is unreachable (perhaps temporary).";
  case PR_CONNECT_TIMEOUT_ERROR:
    return "PR_CONNECT_TIMEOUT_ERROR--The connection attempt did not complete in a reasonable period of time.";
  case PR_NOT_CONNECTED_ERROR:
    return "PR_NOT_CONNECTED_ERROR--The preceding function attempted to use connected semantics on a network file descriptor that was not connected.";
  case PR_LOAD_LIBRARY_ERROR:
    return "PR_LOAD_LIBRARY_ERROR--Failure to load a dynamic library.";
  case PR_UNLOAD_LIBRARY_ERROR:
    return "PR_UNLOAD_LIBRARY_ERROR--Failure to unload a dynamic library.";
  case PR_FIND_SYMBOL_ERROR:
    return "PR_FIND_SYMBOL_ERROR--Symbol could not be found in the specified library.";
  case PR_INSUFFICIENT_RESOURCES_ERROR:
    return "PR_INSUFFICIENT_RESOURCES_ERROR--There are insufficient system resources to process the request.";
  case PR_DIRECTORY_LOOKUP_ERROR:
    return "PR_DIRECTORY_LOOKUP_ERROR--A directory lookup on a network address has failed.";
  case PR_TPD_RANGE_ERROR:
    return "PR_TPD_RANGE_ERROR--Attempt to access a thread-private data index that is out of range of any index that has been allocated to the process.";
  case PR_PROC_DESC_TABLE_FULL_ERROR:
    return "PR_PROC_DESC_TABLE_FULL_ERROR--The process' table for holding open file descriptors is full.";
  case PR_SYS_DESC_TABLE_FULL_ERROR:
    return "PR_SYS_DESC_TABLE_FULL_ERROR--The system's table for holding open file descriptors has been exceeded.";
  case PR_NOT_SOCKET_ERROR:
    return "PR_NOT_SOCKET_ERROR--An attempt to use a non-network file descriptor on a network-only operation.";
  case PR_NOT_TCP_SOCKET_ERROR:
    return "PR_NOT_TCP_SOCKET_ERROR--Attempt to perform a TCP specific function on a non-TCP file descriptor.";
  case PR_SOCKET_ADDRESS_IS_BOUND_ERROR:
    return "PR_SOCKET_ADDRESS_IS_BOUND_ERROR--Attempt to bind an address to a TCP file descriptor that is already bound.";
  case PR_NO_ACCESS_RIGHTS_ERROR:
    return "PR_NO_ACCESS_RIGHTS_ERROR--Calling thread does not have privilege to perform the operation requested.";
  case PR_OPERATION_NOT_SUPPORTED_ERROR:
    return "PR_OPERATION_NOT_SUPPORTED_ERROR--The requested operation is not supported by the platform.";
  case PR_PROTOCOL_NOT_SUPPORTED_ERROR:
    return "PR_PROTOCOL_NOT_SUPPORTED_ERROR--The host operating system does not support the protocol requested.";
  case PR_REMOTE_FILE_ERROR:
    return "PR_REMOTE_FILE_ERROR--Access to the remote file has been severed.";
  case PR_BUFFER_OVERFLOW_ERROR:
    return "PR_BUFFER_OVERFLOW_ERROR--The value retrieved is too large to be stored in the buffer provided.";
  case PR_CONNECT_RESET_ERROR:
    return "PR_CONNECT_RESET_ERROR--The (TCP) connection has been reset by the peer.";
  case PR_RANGE_ERROR:
    return "PR_RANGE_ERROR--Unused.";
  case PR_DEADLOCK_ERROR:
    return "PR_DEADLOCK_ERROR--Performing the requested operation would have caused a deadlock. The deadlock was avoided.";
  case PR_FILE_IS_LOCKED_ERROR:
    return "PR_FILE_IS_LOCKED_ERROR--An attempt to acquire a lock on a file has failed because the file is already locked.";
  case PR_FILE_TOO_BIG_ERROR:
    return "PR_FILE_TOO_BIG_ERROR--Completing the write or seek operation would have resulted in a file larger than the system could handle.";
  case PR_NO_DEVICE_SPACE_ERROR:
    return "PR_NO_DEVICE_SPACE_ERROR--The device for storing the file is full.";
  case PR_PIPE_ERROR:
    return "PR_PIPE_ERROR--Unused.";
  case PR_NO_SEEK_DEVICE_ERROR:
    return "PR_NO_SEEK_DEVICE_ERROR--Unused.";
  case PR_IS_DIRECTORY_ERROR:
    return "PR_IS_DIRECTORY_ERROR--Attempt to perform a normal file operation on a directory.";
  case PR_LOOP_ERROR:
    return "PR_LOOP_ERROR--Symbolic link loop.";
  case PR_NAME_TOO_LONG_ERROR:
    return "PR_NAME_TOO_LONG_ERROR--Filename is longer than allowed by the host operating system.";
  case PR_FILE_NOT_FOUND_ERROR:
    return "PR_FILE_NOT_FOUND_ERROR--The requested file was not found.";
  case PR_NOT_DIRECTORY_ERROR:
    return "PR_NOT_DIRECTORY_ERROR--Attempt to perform directory specific operations on a normal file.";
  case PR_READ_ONLY_FILESYSTEM_ERROR:
    return "PR_READ_ONLY_FILESYSTEM_ERROR--Attempt to write to a read-only file system.";
  case PR_DIRECTORY_NOT_EMPTY_ERROR:
    return "PR_DIRECTORY_NOT_EMPTY_ERROR--Attempt to delete a directory that is not empty.";
  case PR_FILESYSTEM_MOUNTED_ERROR:
    return "PR_FILESYSTEM_MOUNTED_ERROR--Attempt to delete or rename a file object while the file system is busy.";
  case PR_NOT_SAME_DEVICE_ERROR:
    return "PR_NOT_SAME_DEVICE_ERROR--Request to rename a file to a file system on another device.";
  case PR_DIRECTORY_CORRUPTED_ERROR:
    return "PR_DIRECTORY_CORRUPTED_ERROR--The directory object in the file system is corrupted.";
  case PR_FILE_EXISTS_ERROR:
    return "PR_FILE_EXISTS_ERROR--Attempt to create or rename a file when the new name is already being used.";
  case PR_MAX_DIRECTORY_ENTRIES_ERROR:
    return "PR_MAX_DIRECTORY_ENTRIES_ERROR--Attempt to add new filename to directory would exceed the limit allowed.";
  case PR_INVALID_DEVICE_STATE_ERROR:
    return "PR_INVALID_DEVICE_STATE_ERROR--The device was in an invalid state to complete the desired operation.";
  case PR_DEVICE_IS_LOCKED_ERROR:
    return "PR_DEVICE_IS_LOCKED_ERROR--The device needed to perform the desired request is locked.";
  case PR_NO_MORE_FILES_ERROR:
    return "PR_NO_MORE_FILES_ERROR--There are no more entries in the directory.";
  case PR_END_OF_FILE_ERROR:
    return "PR_END_OF_FILE_ERROR--Unexpectedly encountered end of file (Mac OS only).";
  case PR_FILE_SEEK_ERROR:
    return "PR_FILE_SEEK_ERROR--An unexpected seek error (Mac OS only).";
  case PR_FILE_IS_BUSY_ERROR:
    return "PR_FILE_IS_BUSY_ERROR--The file is busy and the operation cannot be performed (Mac OS only).";
  case PR_IN_PROGRESS_ERROR:
    return "PR_IN_PROGRESS_ERROR--The operation is still in progress (probably a nonblocking connect).";
  case PR_ALREADY_INITIATED_ERROR:
    return "PR_ALREADY_INITIATED_ERROR--The (retried) operation has already been initiated (probably a nonblocking connect).";
  case PR_GROUP_EMPTY_ERROR:
    return "PR_GROUP_EMPTY_ERROR--The wait group is empty.";
  case PR_INVALID_STATE_ERROR:
    return "PR_INVALID_STATE_ERROR--The attempted operation is on an object that was in an improper state to perform the request.";

    // These were added with NSPR 4.0.
#ifdef PR_NETWORK_DOWN_ERROR
  case PR_NETWORK_DOWN_ERROR:
    return "PR_NETWORK_DOWN_ERROR--The network is down.";

  case PR_SOCKET_SHUTDOWN_ERROR:
    return "PR_SOCKET_SHUTDOWN_ERROR--The socket has been shut down.";

  case PR_CONNECT_ABORTED_ERROR:
    return "PR_CONNECT_ABORTED_ERROR--The connection has been aborted.";

  case PR_HOST_UNREACHABLE_ERROR:
    return "PR_HOST_UNREACHABLE_ERROR--The host is unreachable.";
#endif

  case PR_MAX_ERROR:
    return "PR_MAX_ERROR--Placeholder for the end of the list.";
  }

  return (const char *)NULL;
}
