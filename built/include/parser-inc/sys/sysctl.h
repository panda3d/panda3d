/**
 * Stub header for sys/sysctl.h in macOS and FreeBSD.
 */

#pragma once

struct ctlname {
  char *ctl_name;
  int ctl_type;
};

struct sysctl_req;
struct sysctl_oid;
struct sysctl_ctx_entry;
struct sysctl_ctx_list;
