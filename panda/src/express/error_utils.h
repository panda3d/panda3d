// Filename: error_utils.h
// Created by:  mike (07Nov00)
//
////////////////////////////////////////////////////////////////////

#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

#include <pandabase.h>
#include "typedef.h"

BEGIN_PUBLISH

enum ErrorUtilCode {
  EU_eof = 5,
  EU_network_no_data = 4,

  EU_ok = 3,
  EU_write = 2,
  EU_success = 1,

  // General errors
  EU_error_abort = -1,
  EU_error_file_empty = -2,
  EU_error_file_invalid = -3,

  // General network errors
  EU_error_network_dead = -30,
  EU_error_network_unreachable = -31,
  EU_error_network_disconnected = -32,
  EU_error_network_timeout = -33,
  EU_error_network_no_data = -34,

  // Local network errors
  EU_error_network_disconnected_locally = -40,
  EU_error_network_buffer_overflow = -41,
  EU_error_network_disk_quota_exceeded = -42,

  // Remote host network errors
  EU_error_network_remote_host_disconnected = -50,
  EU_error_network_remote_host_down = -51,
  EU_error_network_remote_host_unreachable = -52,
  EU_error_network_remote_host_not_found = -53,
  EU_error_network_remote_host_no_response = -54,

  // General local errors
  EU_error_write_out_of_files = -60,
  EU_error_write_out_of_memory = -61,
  EU_error_write_sharing_violation = -62,
  EU_error_write_disk_full = -63,
  EU_error_write_disk_not_found = -64,
  EU_error_write_disk_sector_not_found = -65,
  EU_error_write_disk_fault = -66,
  EU_error_write_file_rename = -67,

  // HTTP errors
  EU_error_http_server_timeout = -70,
  EU_error_http_gateway_timeout = -71,
  EU_error_http_service_unavailable = -72,

  // Zlib errors
  EU_error_zlib = -80,
};

EXPCL_PANDAEXPRESS const char *handle_socket_error(void);
EXPCL_PANDAEXPRESS const char *error_to_text(int err);
EXPCL_PANDAEXPRESS int get_network_error(void);
EXPCL_PANDAEXPRESS int get_write_error(void);

END_PUBLISH

#endif

