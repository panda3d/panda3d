#pragma once

#include <stdtypedefs.h>

struct timespec {
  time_t tv_sec;
  long tv_nsec;
};
