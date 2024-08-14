#pragma once

#include <stdtypedefs.h>
#include <time.h>

#define SIG_DFL SIG_DFL
#define SIG_ERR SIG_ERR
#define SIG_HOLD SIG_HOLD
#define SIG_IGN SIG_IGN

typedef int pthread_t;
typedef unsigned int uid_t;
typedef int pid_t;

typedef int sig_atomic_t;
typedef unsigned int sigset_t;

union sigval {
  int sival_int;
  void *sival_ptr;
};
