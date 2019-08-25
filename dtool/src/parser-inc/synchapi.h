#ifndef SYNCHAPI_H
#define SYNCHAPI_H

#include "winnt.h"

#define SRWLOCK_INIT RTL_SRWLOCK_INIT

typedef RTL_SRWLOCK SRWLOCK, *PSRWLOCK;

#endif
