// Filename: error_utils.cxx
// Created by:  mike (07Nov00)
//
////////////////////////////////////////////////////////////////////

#include "error_utils.h"

#include <errno.h>
#if defined(WIN32_VC)
  #include <winsock2.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: handle_socket_error
//  Description:
////////////////////////////////////////////////////////////////////
char*
handle_socket_error(void) {
#ifndef WIN32
  return strerror(errno);
#else
  switch (WSAGetLastError()) {
    case 10022:
      return "An invalid argument was supplied";
    case 10036:
      return "A blocking operation is currently executing";
    case 10040:
      return "Message was larger than internal buffer or network limit";
    case 10050:
      return "Network dead";
    case 10051:
      return "Network unreachable";
    case 10052:
      return "Connection broken because keep-alive detected a failure";
    case 10053:
      return "Connection aborted by local host software";
    case 10054:
      return "Connection closed by remote host";
    case 10055:
      return "Out of buffer space or queue overflowed";
    case 10057:
      return "Socket was not connected";
    case 10058:
      return "Socket was previously shut down";
    case 10060:
      return "Connection timed out";
    case 10061:
      return "Connection refused by remote host";
    case 10064:
      return "Remote host is down";
    case 10065:
      return "Remote host is unreachable";
    case 10093:
      return "WSAStartup() was not called";
    default:
      return "Unknown WSA error";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: get_network_error
//  Description:
////////////////////////////////////////////////////////////////////
int
get_network_error(void) {
#ifndef WIN32
  return EU_error_abort;
#else
  switch (WSAGetLastError()) {
    case 10050:
      return EU_error_network_dead;
    case 10051:
      return EU_error_network_unreachable;
    case 10052:
    case 10057:
    case 10058:
      return EU_error_network_disconnected;
    case 10053:
      return EU_error_network_disconnected_locally;
    case 10054:
    case 10061:
      return EU_error_network_remote_host_disconnected;
    case 10055:
      return EU_error_network_buffer_overflow;
    case 10060:
      return EU_error_network_timeout;
    case 10064:
      return EU_error_network_remote_host_down;
    case 10065:
      return EU_error_network_remote_host_unreachable;
    case 10069:
      return EU_error_network_disk_quota_exceeded;
    case 11001:
      return EU_error_network_remote_host_not_found;
    case 11002:
      return EU_error_network_remote_host_no_response;
    default:
      return EU_error_abort;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: get_write_error
//  Description:
////////////////////////////////////////////////////////////////////
int
get_write_error(void) {
#ifndef WIN32
  return EU_error_abort;
#else
  switch (errno) {
    case 4:
    case 18:
      return EU_error_write_out_of_files;
    case 8:
    case 14:
      return EU_error_write_out_of_memory;
    case 20:
      return EU_error_write_disk_not_found;
    case 25:
    case 27:
      return EU_error_write_disk_sector_not_found;
    case 29:
    case 30:
    case 31:
      return EU_error_write_disk_fault;
    case 32:
    case 33:
    case 36:
      return EU_error_write_sharing_violation;
    case 39:
      return EU_error_write_disk_full;
    default:
      return EU_error_abort;
  }
#endif
}
