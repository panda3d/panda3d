// Filename: error_utils.cxx
// Created by:  mike (07Nov00)
//
////////////////////////////////////////////////////////////////////

#include "error_utils.h"
#include "config_express.h"

#include <errno.h>
#include <stdio.h>
#if defined(WIN32_VC)
  #include <winsock.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: handle_socket_error
//  Description:
////////////////////////////////////////////////////////////////////
const char*
handle_socket_error(void) {
#ifndef WIN32
  return strerror(errno);
#else
  int err = WSAGetLastError();
  switch (err) {
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
    case 0:
      return strerror(errno);
    default:
      if (express_cat.is_debug())
        express_cat.debug()
	  << "handle_socket_error - unknown error: " << err << endl;
      return "Unknown WSA error";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: error_to_text
//  Description:
////////////////////////////////////////////////////////////////////
const char*
error_to_text(int err) {
  switch (err) {
    case EU_eof:
      return "EU_eof";
    case EU_network_no_data:
      return "EU_network_no_data";
    case EU_ok:
      return "EU_ok";
    case EU_write:
      return "EU_write";
    case EU_success:
      return "EU_success";
    case EU_error_abort:
      return "EU_error_abort";
    case EU_error_file_empty:
      return "EU_error_file_empty";
    case EU_error_file_invalid:
      return "EU_error_file_invalid";
    case EU_error_invalid_checksum:
      return "EU_error_invalid_checksum";
    case EU_error_network_dead:
      return "EU_error_network_dead";
    case EU_error_network_unreachable:
      return "EU_error_network_unreachable";
    case EU_error_network_disconnected:
      return "EU_error_network_disconnected";
    case EU_error_network_timeout:
      return "EU_error_network_timeout";
    case EU_error_network_no_data:
      return "EU_error_network_no_data";
    case EU_error_network_disconnected_locally:
      return "EU_error_network_disconnected_locally";
    case EU_error_network_buffer_overflow:
      return "EU_error_network_buffer_overflow";
    case EU_error_network_disk_quota_exceeded:
      return "EU_error_network_disk_quota_exceeded";
    case EU_error_network_remote_host_disconnected:
      return "EU_error_network_remote_host_disconnected";
    case EU_error_network_remote_host_down:
      return "EU_error_network_remote_host_down";
    case EU_error_network_remote_host_unreachable:
      return "EU_error_network_remote_host_unreachable";
    case EU_error_network_remote_host_not_found:
      return "EU_error_network_remote_host_not_found";
    case EU_error_network_remote_host_no_response:
      return "EU_error_network_remote_host_no_response";
    case EU_error_write_out_of_files:
      return "EU_error_write_out_of_files";
    case EU_error_write_out_of_memory:
      return "EU_error_write_out_of_memory";
    case EU_error_write_sharing_violation:
      return "EU_error_write_sharing_violation";
    case EU_error_write_disk_full:
      return "EU_error_write_disk_full";
    case EU_error_write_disk_not_found:
      return "EU_error_write_disk_not_found";
    case EU_error_write_disk_sector_not_found:
      return "EU_error_write_disk_sector_not_found";
    case EU_error_write_disk_fault:
      return "EU_error_write_disk_fault";
    case EU_error_write_file_rename:
      return "EU_error_write_file_rename";
    case EU_error_http_server_timeout:
      return "EU_error_http_server_timeout";
    case EU_error_http_gateway_timeout:
      return "EU_error_http_gateway_timeout";
    case EU_error_http_service_unavailable:
      return "EU_error_http_service_unavailable";
    case EU_error_zlib:
      return "EU_error_zlib";
    default:
      return "Unknown error";
  }
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
  int err = WSAGetLastError();
  switch (err) {
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
    case 0:
      if (express_cat.is_debug())
	express_cat.debug()
	  << "get_network_error() - WSA error = 0 - error : "
	  << strerror(errno) << endl;
      return EU_error_abort;
    default:
      if (express_cat.is_debug())
	express_cat.debug()
	  << "get_network_error() - unknown error: " << err << endl;
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
