/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file error_utils.cxx
 * @author mike
 * @date 2000-11-07
 */

#include "error_utils.h"
#include "config_express.h"

#include <errno.h>
#include <stdio.h>

#if defined(WIN32_VC) || defined(WIN64_VC)
  #include <winsock2.h>
#endif

using std::string;

/**
 *
 */
string error_to_text(ErrorUtilCode err) {
  const char *errmsg = "Unknown error";

  switch (err) {
  case EU_http_redirect:
    errmsg = "EU_http_redirect";
    break;
  case EU_eof:
    errmsg = "EU_eof";
    break;
  case EU_network_no_data:
    errmsg = "EU_network_no_data";
    break;
  case EU_ok:
    errmsg = "EU_ok";
    break;
  case EU_write:
    errmsg = "EU_write";
    break;
  case EU_success:
    errmsg = "EU_success";
    break;
  case EU_error_abort:
    errmsg = "EU_error_abort";
    break;
  case EU_error_file_empty:
    errmsg = "EU_error_file_empty";
    break;
  case EU_error_file_invalid:
    errmsg = "EU_error_file_invalid";
    break;
  case EU_error_invalid_checksum:
    errmsg = "EU_error_invalid_checksum";
    break;
  case EU_error_network_dead:
    errmsg = "EU_error_network_dead";
    break;
  case EU_error_network_unreachable:
    errmsg = "EU_error_network_unreachable";
    break;
  case EU_error_network_disconnected:
    errmsg = "EU_error_network_disconnected";
    break;
  case EU_error_network_timeout:
    errmsg = "EU_error_network_timeout";
    break;
  case EU_error_network_no_data:
    errmsg = "EU_error_network_no_data";
    break;
  case EU_error_network_disconnected_locally:
    errmsg = "EU_error_network_disconnected_locally";
    break;
  case EU_error_network_buffer_overflow:
    errmsg = "EU_error_network_buffer_overflow";
    break;
  case EU_error_network_disk_quota_exceeded:
    errmsg = "EU_error_network_disk_quota_exceeded";
    break;
  case EU_error_network_remote_host_disconnected:
    errmsg = "EU_error_network_remote_host_disconnected";
    break;
  case EU_error_network_remote_host_down:
    errmsg = "EU_error_network_remote_host_down";
    break;
  case EU_error_network_remote_host_unreachable:
    errmsg = "EU_error_network_remote_host_unreachable";
    break;
  case EU_error_network_remote_host_not_found:
    errmsg = "EU_error_network_remote_host_not_found";
    break;
  case EU_error_network_remote_host_no_response:
    errmsg = "EU_error_network_remote_host_no_response";
    break;
  case EU_error_write_out_of_files:
    errmsg = "EU_error_write_out_of_files";
    break;
  case EU_error_write_out_of_memory:
    errmsg = "EU_error_write_out_of_memory";
    break;
  case EU_error_write_sharing_violation:
    errmsg = "EU_error_write_sharing_violation";
    break;
  case EU_error_write_disk_full:
    errmsg = "EU_error_write_disk_full";
    break;
  case EU_error_write_disk_not_found:
    errmsg = "EU_error_write_disk_not_found";
    break;
  case EU_error_write_disk_sector_not_found:
    errmsg = "EU_error_write_disk_sector_not_found";
    break;
  case EU_error_write_disk_fault:
    errmsg = "EU_error_write_disk_fault";
    break;
  case EU_error_write_file_rename:
    errmsg = "EU_error_write_file_rename";
    break;
  case EU_error_http_server_timeout:
    errmsg = "EU_error_http_server_timeout";
    break;
  case EU_error_http_gateway_timeout:
    errmsg = "EU_error_http_gateway_timeout";
    break;
  case EU_error_http_service_unavailable:
    errmsg = "EU_error_http_service_unavailable";
    break;
  case EU_error_http_proxy_authentication:
    errmsg = "EU_error_http_proxy_authentication";
    break;
  case EU_error_zlib:
    errmsg = "EU_error_zlib";
    break;
  case EU_write_ram:
    errmsg = "EU_write_ram";
    break;
  }

  char msgbuf[1024];
  sprintf(msgbuf,"%d: %s",err,errmsg);
  return string(msgbuf);
}

/**
 *
 */
int
get_write_error() {
#if !defined(WIN32) && !defined(WIN64)
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

#ifdef HAVE_NET
/**
 *
 */
string handle_socket_error() {
#if !defined(WIN32) && !defined(WIN64)
  return string(strerror(errno));
#else
  int err = WSAGetLastError();
  const char *errmsg;
  switch (err) {
    case 10022:
      errmsg =  "An invalid argument was supplied";
    case 10036:
      errmsg =  "A blocking operation is currently executing";
    case 10040:
      errmsg =  "Message was larger than internal buffer or network limit";
    case 10050:
      errmsg =  "Network dead";
    case 10051:
      errmsg =  "Network unreachable";
    case 10052:
      errmsg =  "Connection broken because keep-alive detected a failure";
    case 10053:
      errmsg =  "Connection aborted by local host software";
    case 10054:
      errmsg =  "Connection closed by remote host";
    case 10055:
      errmsg =  "Out of buffer space or queue overflowed";
    case 10057:
      errmsg =  "Socket was not connected";
    case 10058:
      errmsg =  "Socket was previously shut down";
    case 10060:
      errmsg =  "Connection timed out";
    case 10061:
      errmsg =  "Connection refused by remote host";
    case 10064:
      errmsg =  "Remote host is down";
    case 10065:
      errmsg =  "Remote host is unreachable";
    case 10093:
      errmsg =  "WSAStartup() was not called";
    case 0:
      errmsg = strerror(errno);
    default:
      if (express_cat.is_debug())
        express_cat.debug() << "handle_socket_error - unknown error: " << err << std::endl;
      errmsg = "Unknown WSA error";
  }

  char msgbuf[1024];
  sprintf(msgbuf,"WSA errno %d, errno %d: %s",err,errno,errmsg);
  return string(msgbuf);
#endif
}
#endif

#ifdef HAVE_NET
/**
 *
 */
int
get_network_error() {
#if !defined(WIN32) && !defined(WIN64)
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
          << strerror(errno) << std::endl;
      return EU_error_abort;
    default:
      if (express_cat.is_debug())
        express_cat.debug()
          << "get_network_error() - unknown error: " << err << std::endl;
      return EU_error_abort;
  }
#endif
}
#endif
