// Filename: ipc_traits.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_TRAITS_H__
#define __IPC_TRAITS_H__

#include <pandabase.h>
#include <string>

// decide which IPC implementation to use.  This is so much fun, I just can't
// hardly stand it.

// Flavor tokens:
//    0 - mach
//    1 - nt
//    2 - posix
//    3 - solaris
//    4 - NSPR
//    5 - PS2


// I threw PS2 in first because sometimes other env vars are set, but if PENV is PS2,
// we're NEVER building anything else... CSN

#if defined(PENV_PS2)

#define __IPC_FLAVOR_DUJOUR__ PS2
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 5

#elif defined(HAVE_NSPR)

#define __IPC_FLAVOR_DUJOUR__ nspr
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 4

#elif defined(__arm__) && defined(__atmos__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__alpha__) && defined(__osf1__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__powerpc__) && defined(__aix__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__hpux__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(WIN32_VC)

#define __IPC_FLAVOR_DUJOUR__ nt
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 1

#elif defined(__sun__)

#ifdef UsePthreads
#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2
#else
#define __IPC_FLAVOR_DUJOUR__ solaris
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 3
#endif /* UsePthreads */

#elif defined(__linux__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__nextstep__)

#define __IPC_FLAVOR_DUJOUR__ mach
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 0

#elif defined(__VMS)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__SINIX__)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#elif defined(__sgi)

#define __IPC_FLAVOR_DUJOUR__ posix
#define __IPC_FLAVOR_DUJOUR_TOKEN__ 2

#else

#error "Unable to determine which IPC implementation to use"
#endif /* OS identification */

// this exception is a general fatal exception from the IPC code
class EXPCL_PANDAEXPRESS ipc_fatal {
   public:
      int error;
      INLINE ipc_fatal(const int e = 0) : error(e) {}
};

// this exception is a general invalid value/function call exception for the
// IPC code
class EXPCL_PANDAEXPRESS ipc_invalid {};

// this exception is thrown in the event of a fatal error in the mutex code
class EXPCL_PANDAEXPRESS mutex_fatal : public ipc_fatal {
   public:
      INLINE mutex_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when a mutex is called in an invalid state, or the
// arguments to the call are invalid
class EXPCL_PANDAEXPRESS mutex_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the condition
// variable code
class EXPCL_PANDAEXPRESS condition_fatal : public ipc_fatal {
   public:
      INLINE condition_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when a condition variable is called in an
// invalid state, or the arguments to the call are invalid
class EXPCL_PANDAEXPRESS condition_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the semaphore code
class EXPCL_PANDAEXPRESS semaphore_fatal : public ipc_fatal {
   public:
      INLINE semaphore_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when a semaphore is called in an invalid state, or
// the arguments to the call are invalid
class EXPCL_PANDAEXPRESS semaphore_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the thread code
class EXPCL_PANDAEXPRESS thread_fatal : public ipc_fatal {
   public:
      INLINE thread_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when a thread is called in an invalid state, or
// the arguments to the call are invalid
class EXPCL_PANDAEXPRESS thread_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the atomic set
// code
class EXPCL_PANDAEXPRESS set_atomic_fatal : public ipc_fatal {
public:
  INLINE set_atomic_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when an atomic set is called in an invalid state,
// or the arguments to the call are invalid
class EXPCL_PANDAEXPRESS set_atomic_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the atomic
// increment code
class EXPCL_PANDAEXPRESS inc_atomic_fatal : public ipc_fatal {
public:
  INLINE inc_atomic_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when an atomic increment is called in an invalid
// state, or the arguments to the call are invalid
class EXPCL_PANDAEXPRESS inc_atomic_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the atomic
// decrement code
class EXPCL_PANDAEXPRESS dec_atomic_fatal : public ipc_fatal {
public:
  INLINE dec_atomic_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when an atomic decrement is called in an invalid
// state, or the arguments to the call are invalid
class EXPCL_PANDAEXPRESS dec_atomic_invalid : public ipc_invalid {};

// this exception is thrown in the event of a fatal error in the library load
// code
class EXPCL_PANDAEXPRESS lib_load_fatal : public ipc_fatal {
public:
  INLINE lib_load_fatal(const int e = 0) : ipc_fatal(e) {}
};

// this exception is thrown when a library load is called in an invalid state,
// or the arguments to the call are invalid
class EXPCL_PANDAEXPRESS lib_load_invalid : public ipc_invalid {};

#include <dconfig.h>

#if (__IPC_FLAVOR_DUJOUR_TOKEN__ == 0)
#include "ipc_mach_traits.h"
#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 1)
#include "ipc_nt_traits.h"
#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 2)
#include "ipc_posix_traits.h"
#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 3)
#include "ipc_solaris_traits.h"
#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 4)
#include "ipc_nspr_traits.h"
#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 5)
#include "ipc_ps2_traits.h"
#else
#error "invalid ipc flavor token"
#endif /* loading trait specializations */

#endif /* __IPC_TRAITS_H__ */
