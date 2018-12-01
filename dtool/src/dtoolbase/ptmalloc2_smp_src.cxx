/* drose: Note that this file is released under an unrestricted
   license as well as the LGPL, in spite of the comments below.  See
   http://www.malloc.de . */

#if defined(USE_MEMORY_PTMALLOC2) && !defined(linux)
#define USE_DL_PREFIX 1
#define __STDC__ 1
#define MREMAP_MAYMOVE 1  /* terrible hack--drose */

/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 1996,1997,1998,1999,2000,01,02 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>
   and Doug Lea <dl@cs.oswego.edu>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
  This is a version (aka ptmalloc2) of malloc/free/realloc written by
  Doug Lea and adapted to multiple threads/arenas by Wolfram Gloger.
  Gloger's SMP additions were merged back with ptmalloc2 by Niall Douglas
  plus Win32 support readded.

* Version ptmalloc2-smp-20011215
  $Id$
  based on:
* VERSION 2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)

   Note: There may be an updated version of this malloc obtainable at
           http://www.malloc.de/malloc/ptmalloc2.tar.gz
         Check before installing!

* Quickstart

  In order to compile this implementation, a Makefile is provided with
  the ptmalloc2 distribution, which has pre-defined targets for some
  popular systems (e.g. "make posix" for Posix threads).  All that is
  typically required with regard to compiler flags is the selection of
  the thread package via defining one out of USE_PTHREADS, USE_THR or
  USE_SPROC.  Check the thread-m.h file for what effects this has.
  Many/most systems will additionally require USE_TSD_DATA_HACK to be
  defined, so this is the default for "make posix".

* Why use this malloc?

  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and tunable.
  Consistent balance across these factors results in a good general-purpose
  allocator for malloc-intensive programs.

  The main properties of the algorithms are:
  * For large (>= 512 bytes) requests, it is a pure best-fit allocator,
    with ties normally decided via FIFO (i.e. least recently used).
  * For small (<= 64 bytes by default) requests, it is a caching
    allocator, that maintains pools of quickly recycled chunks.
  * In between, and for combinations of large and small requests, it does
    the best it can trying to meet both goals at once.
  * For very large requests (>= 128KB by default), it relies on system
    memory mapping facilities, if supported.

  For a longer but slightly out of date high-level description, see
     http://gee.cs.oswego.edu/dl/html/malloc.html

  You may already by default be using a C library containing a malloc
  that is  based on some version of this malloc (for example in
  linux). You might still want to use the one in this file in order to
  customize settings or to avoid overheads associated with library
  versions.

* Contents, described in more detail in "description of public routines" below.

  Standard (ANSI/SVID/...)  functions:
    malloc(size_t n);
    calloc(size_t n_elements, size_t element_size);
    free(Void_t* p);
    realloc(Void_t* p, size_t n);
    memalign(size_t alignment, size_t n);
    valloc(size_t n);
    mallinfo()
    mallopt(int parameter_number, int parameter_value)

  Additional functions:
    independent_calloc(size_t n_elements, size_t size, Void_t* chunks[]);
    independent_comalloc(size_t n_elements, size_t sizes[], Void_t* chunks[]);
    pvalloc(size_t n);
    cfree(Void_t* p);
    malloc_trim(size_t pad);
    malloc_usable_size(Void_t* p);
    malloc_stats();

* Vital statistics:

  Supported pointer representation:       4 or 8 bytes
  Supported size_t  representation:       4 or 8 bytes 
       Note that size_t is allowed to be 4 bytes even if pointers are 8.
       You can adjust this by defining INTERNAL_SIZE_T

  Alignment:                              2 * sizeof(size_t) (default)
       (i.e., 8 byte alignment with 4byte size_t). This suffices for
       nearly all current machines and C compilers. However, you can
       define MALLOC_ALIGNMENT to be wider than this if necessary.

  Minimum overhead per allocated chunk:   4 or 8 bytes
       Each malloced chunk has a hidden word of overhead holding size
       and status information.

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including 4 overhead)
                          8-byte ptrs:  24/32 bytes (including, 4/8 overhead)

       When a chunk is freed, 12 (for 4byte ptrs) or 20 (for 8 byte
       ptrs but 4 byte size) or 24 (for 8/8) additional bytes are
       needed; 4 (8) for a trailing size field and 8 (16) bytes for
       free list pointers. Thus, the minimum allocatable size is
       16/24/32 bytes.

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.

       The maximum overhead wastage (i.e., number of extra bytes
       allocated than were requested in malloc) is less than or equal
       to the minimum size, except for requests >= mmap_threshold that
       are serviced via mmap(), where the worst case wastage is 2 *
       sizeof(size_t) bytes plus the remainder from a system page (the
       minimal mmap unit); typically 4096 or 8192 bytes.

  Maximum allocated size:  4-byte size_t: 2^32 minus about two pages 
                           8-byte size_t: 2^64 minus about two pages

       It is assumed that (possibly signed) size_t values suffice to
       represent chunk sizes. `Possibly signed' is due to the fact
       that `size_t' may be defined on a system as either a signed or
       an unsigned type. The ISO C standard says that it must be
       unsigned, but a few systems are known not to adhere to this.
       Additionally, even when size_t is unsigned, sbrk (which is by
       default used to obtain memory from system) accepts signed
       arguments, and may not be able to handle size_t-wide arguments
       with negative sign bit.  Generally, values that would
       appear as negative after accounting for overhead and alignment
       are supported only via mmap(), which does not have this
       limitation.

       Requests for sizes outside the allowed range will perform an optional
       failure action and then return null. (Requests may also
       also fail because a system is out of memory.)

  Thread-safety: thread-safe unless NO_THREADS is defined

  Compliance: I believe it is compliant with the 1997 Single Unix Specification
       (See http://www.opennc.org). Also SVID/XPG, ANSI C, and probably 
       others as well.

* Synopsis of compile-time options:

    People have reported using previous versions of this malloc on all
    versions of Unix, sometimes by tweaking some of the defines
    below. It has been tested most extensively on Solaris and
    Linux. It is also reported to work on WIN32 platforms.
    People also report using it in stand-alone embedded systems.

    The implementation is in straight, hand-tuned ANSI C.  It is not
    at all modular. (Sorry!)  It uses a lot of macros.  To be at all
    usable, this code should be compiled using an optimizing compiler
    (for example gcc -O3) that can simplify expressions and control
    paths. (FAQ: some macros import variables as arguments rather than
    declare locals because people reported that some debuggers
    otherwise get confused.)

    OPTION                     DEFAULT VALUE

    Compilation Environment options:

    __STD_C                    derived from C compiler defines
    WIN32                      NOT defined
    HAVE_MEMCPY                defined
    USE_MEMCPY                 1 if HAVE_MEMCPY is defined
    HAVE_MMAP                  defined as 1 
    MMAP_CLEARS                1
    HAVE_MREMAP                0 unless linux defined
    USE_ARENAS                 the same as HAVE_MMAP
    malloc_getpagesize         derived from system #includes, or 4096 if not
    HAVE_USR_INCLUDE_MALLOC_H  NOT defined
    LACKS_UNISTD_H             NOT defined unless WIN32
    LACKS_SYS_PARAM_H          NOT defined unless WIN32
    LACKS_SYS_MMAN_H           NOT defined unless WIN32
    LACKS_FCNTL_H              NOT defined

    Changing default word sizes:

    INTERNAL_SIZE_T            size_t
    MALLOC_ALIGNMENT           2 * sizeof(INTERNAL_SIZE_T)
    PTR_UINT                   unsigned long
    CHUNK_SIZE_T               unsigned long

    Configuration and functionality options:

    USE_DL_PREFIX              NOT defined
        PTMALLOC_IN_CPPNAMESPACE   NOT defined
    USE_PUBLIC_MALLOC_WRAPPERS NOT defined
    MALLOC_DEBUG               NOT defined
    REALLOC_ZERO_BYTES_FREES   1
    MALLOC_FAILURE_ACTION      errno = ENOMEM, if __STD_C defined, else no-op
    TRIM_FASTBINS              0
    FIRST_SORTED_BIN_SIZE      512
        MORECORE_IS_MMAP           NOT defined

    Options for customizing MORECORE:

    MORECORE                   sbrk
    MORECORE_FAILURE           -1
    MORECORE_CONTIGUOUS        1
    MORECORE_CANNOT_TRIM       NOT defined
    MORECORE_CLEARS            1
    MMAP_AS_MORECORE_SIZE      (1024 * 1024)

    Tuning options that are also dynamically changeable via mallopt:

    DEFAULT_MXFAST             64
    DEFAULT_TRIM_THRESHOLD     256 * 1024
    DEFAULT_TOP_PAD            0
    DEFAULT_MMAP_THRESHOLD     256 * 1024
    DEFAULT_MMAP_MAX           65536

    There are several other #defined constants and macros that you
    probably don't want to touch unless you are extending or adapting malloc.
*/

/*************************** thread-m.h ******************************/


#if defined(_LIBC) /* The GNU C library, a special case of Posix threads */

#include <bits/libc-lock.h>

#ifdef PTHREAD_MUTEX_INITIALIZER

typedef pthread_t thread_id;

/* mutex */
typedef pthread_mutex_t mutex_t;

#define MUTEX_INITIALIZER       PTHREAD_MUTEX_INITIALIZER

/* Even if not linking with libpthread, ensure usability of mutex as
   an `in use' flag, see also the NO_THREADS case below.  Assume
   pthread_mutex_t is at least one int wide.  */

#define mutex_init(m)           \
   (__pthread_mutex_init != nullptr \
    ? __pthread_mutex_init (m, nullptr) : (*(int *)(m) = 0))
#define mutex_lock(m)           \
   (__pthread_mutex_lock != nullptr \
    ? __pthread_mutex_lock (m) : ((*(int *)(m) = 1), 0))
#define mutex_trylock(m)        \
   (__pthread_mutex_trylock != nullptr \
    ? __pthread_mutex_trylock (m) : (*(int *)(m) ? 1 : ((*(int *)(m) = 1), 0)))
#define mutex_unlock(m)         \
   (__pthread_mutex_unlock != nullptr \
    ? __pthread_mutex_unlock (m) : (*(int*)(m) = 0))

#define thread_atfork(prepare, parent, child) \
   (__pthread_atfork != nullptr ? __pthread_atfork(prepare, parent, child) : 0)

#elif defined(MUTEX_INITIALIZER)
/* Assume hurd, with cthreads */

/* Cthreads `mutex_t' is a pointer to a mutex, and malloc wants just the
   mutex itself.  */
#undef mutex_t
#define mutex_t struct mutex

#undef mutex_init
#define mutex_init(m) (__mutex_init(m), 0)

#undef mutex_lock
#define mutex_lock(m) (__mutex_lock(m), 0)

#undef mutex_unlock
#define mutex_unlock(m) (__mutex_unlock(m), 0)

#define mutex_trylock(m) (!__mutex_trylock(m))

#define thread_atfork(prepare, parent, child) do {} while(0)
#define thread_atfork_static(prepare, parent, child) \
 text_set_element(_hurd_fork_prepare_hook, prepare); \
 text_set_element(_hurd_fork_parent_hook, parent); \
 text_set_element(_hurd_fork_child_hook, child);

/* No we're *not* using pthreads.  */
#define __pthread_initialize ((void (*)(void))0)

#else

#define NO_THREADS

#endif /* MUTEX_INITIALIZER && PTHREAD_MUTEX_INITIALIZER */

#ifndef NO_THREADS

/* thread specific data for glibc */

#include <bits/libc-tsd.h>

typedef int tsd_key_t[1];       /* no key data structure, libc magic does it */
__libc_tsd_define (, MALLOC)    /* declaration/common definition */
#define tsd_key_create(key, destr)      ((void) (key))
#define tsd_setspecific(key, data)      __libc_tsd_set (MALLOC, (data))
#define tsd_getspecific(key, vptr)      ((vptr) = __libc_tsd_get (MALLOC))

#endif

#elif defined(USE_PTHREADS) /* Posix threads */

#include <pthread.h>

typedef pthread_t thread_id;

/* mutex */
#if (defined __i386__ || defined __x86_64__) && defined __GNUC__ && \
    !defined USE_NO_SPINLOCKS

#include <time.h>

/* Use fast inline spinlocks.  */
typedef struct {
  volatile unsigned int lock;
  int pad0_;
} mutex_t;

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              ((m)->lock = 0)
static inline int mutex_lock(mutex_t *m) {
  int cnt = 0, r;
  struct timespec tm;

  for(;;) {
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(m->lock)
       : "0"(1), "m"(m->lock)
       : "memory");
    if(!r)
      return 0;
    if(cnt < 50) {
      sched_yield();
      cnt++;
    } else {
      tm.tv_sec = 0;
      tm.tv_nsec = 2000001;
      nanosleep(&tm, nullptr);
      cnt = 0;
    }
  }
}
static inline int mutex_trylock(mutex_t *m) {
  int r;

  __asm__ __volatile__
    ("xchgl %0, %1"
     : "=r"(r), "=m"(m->lock)
     : "0"(1), "m"(m->lock)
     : "memory");
  return r;
}
static inline int mutex_unlock(mutex_t *m) {
  int r;

  __asm__ __volatile__
    ("xchgl %0, %1"
     : "=r"(r), "=m"(m->lock)
     : "0"(0), "m"(m->lock)
     : "memory");
  return 0;
}

#else

/* Normal pthread mutex.  */
typedef pthread_mutex_t mutex_t;

#define MUTEX_INITIALIZER          PTHREAD_MUTEX_INITIALIZER
#define mutex_init(m)              pthread_mutex_init(m, nullptr)
#define mutex_lock(m)              pthread_mutex_lock(m)
#define mutex_trylock(m)           pthread_mutex_trylock(m)
#define mutex_unlock(m)            pthread_mutex_unlock(m)

#endif /* (__i386__ || __x86_64__) && __GNUC__ && !USE_NO_SPINLOCKS */

/* thread specific data */
#if defined(__sgi) || defined(USE_TSD_DATA_HACK)

/* Hack for thread-specific data, e.g. on Irix 6.x.  We can't use
   pthread_setspecific because that function calls malloc() itself.
   The hack only works when pthread_t can be converted to an integral
   type. */

typedef void *tsd_key_t[256];
#define tsd_key_create(key, destr) do { \
  int i; \
  for(i=0; i<256; i++) (*key)[i] = 0; \
} while(0)
#define tsd_setspecific(key, data) \
 (key[(unsigned)pthread_self() % 256] = (data))
#define tsd_getspecific(key, vptr) \
 (vptr = key[(unsigned)pthread_self() % 256])

#else

typedef pthread_key_t tsd_key_t;

#define tsd_key_create(key, destr) pthread_key_create(key, destr)
#define tsd_setspecific(key, data) pthread_setspecific(key, data)
#define tsd_getspecific(key, vptr) (vptr = pthread_getspecific(key))

#endif

/* at fork */
#define thread_atfork(prepare, parent, child) \
                                   pthread_atfork(prepare, parent, child)

#elif USE_THR /* Solaris threads */

#include <thread.h>

typedef thread_t thread_id;

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              mutex_init(m, USYNC_THREAD, nullptr)

/*
 * Hack for thread-specific data on Solaris.  We can't use thr_setspecific
 * because that function calls malloc() itself.
 */
typedef void *tsd_key_t[256];
#define tsd_key_create(key, destr) do { \
  int i; \
  for(i=0; i<256; i++) (*key)[i] = 0; \
} while(0)
#define tsd_setspecific(key, data) (key[(unsigned)thr_self() % 256] = (data))
#define tsd_getspecific(key, vptr) (vptr = key[(unsigned)thr_self() % 256])

#define thread_atfork(prepare, parent, child) do {} while(0)

#elif USE_SPROC /* SGI sproc() threads */

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <abi_mutex.h>

typedef int thread_id;

typedef abilock_t mutex_t;

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              init_lock(m)
#define mutex_lock(m)              (spin_lock(m), 0)
#define mutex_trylock(m)           acquire_lock(m)
#define mutex_unlock(m)            release_lock(m)

typedef int tsd_key_t;
int tsd_key_next;
#define tsd_key_create(key, destr) ((*key) = tsd_key_next++)
#define tsd_setspecific(key, data) (((void **)(&PRDA->usr_prda))[key] = data)
#define tsd_getspecific(key, vptr) (vptr = ((void **)(&PRDA->usr_prda))[key])

#define thread_atfork(prepare, parent, child) do {} while(0)

#elif defined(WIN32)
/* Win32 emulation */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#define _WIN32_WINNT 0x600
#include <windows.h>

/* These are emulations based on InterlockedExchange() etc. */
/*static long slwait(long *sl);
static long sltrywait(long *sl);
static long slrelease(long *sl);*/

typedef int thread_id;
/*typedef long mutex_t;

#define MUTEX_INITIALIZER          0
#define mutex_init(m)              (*(m) = 0)
#define mutex_lock(m)              slwait(m)
#define mutex_trylock(m)           sltrywait(m)
#define mutex_unlock(m)            slrelease(m)*/

/* This won't work on Windows 9x. Can't say I personally care (crappy OS) */
typedef CRITICAL_SECTION mutex_t;

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              (!InitializeCriticalSectionAndSpinCount(m, 4000))
#define mutex_lock(m)              (EnterCriticalSection(m), 0)
#define mutex_trylock(m)           (!TryEnterCriticalSection(m))
#define mutex_unlock(m)            (LeaveCriticalSection(m), 0)

typedef DWORD tsd_key_t;
#define tsd_key_create(key, destr) (*(key)=TlsAlloc(), TLS_OUT_OF_INDEXES!=(*key))
#define tsd_setspecific(key, data) (!TlsSetValue(key, data))
#define tsd_getspecific(key, vptr) (vptr = TlsGetValue(key))

#define thread_atfork(prepare, parent, child) do {} while(0)

#else /* no _LIBC or USE_... are defined */

#define NO_THREADS

#endif /* defined(_LIBC) */

#ifdef NO_THREADS /* No threads, provide dummy macros */

typedef int thread_id;

/* The mutex functions used to do absolutely nothing, i.e. lock,
   trylock and unlock would always just return 0.  However, even
   without any concurrently active threads, a mutex can be used
   legitimately as an `in use' flag.  To make the code that is
   protected by a mutex async-signal safe, these macros would have to
   be based on atomic test-and-set operations, for example. */
typedef int mutex_t;

#define MUTEX_INITIALIZER          0
#define mutex_init(m)              (*(m) = 0)
#define mutex_lock(m)              ((*(m) = 1), 0)
#define mutex_trylock(m)           (*(m) ? 1 : ((*(m) = 1), 0))
#define mutex_unlock(m)            (*(m) = 0)

typedef void *tsd_key_t;
#define tsd_key_create(key, destr) do {} while(0)
#define tsd_setspecific(key, data) ((key) = (data))
#define tsd_getspecific(key, vptr) (vptr = (key))

#define thread_atfork(prepare, parent, child) do {} while(0)

#endif /* defined(NO_THREADS) */



/*************************** ptmalloc2.c ******************************/

#ifdef WIN32

/* Win32 doesn't supply or need the following headers */
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H

/* Use the supplied emulation of sbrk */
#define MORECORE sbrk
#define MORECORE_CONTIGUOUS 1
#define MORECORE_FAILURE    ((void*)(-1))

/* Use the supplied emulation of mmap and munmap */
#define HAVE_MMAP 1
#define MUNMAP_FAILURE  (-1)
#define MMAP_CLEARS 1

/* For the windows mmap emulation */
#define MAP_FIXED 0
#define MAP_PRIVATE 1
#define MAP_ANONYMOUS 2
#define MAP_NORESERVE 4
#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2

/* Emulation functions defined at the end of this file */

static long getpagesize(void);
static long getregionsize(void);
static void *sbrk(long size);
static void *mmap(void *ptr, long size, long prot, long type, long handle, long arg);
static long munmap(void *ptr, long size);
static int mprotect(const void *addr, long len, int prot);

static void vminfo (unsigned long*free, unsigned long*reserved, unsigned long*committed);
static int cpuinfo (int whole, unsigned long*kernel, unsigned long*user);

#endif

/*
  __STD_C should be nonzero if using ANSI-standard C compiler, a C++
  compiler, or a C compiler sufficiently close to ANSI to get away
  with it.
*/

#ifndef __STD_C
#if defined(__STDC__) || defined(__cplusplus)
#define __STD_C     1
#else
#define __STD_C     0
#endif 
#endif /*__STD_C*/


/*
  Void_t* is the pointer type that malloc should say it returns
*/

#ifndef Void_t
#if (__STD_C || defined(WIN32))
#define Void_t      void
#else
#define Void_t      char
#endif
#endif /*Void_t*/

#if __STD_C
#include <stddef.h>   /* for size_t */
#include <stdlib.h>   /* for getenv(), abort() */
#else
#include <sys/types.h>
#endif

#if defined(__cplusplus) && ! defined(PTMALLOC_IN_CPPNAMESPACE)
extern "C" {
#endif

/* define LACKS_UNISTD_H if your system does not have a <unistd.h>. */

/* #define  LACKS_UNISTD_H */

#ifndef LACKS_UNISTD_H
#include <unistd.h>
#endif

/* define LACKS_SYS_PARAM_H if your system does not have a <sys/param.h>. */

/* #define  LACKS_SYS_PARAM_H */


#include <stdio.h>    /* needed for malloc_stats */
#include <errno.h>    /* needed for optional MALLOC_FAILURE_ACTION */


/*
  Debugging:

  Because freed chunks may be overwritten with bookkeeping fields, this
  malloc will often die when freed memory is overwritten by user
  programs.  This can be very effective (albeit in an annoying way)
  in helping track down dangling pointers.

  If you compile with -DMALLOC_DEBUG, a number of assertion checks are
  enabled that will catch more memory errors. You probably won't be
  able to make much sense of the actual assertion errors, but they
  should help you locate incorrectly overwritten memory.  The
  checking is fairly extensive, and will slow down execution
  noticeably. Calling malloc_stats or mallinfo with MALLOC_DEBUG set
  will attempt to check every non-mmapped allocated and free chunk in
  the course of computing the summmaries. (By nature, mmapped regions
  cannot be checked very much automatically.)

  Setting MALLOC_DEBUG may also be helpful if you are trying to modify
  this code. The assertions in the check routines spell out in more
  detail the assumptions and invariants underlying the algorithms.

  Setting MALLOC_DEBUG does NOT provide an automated mechanism for
  checking that all accesses to malloced memory stay within their
  bounds. However, there are several add-ons and adaptations of this
  or other mallocs available that do this.
*/

#if MALLOC_DEBUG
#include <assert.h>
#else
#define assert(x) ((void)0)
#endif

/*
  The unsigned integer type used for comparing any two chunk sizes.
  This should be at least as wide as size_t, but should not be signed.
*/

#ifndef CHUNK_SIZE_T
#define CHUNK_SIZE_T unsigned long
#endif

/* 
  The unsigned integer type used to hold addresses when they are are
  manipulated as integers. Except that it is not defined on all
  systems, intptr_t would suffice.
*/
#ifndef PTR_UINT
#define PTR_UINT unsigned long
#endif


/*
  INTERNAL_SIZE_T is the word-size used for internal bookkeeping
  of chunk sizes.

  The default version is the same as size_t.

  While not strictly necessary, it is best to define this as an
  unsigned type, even if size_t is a signed type. This may avoid some
  artificial size limitations on some systems.

  On a 64-bit machine, you may be able to reduce malloc overhead by
  defining INTERNAL_SIZE_T to be a 32 bit `unsigned int' at the
  expense of not being able to handle more than 2^32 of malloced
  space. If this limitation is acceptable, you are encouraged to set
  this unless you are on a platform requiring 16byte alignments. In
  this case the alignment requirements turn out to negate any
  potential advantages of decreasing size_t word size.

  Implementors: Beware of the possible combinations of:
     - INTERNAL_SIZE_T might be signed or unsigned, might be 32 or 64 bits,
       and might be the same width as int or as long
     - size_t might have different width and signedness as INTERNAL_SIZE_T
     - int and long might be 32 or 64 bits, and might be the same width
  To deal with this, most comparisons and difference computations
  among INTERNAL_SIZE_Ts should cast them to CHUNK_SIZE_T, being
  aware of the fact that casting an unsigned int to a wider long does
  not sign-extend. (This also makes checking for negative numbers
  awkward.) Some of these casts result in harmless compiler warnings
  on some systems.
*/

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size */
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))



/*
  MALLOC_ALIGNMENT is the minimum alignment for malloc'ed chunks.
  It must be a power of two at least 2 * SIZE_SZ, even on machines
  for which smaller alignments would suffice. It may be defined as
  larger than this though. Note however that code and data structures
  are optimized for the case of 8-byte alignment.
*/

/* drose: it turns out that setting MALLOC_ALIGNMENT to 16 doesn't
   work; something crashes internally.  Too bad. */

#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT       (2 * SIZE_SZ)
#endif

/* The corresponding bit mask value */
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)



/*
  REALLOC_ZERO_BYTES_FREES should be set if a call to
  realloc with zero bytes should be the same as a call to free.
  This is required by the C standard. Otherwise, since this malloc
  returns a unique pointer for malloc(0), so does realloc(p, 0).
*/

/*   #define REALLOC_ZERO_BYTES_FREES */

/*
  TRIM_FASTBINS controls whether free() of a very small chunk can
  immediately lead to trimming. Setting to true (1) can reduce memory
  footprint, but will almost always slow down programs that use a lot
  of small chunks.

  Define this only if you are willing to give up some speed to more
  aggressively reduce system-level memory footprint when releasing
  memory in programs that use many small chunks.  You can get
  essentially the same effect by setting MXFAST to 0, but this can
  lead to even greater slowdowns in programs using many small chunks.
  TRIM_FASTBINS is an in-between compile-time option, that disables
  only those chunks bordering topmost memory from being placed in
  fastbins.
*/

#ifndef TRIM_FASTBINS
#define TRIM_FASTBINS  0
#endif


/*
  USE_DL_PREFIX will prefix all public routines with the string 'dl'.
  This is necessary when you only want to use this malloc in one part 
  of a program, using your regular system malloc elsewhere.
*/

/* #define USE_DL_PREFIX */


/*
   Two-phase name translation.
   All of the actual routines are given mangled names.
   When wrappers are used, they become the public callable versions.
   When DL_PREFIX is used, the callable names are prefixed.
*/

#ifdef USE_DL_PREFIX
#define public_cALLOc    dlcalloc
#define public_fREe      dlfree
#define public_cFREe     dlcfree
#define public_mALLOc    dlmalloc
#define public_mEMALIGn  dlmemalign
#define public_rEALLOc   dlrealloc
#define public_vALLOc    dlvalloc
#define public_pVALLOc   dlpvalloc
#define public_mALLINFo  dlmallinfo
#define public_mALLOPt   dlmallopt
#define public_mTRIm     dlmalloc_trim
#define public_mSTATs    dlmalloc_stats
#define public_mUSABLe   dlmalloc_usable_size
#define public_iCALLOc   dlindependent_calloc
#define public_iCOMALLOc dlindependent_comalloc
#define public_gET_STATe dlget_state
#define public_sET_STATe dlset_state
#else /* USE_DL_PREFIX */
#define public_cALLOc    calloc
#define public_fREe      free
#define public_cFREe     cfree
#define public_mALLOc    malloc
#define public_mEMALIGn  memalign
#define public_rEALLOc   realloc
#define public_vALLOc    valloc
#define public_pVALLOc   pvalloc
#define public_mALLINFo  mallinfo
#define public_mALLOPt   mallopt
#define public_mTRIm     malloc_trim
#define public_mSTATs    malloc_stats
#define public_mUSABLe   malloc_usable_size
#define public_iCALLOc   independent_calloc
#define public_iCOMALLOc independent_comalloc
#define public_gET_STATe malloc_get_state
#define public_sET_STATe malloc_set_state
#endif /* USE_DL_PREFIX */


/*
  HAVE_MEMCPY should be defined if you are not otherwise using
  ANSI STD C, but still have memcpy and memset in your C library
  and want to use them in calloc and realloc. Otherwise simple
  macro versions are defined below.

  USE_MEMCPY should be defined as 1 if you actually want to
  have memset and memcpy called. People report that the macro
  versions are faster than libc versions on some systems.
  
  Even if USE_MEMCPY is set to 1, loops to copy/clear small chunks
  (of <= 36 bytes) are manually unrolled in realloc and calloc.
*/

#define HAVE_MEMCPY

#ifndef USE_MEMCPY
#ifdef HAVE_MEMCPY
#define USE_MEMCPY 1
#else
#define USE_MEMCPY 0
#endif
#endif


#if (__STD_C || defined(HAVE_MEMCPY))
#include <memory.h>
#endif

/*
  MALLOC_FAILURE_ACTION is the action to take before "return 0" when
  malloc fails to be able to return memory, either because memory is
  exhausted or because of illegal arguments.
  
  By default, sets errno if running on STD_C platform, else does nothing.  
*/

#ifndef MALLOC_FAILURE_ACTION
#if __STD_C
#define MALLOC_FAILURE_ACTION \
   errno = ENOMEM;

#else
#define MALLOC_FAILURE_ACTION
#endif
#endif

/*
  MORECORE-related declarations. By default, rely on sbrk
*/


#if defined(LACKS_UNISTD_H) && !defined(WIN32)
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
#if __STD_C
extern Void_t*     sbrk(ptrdiff_t);
#else
extern Void_t*     sbrk();
#endif
#endif
#endif

/*
  MORECORE_IS_MMAP causes requests to obtain memory from the system
  using mmap() rather than sbrk(). This is useful for testing and
  when your default allocator cannot coexist with ptmalloc2 extending
  core memory.
*/
#ifdef MORECORE_IS_MMAP
#undef MORECORE
#undef MORECORE_CONTIGUOUS
static void *sbrkfail(long size);
#define MORECORE sbrkfail
#define MORECORE_CONTIGUOUS 0
#endif

/*
  MORECORE is the name of the routine to call to obtain more memory
  from the system.  See below for general guidance on writing
  alternative MORECORE functions, as well as a version for WIN32 and a
  sample version for pre-OSX macos.
*/

#ifndef MORECORE
#define MORECORE sbrk
#endif

/*
  MORECORE_FAILURE is the value returned upon failure of MORECORE
  as well as mmap. Since it cannot be an otherwise valid memory address,
  and must reflect values of standard sys calls, you probably ought not
  try to redefine it.
*/

#ifndef MORECORE_FAILURE
#define MORECORE_FAILURE (-1)
#endif

/*
  If MORECORE_CONTIGUOUS is true, take advantage of fact that
  consecutive calls to MORECORE with positive arguments always return
  contiguous increasing addresses.  This is true of unix sbrk.  Even
  if not defined, when regions happen to be contiguous, malloc will
  permit allocations spanning regions obtained from different
  calls. But defining this when applicable enables some stronger
  consistency checks and space efficiencies. 
*/

#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif

/*
  Define MORECORE_CANNOT_TRIM if your version of MORECORE
  cannot release space back to the system when given negative
  arguments. This is generally necessary only if you are using
  a hand-crafted MORECORE function that cannot handle negative arguments.
*/

/* #define MORECORE_CANNOT_TRIM */

/*  MORECORE_CLEARS           (default 1)
     The degree to which the routine mapped to MORECORE zeroes out
     memory: never (0), only for newly allocated space (1) or always
     (2).  The distinction between (1) and (2) is necessary because on
     some systems, if the application first decrements and then
     increments the break value, the contents of the reallocated space
     are unspecified.
*/

#ifndef MORECORE_CLEARS
#define MORECORE_CLEARS 1
#endif


/*
  Define HAVE_MMAP as true to optionally make malloc() use mmap() to
  allocate very large blocks.  These will be returned to the
  operating system immediately after a free(). Also, if mmap
  is available, it is used as a backup strategy in cases where
  MORECORE fails to provide space from system.

  This malloc is best tuned to work with mmap for large requests.
  If you do not have mmap, operations involving very large chunks (1MB
  or so) may be slower than you'd like.
*/

#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif

#if HAVE_MMAP
/* 
   Standard unix mmap using /dev/zero clears memory so calloc doesn't
   need to.
*/

#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif

#else /* no mmap */
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 0
#endif
#endif


/* 
   MMAP_AS_MORECORE_SIZE is the minimum mmap size argument to use if
   sbrk fails, and mmap is used as a backup (which is done only if
   HAVE_MMAP).  The value must be a multiple of page size.  This
   backup strategy generally applies only when systems have "holes" in
   address space, so sbrk cannot perform contiguous expansion, but
   there is still space available on system.  On systems for which
   this is known to be useful (i.e. most linux kernels), this occurs
   only when programs allocate huge amounts of memory.  Between this,
   and the fact that mmap regions tend to be limited, the size should
   be large, to avoid too many mmap calls and thus avoid running out
   of kernel resources.
*/

#ifndef MMAP_AS_MORECORE_SIZE
#define MMAP_AS_MORECORE_SIZE (1024 * 1024)
#endif

/*
  Define HAVE_MREMAP to make realloc() use mremap() to re-allocate
  large blocks.  This is currently only possible on Linux with
  kernel versions newer than 1.3.77.
*/

#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#else
#define HAVE_MREMAP 0
#endif

#endif /* HAVE_MMAP */

/* Define USE_ARENAS to enable support for multiple `arenas'.  These
   are allocated using mmap(), are necessary for threads and
   occasionally useful to overcome address space limitations affecting
   sbrk(). */

#ifndef USE_ARENAS
#define USE_ARENAS HAVE_MMAP
#endif


/*
  The system page size. To the extent possible, this malloc manages
  memory from the system in page-size units.  Note that this value is
  cached during initialization into a field of malloc_state. So even
  if malloc_getpagesize is a function, it is only called once.

  The following mechanics for getpagesize were adapted from bsd/gnu
  getpagesize.h. If none of the system-probes here apply, a value of
  4096 is used, which should be OK: If they don't apply, then using
  the actual value probably doesn't impact performance.
*/


#ifndef malloc_getpagesize

#ifndef LACKS_UNISTD_H
#  include <unistd.h>
#endif

#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif

#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  else
#    if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       extern size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    else
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize() 
#      else
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        else
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            else
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          else
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            else
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              else /* just guess */
#                define malloc_getpagesize (4096) 
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif

/*
  This version of malloc supports the standard SVID/XPG mallinfo
  routine that returns a struct containing usage properties and
  statistics. It should work on any SVID/XPG compliant system that has
  a /usr/include/malloc.h defining struct mallinfo. (If you'd like to
  install such a thing yourself, cut out the preliminary declarations
  as described above and below and save them in a malloc.h file. But
  there's no compelling reason to bother to do this.)

  The main declaration needed is the mallinfo struct that is returned
  (by-copy) by mallinfo().  The SVID/XPG malloinfo struct contains a
  bunch of fields that are not even meaningful in this version of
  malloc.  These fields are are instead filled by mallinfo() with
  other numbers that might be of interest.

  HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
  /usr/include/malloc.h file that includes a declaration of struct
  mallinfo.  If so, it is included; else an SVID2/XPG2 compliant
  version is declared below.  These must be precisely the same for
  mallinfo() to work.  The original SVID version of this struct,
  defined on most systems with mallinfo, declares all fields as
  ints. But some others define as unsigned long. If your system
  defines the fields using a type of different width than listed here,
  you must #include your system version and #define
  HAVE_USR_INCLUDE_MALLOC_H.
*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#if defined(PTMALLOC_IN_CPPNAMESPACE) || !defined(HAVE_USR_INCLUDE_MALLOC_H)

/* SVID2/XPG mallinfo structure */

struct mallinfo {
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

/*
  SVID/XPG defines four standard parameter numbers for mallopt,
  normally defined in malloc.h.  Only one of these (M_MXFAST) is used
  in this malloc. The others (M_NLBLKS, M_GRAIN, M_KEEP) don't apply,
  so setting them has no effect. But this malloc also supports other
  options in mallopt described below.
*/

#elif defined(HAVE_USR_INCLUDE_MALLOC_H)
#include "/usr/include/malloc.h"
#endif


/* ---------- description of public routines ------------ */

/*
  malloc(size_t n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or null
  if no space is available. Additionally, on failure, errno is
  set to ENOMEM on ANSI C systems.

  If n is zero, malloc returns a minumum-sized chunk. (The minimum
  size is 16 bytes on most 32bit systems, and 24 or 32 bytes on 64bit
  systems.)  On most systems, size_t is an unsigned type, so calls
  with negative arguments are interpreted as requests for huge amounts
  of space, which will often fail. The maximum supported value of n
  differs across systems, but is in all cases less than the maximum
  representable value of a size_t.
*/
#if __STD_C
Void_t*  public_mALLOc(size_t);
#else
Void_t*  public_mALLOc();
#endif

/*
  free(Void_t* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. It can have arbitrary (i.e., bad!)
  effects if p has already been freed.

  Unless disabled (using mallopt), freeing very large spaces will
  when possible, automatically trigger operations that give
  back unused memory to the system, thus reducing program footprint.
*/
#if __STD_C
void     public_fREe(Void_t*);
#else
void     public_fREe();
#endif

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
#if __STD_C
Void_t*  public_cALLOc(size_t, size_t);
#else
Void_t*  public_cALLOc();
#endif

/*
  realloc(Void_t* p, size_t n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available. 

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p when possible, otherwise it employs the
  equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.  

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  Unless the #define
  REALLOC_ZERO_BYTES_FREES is set, realloc with a size argument of
  zero (re)allocates a minimum-sized chunk.

  Large chunks that were internally obtained via mmap will always
  be reallocated using malloc-copy-free sequences unless
  the system supports MREMAP (currently only linux).

  The old unix realloc convention of allowing the last-free'd chunk
  to be used as an argument to realloc is not supported.
*/
#if __STD_C
Void_t*  public_rEALLOc(Void_t*, size_t);
#else
Void_t*  public_rEALLOc();
#endif

/*
  memalign(size_t alignment, size_t n);
  Returns a pointer to a newly allocated chunk of n bytes, aligned
  in accord with the alignment argument.

  The alignment argument should be a power of two. If the argument is
  not a power of two, the nearest greater power is used.
  8-byte alignment is guaranteed by normal malloc calls, so don't
  bother calling memalign with an argument of 8 or less.

  Overreliance on memalign is a sure way to fragment space.
*/
#if __STD_C
Void_t*  public_mEMALIGn(size_t, size_t);
#else
Void_t*  public_mEMALIGn();
#endif

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
#if __STD_C
Void_t*  public_vALLOc(size_t);
#else
Void_t*  public_vALLOc();
#endif



/*
  mallopt(int parameter_number, int parameter_value)
  Sets tunable parameters The format is to provide a
  (parameter-number, parameter-value) pair.  mallopt then sets the
  corresponding parameter to the argument value if it can (i.e., so
  long as the value is meaningful), and returns 1 if successful else
  0.  SVID/XPG/ANSI defines four standard param numbers for mallopt,
  normally defined in malloc.h.  Only one of these (M_MXFAST) is used
  in this malloc. The others (M_NLBLKS, M_GRAIN, M_KEEP) don't apply,
  so setting them has no effect. But this malloc also supports four
  other options in mallopt. See below for details.  Briefly, supported
  parameters are as follows (listed defaults are for "typical"
  configurations).

  Symbol            param #   default    allowed param values
  M_MXFAST          1         64         0-80  (0 disables fastbins)
  M_TRIM_THRESHOLD -1         256*1024   any   (-1U disables trimming)
  M_TOP_PAD        -2         0          any  
  M_MMAP_THRESHOLD -3         256*1024   any   (or 0 if no MMAP support)
  M_MMAP_MAX       -4         65536      any   (0 disables use of mmap)
*/
#if __STD_C
int      public_mALLOPt(int, int);
#else
int      public_mALLOPt();
#endif


/*
  mallinfo()
  Returns (by copy) a struct containing various summary statistics:

  arena:     current total non-mmapped bytes allocated from system 
  ordblks:   the number of free chunks 
  smblks:    the number of fastbin blocks (i.e., small chunks that
               have been freed but not use resused or consolidated)
  hblks:     current number of mmapped regions 
  hblkhd:    total bytes held in mmapped regions 
  usmblks:   the maximum total allocated space. This will be greater
                than current total if trimming has occurred.
  fsmblks:   total bytes held in fastbin blocks 
  uordblks:  current total allocated space (normal or mmapped)
  fordblks:  total free space 
  keepcost:  the maximum number of bytes that could ideally be released
               back to system via malloc_trim. ("ideally" means that
               it ignores page restrictions etc.)

  Because these fields are ints, but internal bookkeeping may
  be kept as longs, the reported values may wrap around zero and 
  thus be inaccurate.
*/
#if __STD_C
struct mallinfo public_mALLINFo(void);
#else
struct mallinfo public_mALLINFo();
#endif

/*
  independent_calloc(size_t n_elements, size_t element_size, Void_t* chunks[]);

  independent_calloc is similar to calloc, but instead of returning a
  single cleared space, it returns an array of pointers to n_elements
  independent elements that can hold contents of size elem_size, each
  of which starts out cleared, and can be independently freed,
  realloc'ed etc. The elements are guaranteed to be adjacently
  allocated (this is not guaranteed to occur with multiple callocs or
  mallocs), which may also improve cache locality in some
  applications.

  The "chunks" argument is optional (i.e., may be null, which is
  probably the most typical usage). If it is null, the returned array
  is itself dynamically allocated and should also be freed when it is
  no longer needed. Otherwise, the chunks array must be of at least
  n_elements in length. It is filled in with the pointers to the
  chunks.

  In either case, independent_calloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and "chunks"
  is null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use regular calloc and assign pointers into this
  space to represent elements.  (In this case though, you cannot
  independently free elements.)
  
  independent_calloc simplifies and speeds up implementations of many
  kinds of pools.  It may also be useful when constructing large data
  structures that initially have a fixed number of fixed-sized nodes,
  but the number is not known at compile time, and some of the nodes
  may later need to be freed. For example:

  struct Node { int item; struct Node* next; };
  
  struct Node* build_list() {
    struct Node** pool;
    int n = read_number_of_nodes_needed();
    if (n <= 0) return 0;
    pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
    if (pool == 0) die(); 
    // organize into a linked list... 
    struct Node* first = pool[0];
    for (i = 0; i < n-1; ++i) 
      pool[i]->next = pool[i+1];
    free(pool);     // Can now free the array (or not, if it is needed later)
    return first;
  }
*/
#if __STD_C
Void_t** public_iCALLOc(size_t, size_t, Void_t**);
#else
Void_t** public_iCALLOc();
#endif

/*
  independent_comalloc(size_t n_elements, size_t sizes[], Void_t* chunks[]);

  independent_comalloc allocates, all at once, a set of n_elements
  chunks with sizes indicated in the "sizes" array.    It returns
  an array of pointers to these elements, each of which can be
  independently freed, realloc'ed etc. The elements are guaranteed to
  be adjacently allocated (this is not guaranteed to occur with
  multiple callocs or mallocs), which may also improve cache locality
  in some applications.

  The "chunks" argument is optional (i.e., may be null). If it is null
  the returned array is itself dynamically allocated and should also
  be freed when it is no longer needed. Otherwise, the chunks array
  must be of at least n_elements in length. It is filled in with the
  pointers to the chunks.

  In either case, independent_comalloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and chunks is
  null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).
  
  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use a single regular malloc, and assign pointers at
  particular offsets in the aggregate space. (In this case though, you 
  cannot independently free elements.)

  independent_comallac differs from independent_calloc in that each
  element may have a different size, and also that it does not
  automatically clear elements.

  independent_comalloc can be used to speed up allocation in cases
  where several structs or objects must always be allocated at the
  same time.  For example:

  struct Head { ... }
  struct Foot { ... }

  void send_message(char* msg) {
    int msglen = strlen(msg);
    size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
    void* chunks[3];
    if (independent_comalloc(3, sizes, chunks) == 0)
      die();
    struct Head* head = (struct Head*)(chunks[0]);
    char*        body = (char*)(chunks[1]);
    struct Foot* foot = (struct Foot*)(chunks[2]);
    // ...
  }

  In general though, independent_comalloc is worth using only for
  larger values of n_elements. For small values, you probably won't
  detect enough difference from series of malloc calls to bother.

  Overuse of independent_comalloc can increase overall memory usage,
  since it cannot reuse existing noncontiguous small chunks that
  might be available for some of the elements.
*/
#if __STD_C
Void_t** public_iCOMALLOc(size_t, size_t*, Void_t**);
#else
Void_t** public_iCOMALLOc();
#endif


/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
#if __STD_C
Void_t*  public_pVALLOc(size_t);
#else
Void_t*  public_pVALLOc();
#endif

/*
  cfree(Void_t* p);
  Equivalent to free(p).

  cfree is needed/defined on some systems that pair it with calloc,
  for odd historical reasons (such as: cfree is used in example 
  code in the first edition of K&R).
*/
#if __STD_C
void     public_cFREe(Void_t*);
#else
void     public_cFREe();
#endif

/*
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative
  arguments to sbrk) if there is unused memory at the `high' end of
  the malloc pool. You can call this after freeing large blocks of
  memory to potentially reduce the system-level memory requirements
  of a program. However, it cannot guarantee to reduce memory. Under
  some allocation patterns, some large free blocks of memory will be
  locked between two used chunks, so they cannot be given back to
  the system.
  
  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero,
  only the minimum amount of memory to maintain internal data
  structures will be left (one page or less). Non-zero arguments
  can be supplied to maintain enough trailing space to service
  future expected allocations without having to re-obtain memory
  from the system.
  
  Malloc_trim returns 1 if it actually released any memory, else 0.
  On systems that do not support "negative sbrks", it will always
  rreturn 0.
*/
#if __STD_C
int      public_mTRIm(size_t);
#else
int      public_mTRIm();
#endif

/*
  malloc_usable_size(Void_t* p);

  Returns the number of bytes you can actually use in
  an allocated chunk, which may be more than you requested (although
  often not) due to alignment and minimum size constraints.
  You can use this many bytes without worrying about
  overwriting other allocated objects. This is not a particularly great
  programming practice. malloc_usable_size can be more useful in
  debugging and assertions, for example:

  p = malloc(n);
  assert(malloc_usable_size(p) >= 256);

*/
#if __STD_C
size_t   public_mUSABLe(Void_t*);
#else
size_t   public_mUSABLe();
#endif

/*
  malloc_stats();
  Prints on stderr the amount of space obtained from the system (both
  via sbrk and mmap), the maximum amount (which may be more than
  current if malloc_trim and/or munmap got called), and the current
  number of bytes allocated via malloc (or realloc, etc) but not yet
  freed. Note that this is the number of bytes allocated, not the
  number requested. It will be larger than the number requested
  because of alignment and bookkeeping overhead. Because it includes
  alignment wastage as being in use, this figure may be greater than
  zero even when no user-level chunks are allocated.

  The reported current and maximum system memory can be inaccurate if
  a program makes other calls to system memory allocation functions
  (normally sbrk) outside of malloc.

  malloc_stats prints only the most commonly interesting statistics.
  More information can be obtained by calling mallinfo.

*/
#if __STD_C
void     public_mSTATs(void);
#else
void     public_mSTATs();
#endif

/*
  malloc_get_state(void);

  Returns the state of all malloc variables in an opaque data
  structure.
*/
#if __STD_C
Void_t*  public_gET_STATe(void);
#else
Void_t*  public_gET_STATe();
#endif

/*
  malloc_set_state(Void_t* state);

  Restore the state of all malloc variables from data obtained with
  malloc_get_state().
*/
#if __STD_C
int      public_sET_STATe(Void_t*);
#else
int      public_sET_STATe();
#endif

#ifdef _LIBC
/*
  posix_memalign(void **memptr, size_t alignment, size_t size);

  POSIX wrapper like memalign(), checking for validity of size.
*/
int      __posix_memalign(void **, size_t, size_t);
#endif

/* mallopt tuning options */

/*
  M_MXFAST is the maximum request size used for "fastbins", special bins
  that hold returned chunks without consolidating their spaces. This
  enables future requests for chunks of the same size to be handled
  very quickly, but can increase fragmentation, and thus increase the
  overall memory footprint of a program.

  This malloc manages fastbins very conservatively yet still
  efficiently, so fragmentation is rarely a problem for values less
  than or equal to the default.  The maximum supported value of MXFAST
  is 80. You wouldn't want it any higher than this anyway.  Fastbins
  are designed especially for use with many small structs, objects or
  strings -- the default handles structs/objects/arrays with sizes up
  to 8 4byte fields, or small strings representing words, tokens,
  etc. Using fastbins for larger objects normally worsens
  fragmentation without improving speed.

  M_MXFAST is set in REQUEST size units. It is internally used in
  chunksize units, which adds padding and alignment.  You can reduce
  M_MXFAST to 0 to disable all use of fastbins.  This causes the malloc
  algorithm to be a closer approximation of fifo-best-fit in all cases,
  not just for larger requests, but will generally cause it to be
  slower.
*/


/* M_MXFAST is a standard SVID/XPG tuning option, usually listed in malloc.h */
#ifndef M_MXFAST
#define M_MXFAST            1
#endif

#ifndef DEFAULT_MXFAST
#define DEFAULT_MXFAST     64
#endif


/*
  M_TRIM_THRESHOLD is the maximum amount of unused top-most memory
  to keep before releasing via malloc_trim in free().

  Automatic trimming is mainly useful in long-lived programs.
  Because trimming via sbrk can be slow on some systems, and can
  sometimes be wasteful (in cases where programs immediately
  afterward allocate more large chunks) the value should be high
  enough so that your overall system performance would improve by
  releasing this much memory.

  The trim threshold and the mmap control parameters (see below)
  can be traded off with one another. Trimming and mmapping are
  two different ways of releasing unused memory back to the
  system. Between these two, it is often possible to keep
  system-level demands of a long-lived program down to a bare
  minimum. For example, in one test suite of sessions measuring
  the XF86 X server on Linux, using a trim threshold of 128K and a
  mmap threshold of 192K led to near-minimal long term resource
  consumption.

  If you are using this malloc in a long-lived program, it should
  pay to experiment with these values.  As a rough guide, you
  might set to a value close to the average size of a process
  (program) running on your system.  Releasing this much memory
  would allow such a process to run in memory.  Generally, it's
  worth it to tune for trimming rather tham memory mapping when a
  program undergoes phases where several large chunks are
  allocated and released in ways that can reuse each other's
  storage, perhaps mixed with phases where there are no such
  chunks at all.  And in well-behaved long-lived programs,
  controlling release of large blocks via trimming versus mapping
  is usually faster.

  However, in most programs, these parameters serve mainly as
  protection against the system-level effects of carrying around
  massive amounts of unneeded memory. Since frequent calls to
  sbrk, mmap, and munmap otherwise degrade performance, the default
  parameters are set to relatively high values that serve only as
  safeguards.

  The trim value It must be greater than page size to have any useful
  effect.  To disable trimming completely, you can set to 
  (unsigned long)(-1)

  Trim settings interact with fastbin (MXFAST) settings: Unless
  TRIM_FASTBINS is defined, automatic trimming never takes place upon
  freeing a chunk with size less than or equal to MXFAST. Trimming is
  instead delayed until subsequent freeing of larger chunks. However,
  you can still force an attempted trim by calling malloc_trim.

  Also, trimming is not generally possible in cases where
  the main arena is obtained via mmap.

  Note that the trick some people use of mallocing a huge space and
  then freeing it at program startup, in an attempt to reserve system
  memory, doesn't have the intended effect under automatic trimming,
  since that memory will immediately be returned to the system.
*/

#define M_TRIM_THRESHOLD       -1

#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (256 * 1024)
#endif

/*
  M_TOP_PAD is the amount of extra `padding' space to allocate or
  retain whenever sbrk is called. It is used in two ways internally:

  * When sbrk is called to extend the top of the arena to satisfy
  a new malloc request, this much padding is added to the sbrk
  request.

  * When malloc_trim is called automatically from free(),
  it is used as the `pad' argument.

  In both cases, the actual amount of padding is rounded
  so that the end of the arena is always a system page boundary.

  The main reason for using padding is to avoid calling sbrk so
  often. Having even a small pad greatly reduces the likelihood
  that nearly every malloc request during program start-up (or
  after trimming) will invoke sbrk, which needlessly wastes
  time.

  Automatic rounding-up to page-size units is normally sufficient
  to avoid measurable overhead, so the default is 0.  However, in
  systems where sbrk is relatively slow, it can pay to increase
  this value, at the expense of carrying around more memory than
  the program needs.
*/

#define M_TOP_PAD              -2

#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif

/*
  M_MMAP_THRESHOLD is the request size threshold for using mmap()
  to service a request. Requests of at least this size that cannot
  be allocated using already-existing space will be serviced via mmap.
  (If enough normal freed space already exists it is used instead.)

  Using mmap segregates relatively large chunks of memory so that
  they can be individually obtained and released from the host
  system. A request serviced through mmap is never reused by any
  other request (at least not directly; the system may just so
  happen to remap successive requests to the same locations).

  Segregating space in this way has the benefits that:

   1. Mmapped space can ALWAYS be individually released back 
      to the system, which helps keep the system level memory 
      demands of a long-lived program low. 
   2. Mapped memory can never become `locked' between
      other chunks, as can happen with normally allocated chunks, which
      means that even trimming via malloc_trim would not release them.
   3. On some systems with "holes" in address spaces, mmap can obtain
      memory that sbrk cannot.

  However, it has the disadvantages that:

   1. The space cannot be reclaimed, consolidated, and then
      used to service later requests, as happens with normal chunks.
   2. It can lead to more wastage because of mmap page alignment
      requirements
   3. It causes malloc performance to be more dependent on host
      system memory management support routines which may vary in
      implementation quality and may impose arbitrary
      limitations. Generally, servicing a request via normal
      malloc steps is faster than going through a system's mmap.

  The advantages of mmap nearly always outweigh disadvantages for
  "large" chunks, but the value of "large" varies across systems.  The
  default is an empirically derived value that works well in most
  systems.
*/

#define M_MMAP_THRESHOLD      -3

#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD (256 * 1024)
#endif

/*
  M_MMAP_MAX is the maximum number of requests to simultaneously
  service using mmap. This parameter exists because
  some systems have a limited number of internal tables for
  use by mmap, and using more than a few of them may degrade
  performance.

  The default is set to a value that serves only as a safeguard.
  Setting to 0 disables use of mmap for servicing large requests.  If
  HAVE_MMAP is not set, the default value is 0, and attempts to set it
  to non-zero values in mallopt will fail.
*/

#define M_MMAP_MAX             -4

#ifndef DEFAULT_MMAP_MAX
#if HAVE_MMAP
#define DEFAULT_MMAP_MAX       (65536)
#else
#define DEFAULT_MMAP_MAX       (0)
#endif
#endif

#if defined(__cplusplus) && !defined(PTMALLOC_IN_CPPNAMESPACE)
};  /* end of extern "C" */
#endif


/***************************** malloc.h *******************************/

/* Prototypes and definition for malloc implementation.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _MALLOC_H
#define _MALLOC_H 1

#ifdef _LIBC
#include <features.h>
#endif

/*
  $Id$
  `ptmalloc2', a malloc implementation for multiple threads without
  lock contention, by Wolfram Gloger <wg@malloc.de>.

  VERSION 2.7.0

  This work is mainly derived from malloc-2.7.0 by Doug Lea
  <dl@cs.oswego.edu>, which is available from:

                 ftp://gee.cs.oswego.edu/pub/misc/malloc.c

  This trimmed-down header file only provides function prototypes and
  the exported data structures.  For more detailed function
  descriptions and compile-time options, see the source file
  `malloc.c'.
*/

#if defined(__STDC__) || defined (__cplusplus)
# include <stddef.h>
# define __malloc_ptr_t  void *
#else
# undef  size_t
# define size_t          unsigned int
# undef  ptrdiff_t
# define ptrdiff_t       int
# define __malloc_ptr_t  char *
#endif

#ifdef _LIBC
/* Used by GNU libc internals. */
# define __malloc_size_t size_t
# define __malloc_ptrdiff_t ptrdiff_t
#elif !defined __attribute_malloc__
# define __attribute_malloc__
#endif

#ifdef __GNUC__

/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this works only with
   gcc 2.8.x and egcs.  */
# if defined __cplusplus && (__GNUC__ >= 3 || __GNUC_MINOR__ >= 8)
#  define __THROW       throw ()
# else
#  define __THROW
# endif

# define __MALLOC_P(args)       args __THROW
/* This macro will be used for functions which might take C++ callback
   functions.  */
# define __MALLOC_PMT(args)     args

#else   /* Not GCC.  */

# ifdef __cplusplus
# define __THROW throw()
# define __const const
# else
# define __THROW
# define __const
# endif

# if (defined __STDC__ && __STDC__) || defined __cplusplus

#  define __MALLOC_P(args)      args __THROW
#  define __MALLOC_PMT(args)    args

# else  /* Not ANSI C or C++.  */

#  define __MALLOC_P(args)      ()      /* No prototypes.  */
#  define __MALLOC_PMT(args)    ()

# endif /* ANSI C or C++.  */

#endif  /* GCC.  */

#ifndef NULL
# ifdef __cplusplus
#  define NULL  0
# else
#  define NULL  ((__malloc_ptr_t) 0)
# endif
#endif

#if defined(__cplusplus) && !defined(PTMALLOC_IN_CPPNAMESPACE)
extern "C" {
#endif


/* Returns a copy of the updated current mallinfo. */
extern struct mallinfo mallinfo __MALLOC_P ((void));

/* SVID2/XPG mallopt options */
#ifndef M_MXFAST
# define M_MXFAST  1    /* maximum request size for "fastbins" */
#endif
#ifndef M_NLBLKS
# define M_NLBLKS  2    /* UNUSED in this malloc */
#endif
#ifndef M_GRAIN
# define M_GRAIN   3    /* UNUSED in this malloc */
#endif
#ifndef M_KEEP
# define M_KEEP    4    /* UNUSED in this malloc */
#endif

/* mallopt options that actually do something */
#define M_TRIM_THRESHOLD    -1
#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3
#define M_MMAP_MAX          -4
#define M_CHECK_ACTION      -5

/* General SVID/XPG interface to tunable parameters. */
extern int mallopt __MALLOC_P ((int __param, int __val));

/* Release all but __pad bytes of freed top-most memory back to the
   system. Return 1 if successful, else 0. */
extern int malloc_trim __MALLOC_P ((size_t __pad));

/* Report the number of usable allocated bytes associated with allocated
   chunk __ptr. */
extern size_t malloc_usable_size __MALLOC_P ((__malloc_ptr_t __ptr));

/* Prints brief summary statistics on stderr. */
extern void malloc_stats __MALLOC_P ((void));

/* Record the state of all malloc variables in an opaque data structure. */
extern __malloc_ptr_t malloc_get_state __MALLOC_P ((void));

/* Restore the state of all malloc variables from data obtained with
   malloc_get_state(). */
extern int malloc_set_state __MALLOC_P ((__malloc_ptr_t __ptr));

/* Called once when malloc is initialized; redefining this variable in
   the application provides the preferred way to set up the hook
   pointers. */
extern void (*__malloc_initialize_hook) __MALLOC_PMT ((void));
/* Hooks for debugging and user-defined versions. */
extern void (*__free_hook) __MALLOC_PMT ((__malloc_ptr_t __ptr,
                                        __const __malloc_ptr_t));
extern __malloc_ptr_t (*__malloc_hook) __MALLOC_PMT ((size_t __size,
                                                    __const __malloc_ptr_t));
extern __malloc_ptr_t (*__realloc_hook) __MALLOC_PMT ((__malloc_ptr_t __ptr,
                                                     size_t __size,
                                                     __const __malloc_ptr_t));
extern __malloc_ptr_t (*__memalign_hook) __MALLOC_PMT ((size_t __alignment,
                                                      size_t __size,
                                                      __const __malloc_ptr_t));
extern void (*__after_morecore_hook) __MALLOC_PMT ((void));

/* Activate a standard set of debugging hooks. */
extern void __malloc_check_init __MALLOC_P ((void));

/* Internal routines, operating on "arenas".  */
struct malloc_state;
typedef struct malloc_state *mstate;

extern mstate         _int_new_arena __MALLOC_P ((size_t __ini_size));
extern __malloc_ptr_t _int_malloc __MALLOC_P ((mstate __m, size_t __size));
extern void           _int_free __MALLOC_P ((mstate __m, __malloc_ptr_t __ptr));
extern __malloc_ptr_t _int_realloc __MALLOC_P ((mstate __m,
                                                __malloc_ptr_t __ptr,
                                                size_t __size));
extern __malloc_ptr_t _int_memalign __MALLOC_P ((mstate __m, size_t __alignment,
                                                 size_t __size));

#if defined(__cplusplus) && !defined(PTMALLOC_IN_CPPNAMESPACE)
}; /* end of extern "C" */
#endif

#endif /* malloc.h */

/*************************** ptmalloc2.c ******************************/

/* 
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it 
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

#ifndef BOUNDED_N
#define BOUNDED_N(ptr, sz) (ptr)
#endif
#ifndef RETURN_ADDRESS
#define RETURN_ADDRESS(X_) (nullptr)
#endif

/* On some platforms we can compile internal, not exported functions better.
   Let the environment provide a macro and define it to be empty if it
   is not available.  */
#ifndef internal_function
# define internal_function
#endif

/* Forward declarations.  */
struct malloc_chunk;
typedef struct malloc_chunk* mchunkptr;

/* Internal routines.  */

#if __STD_C

//Void_t*         _int_malloc(mstate, size_t);
//void            _int_free(mstate, Void_t*);
//Void_t*         _int_realloc(mstate, Void_t*, size_t);
//Void_t*         _int_memalign(mstate, size_t, size_t);
Void_t*         _int_valloc(mstate, size_t);
static Void_t*  _int_pvalloc(mstate, size_t);
/*static Void_t*  cALLOc(size_t, size_t);*/
static Void_t** _int_icalloc(mstate, size_t, size_t, Void_t**);
static Void_t** _int_icomalloc(mstate, size_t, size_t*, Void_t**);
static int      mTRIm(size_t);
static size_t   mUSABLe(Void_t*);
static void     mSTATs(void);
static int      mALLOPt(int, int);
static struct mallinfo mALLINFo(mstate);

static Void_t* internal_function mem2mem_check(Void_t *p, size_t sz);
static int internal_function top_check(void);
static void internal_function munmap_chunk(mchunkptr p);
#if HAVE_MREMAP
static mchunkptr internal_function mremap_chunk(mchunkptr p, size_t new_size);
#endif

static Void_t*   malloc_check(size_t sz, const Void_t *caller);
static void      free_check(Void_t* mem, const Void_t *caller);
static Void_t*   realloc_check(Void_t* oldmem, size_t bytes,
                               const Void_t *caller);
static Void_t*   memalign_check(size_t alignment, size_t bytes,
                                const Void_t *caller);
#ifndef NO_THREADS
static Void_t*   malloc_starter(size_t sz, const Void_t *caller);
static void      free_starter(Void_t* mem, const Void_t *caller);
static Void_t*   malloc_atfork(size_t sz, const Void_t *caller);
static void      free_atfork(Void_t* mem, const Void_t *caller);
#endif

#else

Void_t*         _int_malloc();
void            _int_free();
Void_t*         _int_realloc();
Void_t*         _int_memalign();
Void_t*         _int_valloc();
Void_t*         _int_pvalloc();
/*static Void_t*  cALLOc();*/
static Void_t** _int_icalloc();
static Void_t** _int_icomalloc();
static int      mTRIm();
static size_t   mUSABLe();
static void     mSTATs();
static int      mALLOPt();
static struct mallinfo mALLINFo();

#endif


/* ------------- Optional versions of memcopy ---------------- */


#if USE_MEMCPY

/* 
  Note: memcpy is ONLY invoked with non-overlapping regions,
  so the (usually slower) memmove is not needed.
*/

#define MALLOC_COPY(dest, src, nbytes)  memcpy(dest, src, nbytes)
#define MALLOC_ZERO(dest, nbytes)       memset(dest, 0,   nbytes)

#else /* !USE_MEMCPY */

/* Use Duff's device for good zeroing/copying performance. */

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T* mzp = (INTERNAL_SIZE_T*)(charp);                           \
  CHUNK_SIZE_T  mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T);                     \
  long mcn;                                                                   \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mzp++ = 0;                                             \
    case 7:           *mzp++ = 0;                                             \
    case 6:           *mzp++ = 0;                                             \
    case 5:           *mzp++ = 0;                                             \
    case 4:           *mzp++ = 0;                                             \
    case 3:           *mzp++ = 0;                                             \
    case 2:           *mzp++ = 0;                                             \
    case 1:           *mzp++ = 0; if(mcn <= 0) break; mcn--; }                \
  }                                                                           \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) src;                            \
  INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) dest;                           \
  CHUNK_SIZE_T  mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T);                     \
  long mcn;                                                                   \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mcdst++ = *mcsrc++;                                    \
    case 7:           *mcdst++ = *mcsrc++;                                    \
    case 6:           *mcdst++ = *mcsrc++;                                    \
    case 5:           *mcdst++ = *mcsrc++;                                    \
    case 4:           *mcdst++ = *mcsrc++;                                    \
    case 3:           *mcdst++ = *mcsrc++;                                    \
    case 2:           *mcdst++ = *mcsrc++;                                    \
    case 1:           *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }       \
  }                                                                           \
} while(0)

#endif

/* ------------------ MMAP support ------------------  */


#if HAVE_MMAP

#ifndef LACKS_FCNTL_H
#include <fcntl.h>
#endif

#ifndef LACKS_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
# define MAP_ANONYMOUS MAP_ANON
#endif
#if !defined(MAP_FAILED)
# define MAP_FAILED ((char*)-1)
#endif

#ifndef MAP_NORESERVE
# ifdef MAP_AUTORESRV
#  define MAP_NORESERVE MAP_AUTORESRV
# else
#  define MAP_NORESERVE 0
# endif
#endif

/*
   Nearly all versions of mmap support MAP_ANONYMOUS,
   so the following is unlikely to be needed, but is
   supplied just in case.
*/

#ifndef MAP_ANONYMOUS

static int dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */

#define MMAP(addr, size, prot, flags) ((dev_zero_fd < 0) ? \
 (dev_zero_fd = open("/dev/zero", O_RDWR), \
  mmap((addr), (size), (prot), (flags), dev_zero_fd, 0)) : \
   mmap((addr), (size), (prot), (flags), dev_zero_fd, 0))

#else

#define MMAP(addr, size, prot, flags) \
 (mmap((addr), (size), (prot), (flags)|MAP_ANONYMOUS, -1, 0))

#endif


#endif /* HAVE_MMAP */


/*
  -----------------------  Chunk representations -----------------------
*/


/*
  This struct declaration is misleading (but accurate and necessary).
  It declares a "view" into memory allowing access to necessary
  fields at known offsets from a given base. See explanation below.
*/

struct malloc_chunk {

  INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};


typedef struct malloc_chunk* mchunkptr;

/*
   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:


    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk, if allocated            | |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             User data starts here...                          .
            .                                                               .
            .             (malloc_usable_space() bytes)                     .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk                                     |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus at least double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory. If
    prev_inuse is set for any given chunk, then you CANNOT determine
    the size of the previous chunk, and might even get a memory
    addressing fault when trying to do so.

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. This makes it easier to
    deal with alignments etc but can be very confusing when trying
    to extend or adapt this code.

    The two exceptions to all this are

     1. The special chunk `top' doesn't bother using the
        trailing size field since there is no next contiguous chunk
        that would have to index off it. After initialization, `top'
        is forced to always exist.  If it would become less than
        MINSIZE bytes long, it is replenished.

     2. Chunks allocated via mmap, which have the second-lowest-order
        bit (IS_MMAPPED) set in their size fields.  Because they are
        allocated one-by-one, each must contain its own trailing size field.

*/

/*
  ---------- Size and alignment checks and conversions ----------
*/

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((Void_t*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (sizeof(struct malloc_chunk))

/* The smallest size we can malloc is an aligned minimal chunk */

#define MINSIZE  \
  (CHUNK_SIZE_T)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

/* Check if m has acceptable alignment */

#define aligned_OK(m)  (((PTR_UINT)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/* 
   Check if a request is so large that it would wrap around zero when
   padded and aligned. To simplify some other code, the bound is made
   low enough so that adding MINSIZE will also not wrap around sero.
*/

#define REQUEST_OUT_OF_RANGE(req)                                 \
  ((CHUNK_SIZE_T)(req) >=                                        \
   (CHUNK_SIZE_T)(INTERNAL_SIZE_T)(-2 * MINSIZE))    

/* pad request bytes into a usable size -- internal version */

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/*  Same, except also perform argument check */

#define checked_request2size(req, sz)                             \
  if (REQUEST_OUT_OF_RANGE(req)) {                                \
    MALLOC_FAILURE_ACTION;                                        \
    return 0;                                                     \
  }                                                               \
  (sz) = request2size(req);                                              

/*
  --------------- Physical chunk operations ---------------
*/


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
#define PREV_INUSE 0x1

/* extract inuse bit of previous chunk */
#define prev_inuse(p)       ((p)->size & PREV_INUSE)


/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
#define IS_MMAPPED 0x2

/* check for mmap()'ed chunk */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)


/* size field is or'ed with NON_MAIN_ARENA if the chunk was obtained
   from a non-main arena.  This is only set immediately before handing
   the chunk to the user, if necessary.  */
#define NON_MAIN_ARENA 0x4

/* check for chunk from non-main arena */
#define chunk_non_main_arena(p) ((p)->size & NON_MAIN_ARENA)


/*
  Bits to mask off when extracting size

  Note: IS_MMAPPED is intentionally not masked off from size field in
  macros for which mmapped chunks should never be seen. This should
  cause helpful core dumps to occur if it is tried by accident by
  people extending or adapting this malloc.
*/
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED|NON_MAIN_ARENA)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))


/* Ptr to next physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~SIZE_BITS) ))

/* Ptr to previous physical malloc_chunk */
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */
#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* extract p's inuse bit */
#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~SIZE_BITS)))->size) & PREV_INUSE)

/* set/clear chunk as being inuse without otherwise disturbing */
#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~SIZE_BITS)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~SIZE_BITS)))->size &= ~(PREV_INUSE)


/* check/set/clear inuse bits in known places */
#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))


/* Set size at head, without disturbing its use bit */
#define set_head_size(p, s)  ((p)->size = (((p)->size & SIZE_BITS) | (s)))

/* Set size/use field */
#define set_head(p, s)       ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */
#define set_foot(p, s)       (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))


/*
  -------------------- Internal data structures --------------------

   All internal state is held in an instance of malloc_state defined
   below. There are no other static variables, except in two optional
   cases:
   * If HAVE_MMAP is true, but mmap doesn't support
     MAP_ANONYMOUS, a dummy file descriptor for mmap.

   Beware of lots of tricks that minimize the total bookkeeping space
   requirements. The result is a little over 1K bytes (for 4byte
   pointers and size_t.)
*/

/*
  Bins

    An array of bin headers for free chunks. Each bin is doubly
    linked.  The bins are approximately proportionally (log) spaced.
    There are a lot of these bins (128). This may look excessive, but
    works very well in practice.  Most bins hold sizes that are
    unusual as malloc request sizes, but are more usual for fragments
    and consolidated sets of chunks, which is what these bins hold, so
    they can be found quickly.  All procedures maintain the invariant
    that no consolidated chunk physically borders another one, so each
    chunk in a list is known to be preceeded and followed by either
    inuse chunks or the ends of memory.

    Chunks in bins are kept in size order, with ties going to the
    approximately least recently used chunk. Ordering isn't needed
    for the small bins, which all contain the same-sized chunks, but
    facilitates best-fit allocation for larger chunks. These lists
    are just sequential. Keeping them in order almost never requires
    enough traversal to warrant using fancier ordered data
    structures.  

    Chunks of the same size are linked with the most
    recently freed at the front, and allocations are taken from the
    back.  This results in LRU (FIFO) allocation order, which tends
    to give each chunk an equal opportunity to be consolidated with
    adjacent freed chunks, resulting in larger free chunks and less
    fragmentation.

    To simplify use in double-linked lists, each bin header acts
    as a malloc_chunk. This avoids special-casing for headers.
    But to conserve space and improve locality, we allocate
    only the fd/bk pointers of bins, and then use repositioning tricks
    to treat these as the fields of a malloc_chunk*.  
*/

typedef struct malloc_chunk* mbinptr;

/* addressing -- note that bin_at(0) does not exist */
#define bin_at(m, i) ((mbinptr)((char*)&((m)->bins[(i)<<1]) - (SIZE_SZ<<1)))

/* analog of ++bin */
#define next_bin(b)  ((mbinptr)((char*)(b) + (sizeof(mchunkptr)<<1)))

/* Reminders about list directionality within bins */
#define first(b)     ((b)->fd)
#define last(b)      ((b)->bk)

/* Take a chunk off a bin list */
#define unlink(P, BK, FD) {                                            \
  FD = P->fd;                                                          \
  BK = P->bk;                                                          \
  FD->bk = BK;                                                         \
  BK->fd = FD;                                                         \
}

/*
  Indexing

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically spaced:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    There is actually a little bit of slop in the numbers in bin_index
    for the sake of speed. This makes no difference elsewhere.

    The bins top out around 1MB because we expect to service large
    requests via mmap.
*/

#define NBINS             128
#define NSMALLBINS         64
#define SMALLBIN_WIDTH      8
#define MIN_LARGE_SIZE    512

#define in_smallbin_range(sz)  \
  ((CHUNK_SIZE_T)(sz) < (CHUNK_SIZE_T)MIN_LARGE_SIZE)

#define smallbin_index(sz)     (((unsigned)(sz)) >> 3)

/*
  Compute index for size. We expect this to be inlined when
  compiled with optimization, else not, which works out well.
*/
static int largebin_index(unsigned int sz) {
  unsigned int  x = sz >> SMALLBIN_WIDTH; 
  unsigned int m;            /* bit position of highest set bit of m */

  if (x >= 0x10000) return NBINS-1;

  /* On intel, use BSRL instruction to find highest bit */
#if defined(__GNUC__) && defined(i386)

  __asm__("bsrl %1,%0\n\t"
          : "=r" (m) 
          : "g"  (x));

#elif defined(_MSC_VER) && defined(_M_IX86)
  __asm
  {
          bsr eax, [x]
          mov [m], eax
  }
#else
  {
    /*
      Based on branch-free nlz algorithm in chapter 5 of Henry
      S. Warren Jr's book "Hacker's Delight".
    */

    unsigned int n = ((x - 0x100) >> 16) & 8;
    x <<= n; 
    m = ((x - 0x1000) >> 16) & 4;
    n += m; 
    x <<= m; 
    m = ((x - 0x4000) >> 16) & 2;
    n += m; 
    x = (x << m) >> 14;
    m = 13 - n + (x & ~(x>>1));
  }
#endif

  /* Use next 2 bits to create finer-granularity bins */
  return NSMALLBINS + (m << 2) + ((sz >> (m + 6)) & 3);
}

#define bin_index(sz) \
 ((in_smallbin_range(sz)) ? smallbin_index(sz) : largebin_index(sz))

/*
  FIRST_SORTED_BIN_SIZE is the chunk size corresponding to the
  first bin that is maintained in sorted order. This must
  be the smallest size corresponding to a given bin.

  Normally, this should be MIN_LARGE_SIZE. But you can weaken
  best fit guarantees to sometimes speed up malloc by increasing value.
  Doing this means that malloc may choose a chunk that is 
  non-best-fitting by up to the width of the bin.

  Some useful cutoff values:
      512 - all bins sorted
     2560 - leaves bins <=     64 bytes wide unsorted  
    12288 - leaves bins <=    512 bytes wide unsorted
    65536 - leaves bins <=   4096 bytes wide unsorted
   262144 - leaves bins <=  32768 bytes wide unsorted
       -1 - no bins sorted (not recommended!)
*/

#define FIRST_SORTED_BIN_SIZE MIN_LARGE_SIZE 
/* #define FIRST_SORTED_BIN_SIZE 65536 */

/*
  Unsorted chunks

    All remainders from chunk splits, as well as all returned chunks,
    are first placed in the "unsorted" bin. They are then placed
    in regular bins after malloc gives them ONE chance to be used before
    binning. So, basically, the unsorted_chunks list acts as a queue,
    with chunks being placed on it in free (and malloc_consolidate),
    and taken off (to be either used or placed in bins) in malloc.

    The NON_MAIN_ARENA flag is never set for unsorted chunks, so it
    does not have to be taken into account in size comparisons.
*/

/* The otherwise unindexable 1-bin is used to hold unsorted chunks. */
#define unsorted_chunks(M)          (bin_at(M, 1))

/*
  Top

    The top-most available chunk (i.e., the one bordering the end of
    available memory) is treated specially. It is never included in
    any bin, is used only if no other chunk is available, and is
    released back to the system if it is very large (see
    M_TRIM_THRESHOLD).  Because top initially
    points to its own bin with initial zero size, thus forcing
    extension on the first malloc request, we avoid having any special
    code in malloc to check whether it even exists yet. But we still
    need to do so when getting memory from system, so we make
    initial_top treat the bin as a legal but unusable chunk during the
    interval between initialization and the first call to
    sYSMALLOc. (This is somewhat delicate, since it relies on
    the 2 preceding words to be zero during this interval as well.)
*/

/* Conveniently, the unsorted bin can be used as dummy top on first call */
#define initial_top(M)              (unsorted_chunks(M))

/*
  Binmap

    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binmap' is a
    bitvector recording whether bins are definitely empty so they can
    be skipped over during during traversals.  The bits are NOT always
    cleared as soon as bins are empty, but instead only
    when they are noticed to be empty during traversal in malloc.
*/

/* Conservatively use 32 bits per map word, even if on 64bit system */
#define BINMAPSHIFT      5
#define BITSPERMAP       (1U << BINMAPSHIFT)
#define BINMAPSIZE       (NBINS / BITSPERMAP)

#define idx2block(i)     ((i) >> BINMAPSHIFT)
#define idx2bit(i)       ((1U << ((i) & ((1U << BINMAPSHIFT)-1))))

#define mark_bin(m,i)    ((m)->binmap[idx2block(i)] |=  idx2bit(i))
#define unmark_bin(m,i)  ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
#define get_binmap(m,i)  ((m)->binmap[idx2block(i)] &   idx2bit(i))

/*
  Fastbins

    An array of lists holding recently freed small chunks.  Fastbins
    are not doubly linked.  It is faster to single-link them, and
    since chunks are never removed from the middles of these lists,
    double linking is not necessary. Also, unlike regular bins, they
    are not even processed in FIFO order (they use faster LIFO) since
    ordering doesn't much matter in the transient contexts in which
    fastbins are normally used.

    Chunks in fastbins keep their inuse bit set, so they cannot
    be consolidated with other free chunks. malloc_consolidate
    releases all chunks in fastbins and consolidates them with
    other free chunks. 
*/

typedef struct malloc_chunk* mfastbinptr;

/* offset 2 to use otherwise unindexable first 2 bins */
#define fastbin_index(sz)        ((((unsigned int)(sz)) >> 3) - 2)

/* The maximum fastbin request size we support */
#define MAX_FAST_SIZE     80

#define NFASTBINS  (fastbin_index(request2size(MAX_FAST_SIZE))+1)

/*
  FASTBIN_CONSOLIDATION_THRESHOLD is the size of a chunk in free()
  that triggers automatic consolidation of possibly-surrounding
  fastbin chunks. This is a heuristic, so the exact value should not
  matter too much. It is defined at half the default trim threshold as a
  compromise heuristic to only attempt consolidation if it is likely
  to lead to trimming. However, it is not dynamically tunable, since
  consolidation reduces fragmentation surrounding loarge chunks even 
  if trimming is not used.
*/

#define FASTBIN_CONSOLIDATION_THRESHOLD  \
  ((unsigned long)(DEFAULT_TRIM_THRESHOLD) >> 1)

/*
  Since the lowest 2 bits in max_fast don't matter in size comparisons, 
  they are used as flags.
*/

/*
  ANYCHUNKS_BIT held in max_fast indicates that there may be any
  freed chunks at all. It is set true when entering a chunk into any
  bin.
*/

#define ANYCHUNKS_BIT        (1U)

#define have_anychunks(M)     (((M)->max_fast &  ANYCHUNKS_BIT) == 0)
#define clear_anychunks(M)    ((M)->max_fast |=  ANYCHUNKS_BIT)
#define set_anychunks(M)      ((M)->max_fast &= ~ANYCHUNKS_BIT)

/*
  FASTCHUNKS_BIT held in max_fast indicates that there are probably
  some fastbin chunks. It is set true on entering a chunk into any
  fastbin, and cleared only in malloc_consolidate.
*/

#define FASTCHUNKS_BIT        (2U)

#define have_fastchunks(M)   (((M)->max_fast &  FASTCHUNKS_BIT) == 0)
#define clear_fastchunks(M)  ((M)->max_fast |=  (FASTCHUNKS_BIT))
#define set_fastchunks(M)    ((M)->max_fast &= ~(FASTCHUNKS_BIT|ANYCHUNKS_BIT))

/* 
   Set value of max_fast. 
   Use impossibly small value if 0.
*/

#define set_max_fast(M, s) \
  (M)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  ((M)->max_fast &  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))

#define get_max_fast(M) \
  ((M)->max_fast & ~(FASTCHUNKS_BIT | ANYCHUNKS_BIT))


/*
  morecore_properties is a status word holding dynamically discovered
  or controlled properties of the morecore function
*/

#define MORECORE_CONTIGUOUS_BIT  (1U)

#define contiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT) == 0)
#define noncontiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT) != 0)
#define set_noncontiguous(M) \
        ((M)->morecore_properties |=  MORECORE_CONTIGUOUS_BIT)
#define set_contiguous(M) \
        ((M)->morecore_properties &= ~MORECORE_CONTIGUOUS_BIT)


/*
   ----------- Internal state representation and initialization -----------
*/

struct malloc_state {
  /* Serialize access.  */
  mutex_t mutex;

  /* Statistics for locking.  Only used if THREAD_STATS is defined.  */
  long stat_lock_direct, stat_lock_loop, stat_lock_wait;
  long pad0_[1]; /* try to give the mutex its own cacheline */

  /* The maximum chunk size to be eligible for fastbin */
  INTERNAL_SIZE_T  max_fast;   /* low 2 bits used as flags */

  /* Track properties of MORECORE */
  unsigned int     morecore_properties;

  /* Fastbins */
  mfastbinptr      fastbins[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr        top;

  /* The remainder from the most recent split of a small request */
  mchunkptr        last_remainder;

  /* Normal bins packed as described above */
  mchunkptr        bins[NBINS * 2];

  /* Bitmap of bins. Trailing zero map handles cases of largest binned size */
  unsigned int     binmap[BINMAPSIZE+1];

  /* Linked list */
  struct malloc_state *next;

  /* Memory allocated from the system in this arena.  */
  INTERNAL_SIZE_T system_mem;
  INTERNAL_SIZE_T max_system_mem;
};

struct malloc_par {
  /* Tunable parameters */
  CHUNK_SIZE_T     trim_threshold;
  INTERNAL_SIZE_T  top_pad;
  INTERNAL_SIZE_T  mmap_threshold;

  /* Memory map support */
  int              n_mmaps;
  int              n_mmaps_max;
  int              max_n_mmaps;

  /* Cache malloc_getpagesize */
  unsigned int     pagesize;

  /* Statistics */
  INTERNAL_SIZE_T  mmapped_mem;
  /*INTERNAL_SIZE_T  sbrked_mem;*/
  /*INTERNAL_SIZE_T  max_sbrked_mem;*/
  INTERNAL_SIZE_T  max_mmapped_mem;
  INTERNAL_SIZE_T  max_total_mem; /* only kept for NO_THREADS */

  /* First address handed out by MORECORE/sbrk.  */
  char*            sbrk_base;
};

typedef struct malloc_state *mstate;

/* There are several instances of this struct ("arenas") in this
   malloc.  If you are adapting this malloc in a way that does NOT use
   a static or mmapped malloc_state, you MUST explicitly zero-fill it
   before using. This malloc relies on the property that malloc_state
   is initialized to all zeroes (as is true of C statics).  */

static struct malloc_state main_arena;

/* There is only one instance of the malloc parameters.  */

static struct malloc_par mp_;

/*
  Initialize a malloc_state struct.

  This is called only from within malloc_consolidate, which needs
  be called in the same contexts anyway.  It is never called directly
  outside of malloc_consolidate because some optimizing compilers try
  to inline it at all call points, which turns out not to be an
  optimization at all. (Inlining it in malloc_consolidate is fine though.)
*/

#if __STD_C
static void malloc_init_state(mstate av)
#else
static void malloc_init_state(av) mstate av;
#endif
{
  int     i;
  mbinptr bin;

  /* Establish circular links for normal bins */
  for (i = 1; i < NBINS; ++i) {
    bin = bin_at(av,i);
    bin->fd = bin->bk = bin;
  }

#if MORECORE_CONTIGUOUS
  if (av != &main_arena)
#endif
    set_noncontiguous(av);

  set_max_fast(av, DEFAULT_MXFAST);

  av->top            = initial_top(av);
}

/*
   Other internal utilities operating on mstates
*/

#if __STD_C
static Void_t*  sYSMALLOc(INTERNAL_SIZE_T, mstate);
static int      sYSTRIm(size_t, mstate);
static void     malloc_consolidate(mstate);
static Void_t** iALLOc(mstate, size_t, size_t*, int, Void_t**);
#else
static Void_t*  sYSMALLOc();
static int      sYSTRIm();
static void     malloc_consolidate();
static Void_t** iALLOc();
#endif

/* ------------------- Support for multiple arenas -------------------- */



/*************************** arena.c ******************************/

/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* $Id$ */

/* Compile-time constants.  */

#define HEAP_MIN_SIZE (32*1024)
#ifndef HEAP_MAX_SIZE
#define HEAP_MAX_SIZE (1024*1024) /* must be a power of two */
#endif

/* HEAP_MIN_SIZE and HEAP_MAX_SIZE limit the size of mmap()ed heaps
   that are dynamically created for multi-threaded programs.  The
   maximum size must be a power of two, for fast determination of
   which heap belongs to a chunk.  It should be much larger than the
   mmap threshold, so that requests with a size just below that
   threshold can be fulfilled without creating too many heaps.  */


#ifndef THREAD_STATS
#define THREAD_STATS 0
#endif

/* If THREAD_STATS is non-zero, some statistics on mutex locking are
   computed.  */

/***************************************************************************/

#define top(ar_ptr) ((ar_ptr)->top)

/* A heap is a single contiguous memory region holding (coalesceable)
   malloc_chunks.  It is allocated with mmap() and always starts at an
   address aligned to HEAP_MAX_SIZE.  Not used unless compiling with
   USE_ARENAS. */

typedef struct _heap_info {
  mstate ar_ptr; /* Arena for this heap. */
  struct _heap_info *prev; /* Previous heap. */
  size_t size;   /* Current size in bytes. */
  size_t pad;    /* Make sure the following data is properly aligned. */
} heap_info;

/* Thread specific data */

static tsd_key_t arena_key;
static mutex_t list_lock;

#if THREAD_STATS
static int stat_n_heaps;
#define THREAD_STAT(x) x
#else
#define THREAD_STAT(x) do ; while(0)
#endif

/* Mapped memory in non-main arenas (reliable only for NO_THREADS). */
static unsigned long arena_mem;

/* Already initialized? */
int __malloc_initialized = -1;

/**************************************************************************/

#if USE_ARENAS

/* arena_get() acquires an arena and locks the corresponding mutex.
   First, try the one last locked successfully by this thread.  (This
   is the common case and handled with a macro for speed.)  Then, loop
   once over the circularly linked list of arenas.  If no arena is
   readily available, create a new one.  In this latter case, `size'
   is just a hint as to how much memory will be required immediately
   in the new arena. */

#define arena_get(ptr, size) do { \
  Void_t *vptr = nullptr; \
  ptr = (mstate)tsd_getspecific(arena_key, vptr); \
  if(ptr && !mutex_trylock(&ptr->mutex)) { \
    THREAD_STAT(++(ptr->stat_lock_direct)); \
  } else \
    ptr = arena_get2(ptr, (size)); \
} while(0)

/* find the heap and corresponding arena for a given ptr */

#define heap_for_ptr(ptr) \
 ((heap_info *)((unsigned long)(ptr) & ~(HEAP_MAX_SIZE-1)))
#define arena_for_chunk(ptr) \
 (chunk_non_main_arena(ptr) ? heap_for_ptr(ptr)->ar_ptr : &main_arena)

#else /* !USE_ARENAS */

/* There is only one arena, main_arena. */

#if THREAD_STATS
#define arena_get(ar_ptr, sz) do { \
  ar_ptr = &main_arena; \
  if(!mutex_trylock(&ar_ptr->mutex)) \
    ++(ar_ptr->stat_lock_direct); \
  else { \
    (void)mutex_lock(&ar_ptr->mutex); \
    ++(ar_ptr->stat_lock_wait); \
  } \
} while(0)
#else
#define arena_get(ar_ptr, sz) do { \
  ar_ptr = &main_arena; \
  (void)mutex_lock(&ar_ptr->mutex); \
} while(0)
#endif
#define arena_for_chunk(ptr) (&main_arena)

#endif /* USE_ARENAS */

/**************************************************************************/

#ifndef NO_THREADS

/* atfork support.  */

static __malloc_ptr_t (*save_malloc_hook) __MALLOC_P ((size_t __size,
                                                       __const __malloc_ptr_t));
static void           (*save_free_hook) __MALLOC_P ((__malloc_ptr_t __ptr,
                                                     __const __malloc_ptr_t));
static Void_t*        save_arena;

/* Magic value for the thread-specific arena pointer when
   malloc_atfork() is in use.  */

#define ATFORK_ARENA_PTR ((Void_t*)-1)

/* The following hooks are used while the `atfork' handling mechanism
   is active. */

static Void_t*
malloc_atfork(size_t sz, const Void_t *caller)
{
  Void_t *vptr = nullptr;
  Void_t *victim;

  tsd_getspecific(arena_key, vptr);
  if(vptr == ATFORK_ARENA_PTR) {
    /* We are the only thread that may allocate at all.  */
    if(save_malloc_hook != malloc_check) {
      return _int_malloc(&main_arena, sz);
    } else {
      if(top_check()<0)
        return 0;
      victim = _int_malloc(&main_arena, sz+1);
      return mem2mem_check(victim, sz);
    }
  } else {
    /* Suspend the thread until the `atfork' handlers have completed.
       By that time, the hooks will have been reset as well, so that
       mALLOc() can be used again. */
    (void)mutex_lock(&list_lock);
    (void)mutex_unlock(&list_lock);
    return public_mALLOc(sz);
  }
}

static void
free_atfork(Void_t* mem, const Void_t *caller)
{
  Void_t *vptr = nullptr;
  mstate ar_ptr;
  mchunkptr p;                          /* chunk corresponding to mem */

  if (mem == 0)                              /* free(0) has no effect */
    return;

  p = mem2chunk(mem);         /* do not bother to replicate free_check here */

#if HAVE_MMAP
  if (chunk_is_mmapped(p))                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    return;
  }
#endif

  ar_ptr = arena_for_chunk(p);
  tsd_getspecific(arena_key, vptr);
  if(vptr != ATFORK_ARENA_PTR)
    (void)mutex_lock(&ar_ptr->mutex);
  _int_free(ar_ptr, mem);
  if(vptr != ATFORK_ARENA_PTR)
    (void)mutex_unlock(&ar_ptr->mutex);
}

/* The following two functions are registered via thread_atfork() to
   make sure that the mutexes remain in a consistent state in the
   fork()ed version of a thread.  Also adapt the malloc and free hooks
   temporarily, because the `atfork' handler mechanism may use
   malloc/free internally (e.g. in LinuxThreads). */

static void
ptmalloc_lock_all __MALLOC_P((void))
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
  (void)mutex_lock(&list_lock);
  for(ar_ptr = &main_arena;;) {
    (void)mutex_lock(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  save_malloc_hook = __malloc_hook;
  save_free_hook = __free_hook;
  __malloc_hook = malloc_atfork;
  __free_hook = free_atfork;
  /* Only the current thread may perform malloc/free calls now. */
  tsd_getspecific(arena_key, save_arena);
  tsd_setspecific(arena_key, ATFORK_ARENA_PTR);
}

static void
ptmalloc_unlock_all __MALLOC_P((void))
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
  tsd_setspecific(arena_key, save_arena);
  __malloc_hook = save_malloc_hook;
  __free_hook = save_free_hook;
  for(ar_ptr = &main_arena;;) {
    (void)mutex_unlock(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  (void)mutex_unlock(&list_lock);
}

#ifdef __linux__

/* In LinuxThreads, unlocking a mutex in the child process after a
   fork() is currently unsafe, whereas re-initializing it is safe and
   does not leak resources.  Therefore, a special atfork handler is
   installed for the child. */

static void
ptmalloc_unlock_all2 __MALLOC_P((void))
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
#if defined _LIBC || defined MALLOC_HOOKS
  tsd_setspecific(arena_key, save_arena);
  __malloc_hook = save_malloc_hook;
  __free_hook = save_free_hook;
#endif
  for(ar_ptr = &main_arena;;) {
    (void)mutex_init(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  (void)mutex_init(&list_lock);
}

#else

#define ptmalloc_unlock_all2 ptmalloc_unlock_all

#endif

#endif /* !defined NO_THREADS */

/* Initialization routine. */
#ifdef _LIBC
#include <string.h>
extern char **_environ;

static char *
internal_function
next_env_entry (char ***position)
{
  char **current = *position;
  char *result = nullptr;

  while (*current != nullptr)
    {
      if (__builtin_expect ((*current)[0] == 'M', 0)
          && (*current)[1] == 'A'
          && (*current)[2] == 'L'
          && (*current)[3] == 'L'
          && (*current)[4] == 'O'
          && (*current)[5] == 'C'
          && (*current)[6] == '_')
        {
          result = &(*current)[7];

          /* Save current position for next visit.  */
          *position = ++current;

          break;
        }

      ++current;
    }

  return result;
}
#endif /* _LIBC */

static void
ptmalloc_init __MALLOC_P((void))
{
#if __STD_C
  const char* s;
#else
  char* s;
#endif
  int secure = 0;

  if(__malloc_initialized >= 0) return;
  __malloc_initialized = 0;

  mp_.top_pad        = DEFAULT_TOP_PAD;
  mp_.n_mmaps_max    = DEFAULT_MMAP_MAX;
  mp_.mmap_threshold = DEFAULT_MMAP_THRESHOLD;
  mp_.trim_threshold = DEFAULT_TRIM_THRESHOLD;
  mp_.pagesize       = malloc_getpagesize;

#ifndef NO_THREADS
  /* With some threads implementations, creating thread-specific data
     or initializing a mutex may call malloc() itself.  Provide a
     simple starter version (realloc() won't work). */
  save_malloc_hook = __malloc_hook;
  save_free_hook = __free_hook;
  __malloc_hook = malloc_starter;
  __free_hook = free_starter;
#ifdef _LIBC
  /* Initialize the pthreads interface. */
  if (__pthread_initialize != nullptr)
    __pthread_initialize();
#endif
#endif /* !defined NO_THREADS */
  mutex_init(&main_arena.mutex);
  main_arena.next = &main_arena;

  mutex_init(&list_lock);
  tsd_key_create(&arena_key, nullptr);
  tsd_setspecific(arena_key, (Void_t *)&main_arena);
  thread_atfork(ptmalloc_lock_all, ptmalloc_unlock_all, ptmalloc_unlock_all2);
#ifndef NO_THREADS
  __malloc_hook = save_malloc_hook;
  __free_hook = save_free_hook;
#endif
#ifdef _LIBC
  secure = __libc_enable_secure;
  s = nullptr;
  {
    char **runp = _environ;
    char *envline;

    while (__builtin_expect ((envline = next_env_entry (&runp)) != nullptr,
                             0))
      {
        size_t len = strcspn (envline, "=");

        if (envline[len] != '=')
          /* This is a "MALLOC_" variable at the end of the string
             without a '=' character.  Ignore it since otherwise we
             will access invalid memory below.  */
          continue;

        switch (len)
          {
          case 6:
            if (memcmp (envline, "CHECK_", 6) == 0)
              s = &envline[7];
            break;
          case 8:
            if (! secure && memcmp (envline, "TOP_PAD_", 8) == 0)
              mALLOPt(M_TOP_PAD, atoi(&envline[9]));
            break;
          case 9:
            if (! secure && memcmp (envline, "MMAP_MAX_", 9) == 0)
              mALLOPt(M_MMAP_MAX, atoi(&envline[10]));
            break;
          case 15:
            if (! secure)
              {
                if (memcmp (envline, "TRIM_THRESHOLD_", 15) == 0)
                  mALLOPt(M_TRIM_THRESHOLD, atoi(&envline[16]));
                else if (memcmp (envline, "MMAP_THRESHOLD_", 15) == 0)
                  mALLOPt(M_MMAP_THRESHOLD, atoi(&envline[16]));
              }
            break;
          default:
            break;
          }
      }
  }
#else
  if (! secure)
    {
      if((s = getenv("MALLOC_TRIM_THRESHOLD_")))
        mALLOPt(M_TRIM_THRESHOLD, atoi(s));
      if((s = getenv("MALLOC_TOP_PAD_")))
        mALLOPt(M_TOP_PAD, atoi(s));
      if((s = getenv("MALLOC_MMAP_THRESHOLD_")))
        mALLOPt(M_MMAP_THRESHOLD, atoi(s));
      if((s = getenv("MALLOC_MMAP_MAX_")))
        mALLOPt(M_MMAP_MAX, atoi(s));
    }
  s = getenv("MALLOC_CHECK_");
#endif
  if(s) {
    if(s[0]) mALLOPt(M_CHECK_ACTION, (int)(s[0] - '0'));
    __malloc_check_init();
  }
  if(__malloc_initialize_hook != nullptr)
    (*__malloc_initialize_hook)();
  __malloc_initialized = 1;
}

/* There are platforms (e.g. Hurd) with a link-time hook mechanism. */
#ifdef thread_atfork_static
thread_atfork_static(ptmalloc_lock_all, ptmalloc_unlock_all, \
                     ptmalloc_unlock_all2)
#endif



/* Managing heaps and arenas (for concurrent threads) */

#if USE_ARENAS

#if MALLOC_DEBUG > 1

/* Print the complete contents of a single heap to stderr. */

static void
#if __STD_C
dump_heap(heap_info *heap)
#else
dump_heap(heap) heap_info *heap;
#endif
{
  char *ptr;
  mchunkptr p;

  fprintf(stderr, "Heap %p, size %10lx:\n", heap, (long)heap->size);
  ptr = (heap->ar_ptr != (mstate)(heap+1)) ?
    (char*)(heap + 1) : (char*)(heap + 1) + sizeof(struct malloc_state);
  p = (mchunkptr)(((CHUNK_SIZE_T)ptr + MALLOC_ALIGN_MASK) &
                  ~MALLOC_ALIGN_MASK);
  for(;;) {
    fprintf(stderr, "chunk %p size %10lx", p, (long)p->size);
    if(p == top(heap->ar_ptr)) {
      fprintf(stderr, " (top)\n");
      break;
    } else if(p->size == (0|PREV_INUSE)) {
      fprintf(stderr, " (fence)\n");
      break;
    }
    fprintf(stderr, "\n");
    p = next_chunk(p);
  }
}

#endif /* MALLOC_DEBUG > 1 */

/* Create a new heap.  size is automatically rounded up to a multiple
   of the page size. */

static heap_info *
internal_function
#if __STD_C
new_heap(size_t size, size_t top_pad)
#else
new_heap(size, top_pad) size_t size, top_pad;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  char *p1, *p2;
  unsigned long ul;
  heap_info *h;

  if(size+top_pad < HEAP_MIN_SIZE)
    size = HEAP_MIN_SIZE;
  else if(size+top_pad <= HEAP_MAX_SIZE)
    size += top_pad;
  else if(size > HEAP_MAX_SIZE)
    return 0;
  else
    size = HEAP_MAX_SIZE;
  size = (size + page_mask) & ~page_mask;

  /* A memory region aligned to a multiple of HEAP_MAX_SIZE is needed.
     No swap space needs to be reserved for the following large
     mapping (on Linux, this is the case for all non-writable mappings
     anyway). */
#ifndef WIN32
  /* Win32 emulation function has special case for HEAP_MAX_SIZE */
  p1 = (char *)MMAP(0, HEAP_MAX_SIZE<<1, PROT_NONE, MAP_PRIVATE|MAP_NORESERVE);
  if(p1 != MAP_FAILED) {
    p2 = (char *)(((unsigned long)p1 + (HEAP_MAX_SIZE-1)) & ~(HEAP_MAX_SIZE-1));
    ul = p2 - p1;
    munmap(p1, ul);
    munmap(p2 + HEAP_MAX_SIZE, HEAP_MAX_SIZE - ul);
  } else
#endif
  {
    /* Try to take the chance that an allocation of only HEAP_MAX_SIZE
       is already aligned. */
    p2 = (char *)MMAP(0, HEAP_MAX_SIZE, PROT_NONE, MAP_PRIVATE|MAP_NORESERVE);
    if(p2 == MAP_FAILED)
      return 0;
    if((unsigned long)p2 & (HEAP_MAX_SIZE-1)) {
      munmap(p2, HEAP_MAX_SIZE);
      return 0;
    }
  }
  if(mprotect(p2, size, PROT_READ|PROT_WRITE) != 0) {
    munmap(p2, HEAP_MAX_SIZE);
    return 0;
  }
  h = (heap_info *)p2;
  h->size = size;
  THREAD_STAT(stat_n_heaps++);
  memory_hook->inc_heap(size);
  return h;
}

/* Grow or shrink a heap.  size is automatically rounded up to a
   multiple of the page size if it is positive. */

static int
#if __STD_C
grow_heap(heap_info *h, long diff)
#else
grow_heap(h, diff) heap_info *h; long diff;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  long new_size;

  if(diff >= 0) {
    diff = (diff + page_mask) & ~page_mask;
    new_size = (long)h->size + diff;
    if(new_size > HEAP_MAX_SIZE)
      return -1;
    if(mprotect((char *)h + h->size, diff, PROT_READ|PROT_WRITE) != 0)
      return -2;
    memory_hook->inc_heap(diff);
  } else {
    new_size = (long)h->size + diff;
    if(new_size < (long)sizeof(*h))
      return -1;
    /* Try to re-map the extra heap space freshly to save memory, and
       make it inaccessible. */
    if((char *)MMAP((char *)h + new_size, -diff, PROT_NONE,
                    MAP_PRIVATE|MAP_FIXED) == (char *) MAP_FAILED)
      return -2;
    /*fprintf(stderr, "shrink %p %08lx\n", h, new_size);*/
    memory_hook->dec_heap(-diff);
  }
  h->size = new_size;
  return 0;
}

/* Delete a heap. */

#define delete_heap(heap) { \
  memory_hook->dec_heap(heap->size); \
  munmap((char*)(heap), HEAP_MAX_SIZE); \
}

static int
internal_function
#if __STD_C
heap_trim(heap_info *heap, size_t pad)
#else
heap_trim(heap, pad) heap_info *heap; size_t pad;
#endif
{
  mstate ar_ptr = heap->ar_ptr;
  unsigned long pagesz = mp_.pagesize;
  mchunkptr top_chunk = top(ar_ptr), p, bck, fwd;
  heap_info *prev_heap;
  long new_size, top_size, extra;

  /* Can this heap go away completely? */
  while(top_chunk == chunk_at_offset(heap, sizeof(*heap))) {
    prev_heap = heap->prev;
    p = chunk_at_offset(prev_heap, prev_heap->size - (MINSIZE-2*SIZE_SZ));
    assert(p->size == (0|PREV_INUSE)); /* must be fencepost */
    p = prev_chunk(p);
    new_size = chunksize(p) + (MINSIZE-2*SIZE_SZ);
    assert(new_size>0 && new_size<(long)(2*MINSIZE));
    if(!prev_inuse(p))
      new_size += p->prev_size;
    assert(new_size>0 && new_size<HEAP_MAX_SIZE);
    if(new_size + (HEAP_MAX_SIZE - prev_heap->size) < pad + MINSIZE + pagesz)
      break;
    ar_ptr->system_mem -= heap->size;
    arena_mem -= heap->size;
    delete_heap(heap);
    heap = prev_heap;
    if(!prev_inuse(p)) { /* consolidate backward */
      p = prev_chunk(p);
      unlink(p, bck, fwd);
    }
    assert((((CHUNK_SIZE_T)((char*)p + new_size)) & (pagesz-1)) == 0);
    assert( ((char*)p + new_size) == ((char*)heap + heap->size) );
    top(ar_ptr) = top_chunk = p;
    set_head(top_chunk, new_size | PREV_INUSE);
    /*check_chunk(ar_ptr, top_chunk);*/
  }
  top_size = chunksize(top_chunk);
  extra = ((top_size - pad - MINSIZE + (pagesz-1))/pagesz - 1) * pagesz;
  if(extra < (long)pagesz)
    return 0;
  /* Try to shrink. */
  if(grow_heap(heap, -extra) != 0)
    return 0;
  ar_ptr->system_mem -= extra;
  arena_mem -= extra;

  /* Success. Adjust top accordingly. */
  set_head(top_chunk, (top_size - extra) | PREV_INUSE);
  /*check_chunk(ar_ptr, top_chunk);*/
  return 1;
}

static mstate
internal_function
#if __STD_C
arena_get2(mstate a_tsd, size_t size)
#else
arena_get2(a_tsd, size) mstate a_tsd; size_t size;
#endif
{
  mstate a;
  int err;

  if(!a_tsd)
    a = a_tsd = &main_arena;
  else {
    a = a_tsd->next;
    if(!a) {
      /* This can only happen while initializing the new arena. */
      (void)mutex_lock(&main_arena.mutex);
      THREAD_STAT(++(main_arena.stat_lock_wait));
      return &main_arena;
    }
  }

  /* Check the global, circularly linked list for available arenas. */
 repeat:
  do {
    if(!mutex_trylock(&a->mutex)) {
      THREAD_STAT(++(a->stat_lock_loop));
      tsd_setspecific(arena_key, (Void_t *)a);
      return a;
    }
    a = a->next;
  } while(a != a_tsd);

  /* If not even the list_lock can be obtained, try again.  This can
     happen during `atfork', or for example on systems where thread
     creation makes it temporarily impossible to obtain _any_
     locks. */
  if(mutex_trylock(&list_lock)) {
    a = a_tsd;
    goto repeat;
  }
  (void)mutex_unlock(&list_lock);

  /* Nothing immediately available, so generate a new arena.  */
  a = _int_new_arena(size);
  if(!a)
    return 0;

  tsd_setspecific(arena_key, (Void_t *)a);
  mutex_init(&a->mutex);
  err = mutex_lock(&a->mutex); /* remember result */

  /* Add the new arena to the global list.  */
  (void)mutex_lock(&list_lock);
  a->next = main_arena.next;
  main_arena.next = a;
  (void)mutex_unlock(&list_lock);

  if(err) /* locking failed; keep arena for further attempts later */
    return 0;

  THREAD_STAT(++(a->stat_lock_loop));
  return a;
}

/* Create a new arena with initial size "size".  */

mstate
_int_new_arena __MALLOC_P((size_t size))
{
  mstate a;
  heap_info *h;
  char *ptr;
  unsigned long misalign;

  h = new_heap(size + (sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT),
               mp_.top_pad);
  if(!h) {
    /* Maybe size is too large to fit in a single heap.  So, just try
       to create a minimally-sized arena and let _int_malloc() attempt
       to deal with the large request via mmap_chunk().  */
    h = new_heap(sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT, mp_.top_pad);
    if(!h)
      return 0;
  }
  a = h->ar_ptr = (mstate)(h+1);
  malloc_init_state(a);
  /*a->next = NULL;*/
  a->system_mem = a->max_system_mem = h->size;
  arena_mem += h->size;
#ifdef NO_THREADS
  if((unsigned long)(mp_.mmapped_mem + arena_mem + main_arena.system_mem) >
     mp_.max_total_mem)
    mp_.max_total_mem = mp_.mmapped_mem + arena_mem + main_arena.system_mem;
#endif

  /* Set up the top chunk, with proper alignment. */
  ptr = (char *)(a + 1);
  misalign = (unsigned long)chunk2mem(ptr) & MALLOC_ALIGN_MASK;
  if (misalign > 0)
    ptr += MALLOC_ALIGNMENT - misalign;
  top(a) = (mchunkptr)ptr;
  set_head(top(a), (((char*)h + h->size) - ptr) | PREV_INUSE);

  return a;
}

#endif /* USE_ARENAS */

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */


/*************************** ptmalloc2.c ******************************/

/*
  Debugging support

  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#if ! MALLOC_DEBUG

#define check_chunk(A,P)
#define check_free_chunk(A,P)
#define check_inuse_chunk(A,P)
#define check_remalloced_chunk(A,P,N)
#define check_malloced_chunk(A,P,N)
#define check_malloc_state(A)

#else

#define check_chunk(A,P)              do_check_chunk(A,P)
#define check_free_chunk(A,P)         do_check_free_chunk(A,P)
#define check_inuse_chunk(A,P)        do_check_inuse_chunk(A,P)
#define check_remalloced_chunk(A,P,N) do_check_remalloced_chunk(A,P,N)
#define check_malloced_chunk(A,P,N)   do_check_malloced_chunk(A,P,N)
#define check_malloc_state(A)         do_check_malloc_state(A)

/*
  Properties of all chunks
*/

#if __STD_C
static void do_check_chunk(mstate av, mchunkptr p)
#else
static void do_check_chunk(av, p) mstate av; mchunkptr p;
#endif
{
  CHUNK_SIZE_T  sz = chunksize(p);
  /* min and max possible addresses assuming contiguous allocation */
  char* max_address = (char*)(av->top) + chunksize(av->top);
  char* min_address = max_address - av->system_mem;

  if (!chunk_is_mmapped(p)) {
    
    /* Has legal address ... */
    if (p != av->top) {
      if (contiguous(av)) {
        assert(((char*)p) >= min_address);
        assert(((char*)p + sz) <= ((char*)(av->top)));
      }
    }
    else {
      /* top size is always at least MINSIZE */
      assert((CHUNK_SIZE_T)(sz) >= MINSIZE);
      /* top predecessor always marked inuse */
      assert(prev_inuse(p));
    }
      
  }
  else {
#if HAVE_MMAP
    /* address is outside main heap  */
    if (contiguous(av) && av->top != initial_top(av)) {
      assert(((char*)p) < min_address || ((char*)p) > max_address);
    }
    /* chunk is page-aligned */
    assert(((p->prev_size + sz) & (mp_.pagesize-1)) == 0);
    /* mem is aligned */
    assert(aligned_OK(chunk2mem(p)));
#else
    /* force an appropriate assert violation if debug set */
    assert(!chunk_is_mmapped(p));
#endif
  }
}

/*
  Properties of free chunks
*/

#if __STD_C
static void do_check_free_chunk(mstate av, mchunkptr p)
#else
static void do_check_free_chunk(av, p) mstate av; mchunkptr p;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~(PREV_INUSE|NON_MAIN_ARENA);
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(av, p);

  /* Chunk must claim to be free ... */
  assert(!inuse(p));
  assert (!chunk_is_mmapped(p));

  /* Unless a special marker, must have OK fields */
  if ((CHUNK_SIZE_T)(sz) >= MINSIZE)
  {
    assert((sz & MALLOC_ALIGN_MASK) == 0);
    assert(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    assert(next->prev_size == sz);
    /* ... and is fully consolidated */
    assert(prev_inuse(p));
    assert (next == av->top || inuse(next));

    /* ... and has minimally sane links */
    assert(p->fd->bk == p);
    assert(p->bk->fd == p);
  }
  else /* markers are always of size SIZE_SZ */
    assert(sz == SIZE_SZ);
}

/*
  Properties of inuse chunks
*/

#if __STD_C
static void do_check_inuse_chunk(mstate av, mchunkptr p)
#else
static void do_check_inuse_chunk(av, p) mstate av; mchunkptr p;
#endif
{
  mchunkptr next;

  do_check_chunk(av, p);

  if (chunk_is_mmapped(p))
    return; /* mmapped chunks have no next/prev */

  /* Check whether it claims to be in use ... */
  assert(inuse(p));

  next = next_chunk(p);

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  if (!prev_inuse(p))  {
    /* Note that we cannot even look at prev unless it is not inuse */
    mchunkptr prv = prev_chunk(p);
    assert(next_chunk(prv) == p);
    do_check_free_chunk(av, prv);
  }

  if (next == av->top) {
    assert(prev_inuse(next));
    assert(chunksize(next) >= MINSIZE);
  }
  else if (!inuse(next))
    do_check_free_chunk(av, next);
}

/*
  Properties of chunks recycled from fastbins
*/

#if __STD_C
static void do_check_remalloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s)
#else
static void do_check_remalloced_chunk(av, p, s)
mstate av; mchunkptr p; INTERNAL_SIZE_T s;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~(PREV_INUSE|NON_MAIN_ARENA);

  if (!chunk_is_mmapped(p)) {
    assert(av == arena_for_chunk(p));
    if (chunk_non_main_arena(p))
      assert(av != &main_arena);
    else
      assert(av == &main_arena);
  }

  do_check_inuse_chunk(av, p);

  /* Legal size ... */
  assert((sz & MALLOC_ALIGN_MASK) == 0);
  assert((CHUNK_SIZE_T)(sz) >= MINSIZE);
  /* ... and alignment */
  assert(aligned_OK(chunk2mem(p)));
  /* chunk is less than MINSIZE more than request */
  assert((long)(sz) - (long)(s) >= 0);
  assert((long)(sz) - (long)(s + MINSIZE) < 0);
}

/*
  Properties of nonrecycled chunks at the point they are malloced
*/

#if __STD_C
static void do_check_malloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s)
#else
static void do_check_malloced_chunk(av, p, s)
mstate av; mchunkptr p; INTERNAL_SIZE_T s;
#endif
{
  /* same as recycled case ... */
  do_check_remalloced_chunk(av, p, s);

  /*
    ... plus,  must obey implementation invariant that prev_inuse is
    always true of any allocated chunk; i.e., that each allocated
    chunk borders either a previously allocated and still in-use
    chunk, or the base of its memory arena. This is ensured
    by making all allocations from the the `lowest' part of any found
    chunk.  This does not necessarily hold however for chunks
    recycled via fastbins.
  */

  assert(prev_inuse(p));
}


/*
  Properties of malloc_state.

  This may be useful for debugging malloc, as well as detecting user
  programmer errors that somehow write into malloc_state.

  If you are extending or experimenting with this malloc, you can
  probably figure out how to hack this routine to print out or
  display chunk addresses, sizes, bins, and other instrumentation.
*/

static void do_check_malloc_state(mstate av)
{
  int i;
  mchunkptr p;
  mchunkptr q;
  mbinptr b;
  unsigned int binbit;
  int empty;
  unsigned int idx;
  INTERNAL_SIZE_T size;
  CHUNK_SIZE_T  total = 0;
  int max_fast_bin;

  /* internal size_t must be no wider than pointer type */
  assert(sizeof(INTERNAL_SIZE_T) <= sizeof(char*));

  /* alignment is a power of 2 */
  assert((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-1)) == 0);

  /* cannot run remaining checks until fully initialized */
  if (av->top == 0 || av->top == initial_top(av))
    return;

  /* pagesize is a power of 2 */
  assert((mp_.pagesize & (mp_.pagesize-1)) == 0);

  /* A contiguous main_arena is consistent with sbrk_base.  */
  if (av == &main_arena && contiguous(av))
    assert((char*)mp_.sbrk_base + av->system_mem ==
           (char*)av->top + chunksize(av->top));

  /* properties of fastbins */

  /* max_fast is in allowed range */
  assert((av->max_fast & ~1) <= request2size(MAX_FAST_SIZE));

  max_fast_bin = fastbin_index(av->max_fast);

  for (i = 0; i < NFASTBINS; ++i) {
    p = av->fastbins[i];

    /* all bins past max_fast are empty */
    if (i > max_fast_bin)
      assert(p == 0);

    while (p != 0) {
      /* each chunk claims to be inuse */
      do_check_inuse_chunk(av, p);
      total += chunksize(p);
      /* chunk belongs in this bin */
      assert(fastbin_index(chunksize(p)) == i);
      p = p->fd;
    }
  }

  if (total != 0)
    assert(have_fastchunks(av));
  else if (!have_fastchunks(av))
    assert(total == 0);

  /* check normal bins */
  for (i = 1; i < NBINS; ++i) {
    b = bin_at(av,i);

    /* binmap is accurate (except for bin 1 == unsorted_chunks) */
    if (i >= 2) {
      binbit = get_binmap(av,i);
      empty = last(b) == b;
      if (!binbit)
        assert(empty);
      else if (!empty)
        assert(binbit);
    }

    for (p = last(b); p != b; p = p->bk) {
      /* each chunk claims to be free */
      do_check_free_chunk(av, p);
      size = chunksize(p);
      total += size;
      if (i >= 2) {
        /* chunk belongs in bin */
        idx = bin_index(size);
        assert(idx == i);
        /* lists are sorted */
        if ((CHUNK_SIZE_T) size >= (CHUNK_SIZE_T)(FIRST_SORTED_BIN_SIZE)) {
          assert(p->bk == b || 
                 (CHUNK_SIZE_T)chunksize(p->bk) >= 
                 (CHUNK_SIZE_T)chunksize(p));
        }
      }
      /* chunk is followed by a legal chain of inuse chunks */
      for (q = next_chunk(p);
           (q != av->top && inuse(q) && 
             (CHUNK_SIZE_T)(chunksize(q)) >= MINSIZE);
           q = next_chunk(q))
        do_check_inuse_chunk(av, q);
    }
  }

  /* top chunk is OK */
  check_chunk(av, av->top);

  /* sanity checks for statistics */

#ifdef NO_THREADS
  assert(total <= (CHUNK_SIZE_T)(mp_.max_total_mem));
  assert(mp_.n_mmaps >= 0);
#endif
  assert(mp_.n_mmaps <= mp_.n_mmaps_max);
  assert(mp_.n_mmaps <= mp_.max_n_mmaps);

  assert((CHUNK_SIZE_T)(av->system_mem) <=
         (CHUNK_SIZE_T)(av->max_system_mem));

  assert((CHUNK_SIZE_T)(mp_.mmapped_mem) <=
         (CHUNK_SIZE_T)(mp_.max_mmapped_mem));

#ifdef NO_THREADS
  assert((CHUNK_SIZE_T)(mp_.max_total_mem) >=
         (CHUNK_SIZE_T)(mp_.mmapped_mem) + (CHUNK_SIZE_T)(av->system_mem));
#endif
}
#endif


/* ----------------- Support for debugging hooks -------------------- */

/*************************** ptmalloc2.c ******************************/

/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 2001,02 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* $Id$ */

#ifndef weak_variable
#define weak_variable /**/
#endif

#ifndef DEFAULT_CHECK_ACTION
#define DEFAULT_CHECK_ACTION 1
#endif

/* What to do if the standard debugging hooks are in place and a
   corrupt pointer is detected: do nothing (0), print an error message
   (1), or call abort() (2). */

/* Hooks for debugging versions.  The initial hooks just call the
   initialization routine, then do the normal work. */

static Void_t*
#if __STD_C
malloc_hook_ini(size_t sz, const __malloc_ptr_t caller)
#else
malloc_hook_ini(sz, caller)
     size_t sz; const __malloc_ptr_t caller;
#endif
{
  __malloc_hook = nullptr;
  ptmalloc_init();
  return public_mALLOc(sz);
}

static Void_t*
#if __STD_C
realloc_hook_ini(Void_t* ptr, size_t sz, const __malloc_ptr_t caller)
#else
realloc_hook_ini(ptr, sz, caller)
     Void_t* ptr; size_t sz; const __malloc_ptr_t caller;
#endif
{
  __malloc_hook = nullptr;
  __realloc_hook = nullptr;
  ptmalloc_init();
  return public_rEALLOc(ptr, sz);
}

static Void_t*
#if __STD_C
memalign_hook_ini(size_t alignment, size_t sz, const __malloc_ptr_t caller)
#else
memalign_hook_ini(alignment, sz, caller)
     size_t alignment; size_t sz; const __malloc_ptr_t caller;
#endif
{
  __memalign_hook = nullptr;
  ptmalloc_init();
  return public_mEMALIGn(alignment, sz);
}

void weak_variable (*__malloc_initialize_hook) __MALLOC_P ((void)) = nullptr;
void weak_variable (*__free_hook) __MALLOC_P ((__malloc_ptr_t __ptr,
                                               const __malloc_ptr_t)) = nullptr;
__malloc_ptr_t weak_variable (*__malloc_hook)
 __MALLOC_P ((size_t __size, const __malloc_ptr_t)) = malloc_hook_ini;
__malloc_ptr_t weak_variable (*__realloc_hook)
 __MALLOC_P ((__malloc_ptr_t __ptr, size_t __size, const __malloc_ptr_t))
     = realloc_hook_ini;
__malloc_ptr_t weak_variable (*__memalign_hook)
 __MALLOC_P ((size_t __alignment, size_t __size, const __malloc_ptr_t))
     = memalign_hook_ini;
void weak_variable (*__after_morecore_hook) __MALLOC_P ((void)) = nullptr;


static int check_action = DEFAULT_CHECK_ACTION;

/* Whether we are using malloc checking.  */
static int using_malloc_checking;

/* A flag that is set by malloc_set_state, to signal that malloc checking
   must not be enabled on the request from the user (via the MALLOC_CHECK_
   environment variable).  It is reset by __malloc_check_init to tell
   malloc_set_state that the user has requested malloc checking.

   The purpose of this flag is to make sure that malloc checking is not
   enabled when the heap to be restored was constructed without malloc
   checking, and thus does not contain the required magic bytes.
   Otherwise the heap would be corrupted by calls to free and realloc.  If
   it turns out that the heap was created with malloc checking and the
   user has requested it malloc_set_state just calls __malloc_check_init
   again to enable it.  On the other hand, reusing such a heap without
   further malloc checking is safe.  */
static int disallow_malloc_check;

/* Activate a standard set of debugging hooks. */
void
__malloc_check_init __MALLOC_P(())
{
  if (disallow_malloc_check) {
    disallow_malloc_check = 0;
    return;
  }
  using_malloc_checking = 1;
  __malloc_hook = malloc_check;
  __free_hook = free_check;
  __realloc_hook = realloc_check;
  __memalign_hook = memalign_check;
  if(check_action & 1)
    fprintf(stderr, "malloc: using debugging hooks\n");
}

/* A simple, standard set of debugging hooks.  Overhead is `only' one
   byte per chunk; still this will catch most cases of double frees or
   overruns.  The goal here is to avoid obscure crashes due to invalid
   usage, unlike in the MALLOC_DEBUG code. */

#define MAGICBYTE(p) ( ( ((size_t)p >> 3) ^ ((size_t)p >> 11)) & 0xFF )

/* Instrument a chunk with overrun detector byte(s) and convert it
   into a user pointer with requested size sz. */

static Void_t*
internal_function
#if __STD_C
mem2mem_check(Void_t *ptr, size_t sz)
#else
mem2mem_check(ptr, sz) Void_t *ptr; size_t sz;
#endif
{
  mchunkptr p;
  unsigned char* m_ptr = (unsigned char*)BOUNDED_N(ptr, sz);
  size_t i;

  if (!ptr)
    return ptr;
  p = mem2chunk(ptr);
  for(i = chunksize(p) - (chunk_is_mmapped(p) ? 2*SIZE_SZ+1 : SIZE_SZ+1);
      i > sz;
      i -= 0xFF) {
    if(i-sz < 0x100) {
      m_ptr[i] = (unsigned char)(i-sz);
      break;
    }
    m_ptr[i] = 0xFF;
  }
  m_ptr[sz] = MAGICBYTE(p);
  return (Void_t*)m_ptr;
}

/* Convert a pointer to be free()d or realloc()ed to a valid chunk
   pointer.  If the provided pointer is not valid, return NULL. */

static mchunkptr
internal_function
#if __STD_C
mem2chunk_check(Void_t* mem)
#else
mem2chunk_check(mem) Void_t* mem;
#endif
{
  mchunkptr p;
  INTERNAL_SIZE_T sz, c;
  unsigned char magic;

  p = mem2chunk(mem);
  if(!aligned_OK(p)) return nullptr;
  if( (char*)p>=mp_.sbrk_base &&
      (char*)p<(mp_.sbrk_base+main_arena.system_mem) ) {
    /* Must be a chunk in conventional heap memory. */
    if(chunk_is_mmapped(p) ||
       ( (sz = chunksize(p)),
         ((char*)p + sz)>=(mp_.sbrk_base+main_arena.system_mem) ) ||
       sz<MINSIZE || sz&MALLOC_ALIGN_MASK || !inuse(p) ||
       ( !prev_inuse(p) && (p->prev_size&MALLOC_ALIGN_MASK ||
                            (long)prev_chunk(p)<(long)mp_.sbrk_base ||
                            next_chunk(prev_chunk(p))!=p) ))
      return nullptr;
    magic = MAGICBYTE(p);
    for(sz += SIZE_SZ-1; (c = ((unsigned char*)p)[sz]) != magic; sz -= c) {
      if(c<=0 || sz<(c+2*SIZE_SZ)) return nullptr;
    }
    ((unsigned char*)p)[sz] ^= 0xFF;
  } else {
    unsigned long offset, page_mask = malloc_getpagesize-1;

    /* mmap()ed chunks have MALLOC_ALIGNMENT or higher power-of-two
       alignment relative to the beginning of a page.  Check this
       first. */
    offset = (unsigned long)mem & page_mask;
    if((offset!=MALLOC_ALIGNMENT && offset!=0 && offset!=0x10 &&
        offset!=0x20 && offset!=0x40 && offset!=0x80 && offset!=0x100 &&
        offset!=0x200 && offset!=0x400 && offset!=0x800 && offset!=0x1000 &&
        offset<0x2000) ||
       !chunk_is_mmapped(p) || (p->size & PREV_INUSE) ||
       ( (((unsigned long)p - p->prev_size) & page_mask) != 0 ) ||
       ( (sz = chunksize(p)), ((p->prev_size + sz) & page_mask) != 0 ) )
      return nullptr;
    magic = MAGICBYTE(p);
    for(sz -= 1; (c = ((unsigned char*)p)[sz]) != magic; sz -= c) {
      if(c<=0 || sz<(c+2*SIZE_SZ)) return nullptr;
    }
    ((unsigned char*)p)[sz] ^= 0xFF;
  }
  return p;
}

/* Check for corruption of the top chunk, and try to recover if
   necessary. */

static int
internal_function
#if __STD_C
top_check(void)
#else
top_check()
#endif
{
  mchunkptr t = top(&main_arena);
  char* brk, * new_brk;
  INTERNAL_SIZE_T front_misalign, sbrk_size;
  unsigned long pagesz = malloc_getpagesize;

  if((char*)t + chunksize(t) == mp_.sbrk_base + main_arena.system_mem ||
     t == initial_top(&main_arena)) return 0;

  if(check_action & 1)
    fprintf(stderr, "malloc: top chunk is corrupt\n");
  if(check_action & 2)
    abort();

  /* Try to set up a new top chunk. */
  brk = (char *) MORECORE(0);
  front_misalign = (unsigned long)chunk2mem(brk) & MALLOC_ALIGN_MASK;
  if (front_misalign > 0)
    front_misalign = MALLOC_ALIGNMENT - front_misalign;
  sbrk_size = front_misalign + mp_.top_pad + MINSIZE;
  sbrk_size += pagesz - ((unsigned long)(brk + sbrk_size) & (pagesz - 1));
  new_brk = (char*)(MORECORE (sbrk_size));
  if (new_brk == (char*)(MORECORE_FAILURE)) return -1;
  /* Call the `morecore' hook if necessary.  */
  if (__after_morecore_hook)
    (*__after_morecore_hook) ();
  memory_hook->inc_heap(sbrk_size);
  main_arena.system_mem = (new_brk - mp_.sbrk_base) + sbrk_size;

  top(&main_arena) = (mchunkptr)(brk + front_misalign);
  set_head(top(&main_arena), (sbrk_size - front_misalign) | PREV_INUSE);

  return 0;
}

static Void_t*
#if __STD_C
malloc_check(size_t sz, const Void_t *caller)
#else
malloc_check(sz, caller) size_t sz; const Void_t *caller;
#endif
{
  Void_t *victim;

  (void)mutex_lock(&main_arena.mutex);
  victim = (top_check() >= 0) ? _int_malloc(&main_arena, sz+1) : nullptr;
  (void)mutex_unlock(&main_arena.mutex);
  return mem2mem_check(victim, sz);
}

static void
#if __STD_C
free_check(Void_t* mem, const Void_t *caller)
#else
free_check(mem, caller) Void_t* mem; const Void_t *caller;
#endif
{
  mchunkptr p;

  if(!mem) return;
  (void)mutex_lock(&main_arena.mutex);
  p = mem2chunk_check(mem);
  if(!p) {
    (void)mutex_unlock(&main_arena.mutex);
    if(check_action & 1)
      fprintf(stderr, "free(): invalid pointer %p!\n", mem);
    if(check_action & 2)
      abort();
    return;
  }
#if HAVE_MMAP
  if (chunk_is_mmapped(p)) {
    (void)mutex_unlock(&main_arena.mutex);
    munmap_chunk(p);
    return;
  }
#endif
#if 0 /* Erase freed memory. */
  memset(mem, 0, chunksize(p) - (SIZE_SZ+1));
#endif
  _int_free(&main_arena, mem);
  (void)mutex_unlock(&main_arena.mutex);
}

static Void_t*
#if __STD_C
realloc_check(Void_t* oldmem, size_t bytes, const Void_t *caller)
#else
realloc_check(oldmem, bytes, caller)
     Void_t* oldmem; size_t bytes; const Void_t *caller;
#endif
{
  mchunkptr oldp;
  INTERNAL_SIZE_T nb, oldsize;
  Void_t* newmem = 0;

  if (oldmem == 0) return malloc_check(bytes, nullptr);
  (void)mutex_lock(&main_arena.mutex);
  oldp = mem2chunk_check(oldmem);
  (void)mutex_unlock(&main_arena.mutex);
  if(!oldp) {
    if(check_action & 1)
      fprintf(stderr, "realloc(): invalid pointer %p!\n", oldmem);
    if(check_action & 2)
      abort();
    return malloc_check(bytes, nullptr);
  }
  oldsize = chunksize(oldp);

  checked_request2size(bytes+1, nb);
  (void)mutex_lock(&main_arena.mutex);

#if HAVE_MMAP
  if (chunk_is_mmapped(oldp)) {
#if HAVE_MREMAP
    mchunkptr newp = mremap_chunk(oldp, nb);
    if(newp)
      newmem = chunk2mem(newp);
    else
#endif
    {
      /* Note the extra SIZE_SZ overhead. */
      if(oldsize - SIZE_SZ >= nb)
        newmem = oldmem; /* do nothing */
      else {
        /* Must alloc, copy, free. */
        if (top_check() >= 0)
          newmem = _int_malloc(&main_arena, bytes+1);
        if (newmem) {
          MALLOC_COPY(BOUNDED_N(newmem, bytes+1), oldmem, oldsize - 2*SIZE_SZ);
          munmap_chunk(oldp);
        }
      }
    }
  } else {
#endif /* HAVE_MMAP */
    if (top_check() >= 0)
      newmem = _int_realloc(&main_arena, oldmem, bytes+1);
#if 0 /* Erase freed memory. */
    if(newmem)
      newp = mem2chunk(newmem);
    nb = chunksize(newp);
    if(oldp<newp || oldp>=chunk_at_offset(newp, nb)) {
      memset((char*)oldmem + 2*sizeof(mbinptr), 0,
             oldsize - (2*sizeof(mbinptr)+2*SIZE_SZ+1));
    } else if(nb > oldsize+SIZE_SZ) {
      memset((char*)BOUNDED_N(chunk2mem(newp), bytes) + oldsize,
             0, nb - (oldsize+SIZE_SZ));
    }
#endif
#if HAVE_MMAP
  }
#endif
  (void)mutex_unlock(&main_arena.mutex);

  return mem2mem_check(newmem, bytes);
}

static Void_t*
#if __STD_C
memalign_check(size_t alignment, size_t bytes, const Void_t *caller)
#else
memalign_check(alignment, bytes, caller)
     size_t alignment; size_t bytes; const Void_t *caller;
#endif
{
  INTERNAL_SIZE_T nb;
  Void_t* mem;

  if (alignment <= MALLOC_ALIGNMENT) return malloc_check(bytes, nullptr);
  if (alignment <  MINSIZE) alignment = MINSIZE;

  checked_request2size(bytes+1, nb);
  (void)mutex_lock(&main_arena.mutex);
  mem = (top_check() >= 0) ? _int_memalign(&main_arena, alignment, bytes+1) :
    nullptr;
  (void)mutex_unlock(&main_arena.mutex);
  return mem2mem_check(mem, bytes);
}

#ifndef NO_THREADS

/* The following hooks are used when the global initialization in
   ptmalloc_init() hasn't completed yet. */

static Void_t*
#if __STD_C
malloc_starter(size_t sz, const Void_t *caller)
#else
malloc_starter(sz, caller) size_t sz; const Void_t *caller;
#endif
{
  Void_t* victim;

  victim = _int_malloc(&main_arena, sz);

  return victim ? BOUNDED_N(victim, sz) : 0;
}

static void
#if __STD_C
free_starter(Void_t* mem, const Void_t *caller)
#else
free_starter(mem, caller) Void_t* mem; const Void_t *caller;
#endif
{
  mchunkptr p;

  if(!mem) return;
  p = mem2chunk(mem);
#if HAVE_MMAP
  if (chunk_is_mmapped(p)) {
    munmap_chunk(p);
    return;
  }
#endif
  _int_free(&main_arena, mem);
}

#endif /* NO_THREADS */


/* Get/set state: malloc_get_state() records the current state of all
   malloc variables (_except_ for the actual heap contents and `hook'
   function pointers) in a system dependent, opaque data structure.
   This data structure is dynamically allocated and can be free()d
   after use.  malloc_set_state() restores the state of all malloc
   variables to the previously obtained state.  This is especially
   useful when using this malloc as part of a shared library, and when
   the heap contents are saved/restored via some other method.  The
   primary example for this is GNU Emacs with its `dumping' procedure.
   `Hook' function pointers are never saved or restored by these
   functions, with two exceptions: If malloc checking was in use when
   malloc_get_state() was called, then malloc_set_state() calls
   __malloc_check_init() if possible; if malloc checking was not in
   use in the recorded state but the user requested malloc checking,
   then the hooks are reset to 0.  */

#define MALLOC_STATE_MAGIC   0x444c4541l
#define MALLOC_STATE_VERSION (0*0x100l + 2l) /* major*0x100 + minor */

struct malloc_save_state {
  long          magic;
  long          version;
  mbinptr       av[NBINS * 2 + 2];
  char*         sbrk_base;
  int           sbrked_mem_bytes;
  unsigned long trim_threshold;
  unsigned long top_pad;
  unsigned int  n_mmaps_max;
  unsigned long mmap_threshold;
  int           check_action;
  unsigned long max_sbrked_mem;
  unsigned long max_total_mem;
  unsigned int  n_mmaps;
  unsigned int  max_n_mmaps;
  unsigned long mmapped_mem;
  unsigned long max_mmapped_mem;
  int           using_malloc_checking;
};

Void_t*
public_gET_STATe(void)
{
  struct malloc_save_state* ms;
  int i;
  mbinptr b;

  ms = (struct malloc_save_state*)public_mALLOc(sizeof(*ms));
  if (!ms)
    return 0;
  (void)mutex_lock(&main_arena.mutex);
  malloc_consolidate(&main_arena);
  ms->magic = MALLOC_STATE_MAGIC;
  ms->version = MALLOC_STATE_VERSION;
  ms->av[0] = 0;
  ms->av[1] = 0; /* used to be binblocks, now no longer used */
  ms->av[2] = top(&main_arena);
  ms->av[3] = 0; /* used to be undefined */
  for(i=1; i<NBINS; i++) {
    b = bin_at(&main_arena, i);
    if(first(b) == b)
      ms->av[2*i+2] = ms->av[2*i+3] = 0; /* empty bin */
    else {
      ms->av[2*i+2] = first(b);
      ms->av[2*i+3] = last(b);
    }
  }
  ms->sbrk_base = mp_.sbrk_base;
  ms->sbrked_mem_bytes = main_arena.system_mem;
  ms->trim_threshold = mp_.trim_threshold;
  ms->top_pad = mp_.top_pad;
  ms->n_mmaps_max = mp_.n_mmaps_max;
  ms->mmap_threshold = mp_.mmap_threshold;
  ms->check_action = check_action;
  ms->max_sbrked_mem = main_arena.max_system_mem;
#ifdef NO_THREADS
  ms->max_total_mem = mp_.max_total_mem;
#else
  ms->max_total_mem = 0;
#endif
  ms->n_mmaps = mp_.n_mmaps;
  ms->max_n_mmaps = mp_.max_n_mmaps;
  ms->mmapped_mem = mp_.mmapped_mem;
  ms->max_mmapped_mem = mp_.max_mmapped_mem;
  ms->using_malloc_checking = using_malloc_checking;
  (void)mutex_unlock(&main_arena.mutex);
  return (Void_t*)ms;
}

int
public_sET_STATe(Void_t* msptr)
{
  struct malloc_save_state* ms = (struct malloc_save_state*)msptr;
  int i;
  mbinptr b;

  disallow_malloc_check = 1;
  ptmalloc_init();
  if(ms->magic != MALLOC_STATE_MAGIC) return -1;
  /* Must fail if the major version is too high. */
  if((ms->version & ~0xffl) > (MALLOC_STATE_VERSION & ~0xffl)) return -2;
  (void)mutex_lock(&main_arena.mutex);
  /* There are no fastchunks.  */
  clear_fastchunks(&main_arena);
  set_max_fast(&main_arena, DEFAULT_MXFAST);
  for (i=0; i<NFASTBINS; ++i)
    main_arena.fastbins[i] = 0;
  for (i=0; i<BINMAPSIZE; ++i)
    main_arena.binmap[i] = 0;
  top(&main_arena) = ms->av[2];
  main_arena.last_remainder = 0;
  for(i=1; i<NBINS; i++) {
    b = bin_at(&main_arena, i);
    if(ms->av[2*i+2] == 0) {
      assert(ms->av[2*i+3] == 0);
      first(b) = last(b) = b;
    } else {
      if(i<NSMALLBINS || (largebin_index(chunksize(ms->av[2*i+2]))==i &&
                          largebin_index(chunksize(ms->av[2*i+3]))==i)) {
        first(b) = ms->av[2*i+2];
        last(b) = ms->av[2*i+3];
        /* Make sure the links to the bins within the heap are correct.  */
        first(b)->bk = b;
        last(b)->fd = b;
        /* Set bit in binblocks.  */
        mark_bin(&main_arena, i);
      } else {
        /* Oops, index computation from chunksize must have changed.
           Link the whole list into unsorted_chunks.  */
        first(b) = last(b) = b;
        b = unsorted_chunks(&main_arena);
        ms->av[2*i+2]->bk = b;
        ms->av[2*i+3]->fd = b->fd;
        b->fd->bk = ms->av[2*i+3];
        b->fd = ms->av[2*i+2];
      }
    }
  }
  mp_.sbrk_base = ms->sbrk_base;
  main_arena.system_mem = ms->sbrked_mem_bytes;
  mp_.trim_threshold = ms->trim_threshold;
  mp_.top_pad = ms->top_pad;
  mp_.n_mmaps_max = ms->n_mmaps_max;
  mp_.mmap_threshold = ms->mmap_threshold;
  check_action = ms->check_action;
  main_arena.max_system_mem = ms->max_sbrked_mem;
#ifdef NO_THREADS
  mp_.max_total_mem = ms->max_total_mem;
#endif
  mp_.n_mmaps = ms->n_mmaps;
  mp_.max_n_mmaps = ms->max_n_mmaps;
  mp_.mmapped_mem = ms->mmapped_mem;
  mp_.max_mmapped_mem = ms->max_mmapped_mem;
  /* add version-dependent code here */
  if (ms->version >= 1) {
    /* Check whether it is safe to enable malloc checking, or whether
       it is necessary to disable it.  */
    if (ms->using_malloc_checking && !using_malloc_checking &&
        !disallow_malloc_check)
      __malloc_check_init ();
    else if (!ms->using_malloc_checking && using_malloc_checking) {
      __malloc_hook = 0;
      __free_hook = 0;
      __realloc_hook = 0;
      __memalign_hook = 0;
      using_malloc_checking = 0;
    }
  }
  check_malloc_state(&main_arena);

  (void)mutex_unlock(&main_arena.mutex);
  return 0;
}

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */

/*************************** ptmalloc2.c ******************************/


/* ----------- Routines dealing with system allocation -------------- */

/*
  sysmalloc handles malloc cases requiring more memory from the system.
  On entry, it is assumed that av->top does not have enough
  space to service request for nb bytes, thus requiring that av->top
  be extended or replaced.
*/

#if __STD_C
static Void_t* sYSMALLOc(INTERNAL_SIZE_T nb, mstate av)
#else
static Void_t* sYSMALLOc(nb, av) INTERNAL_SIZE_T nb; mstate av;
#endif
{
  mchunkptr       old_top;        /* incoming value of av->top */
  INTERNAL_SIZE_T old_size;       /* its size */
  char*           old_end;        /* its end address */

  long            size;           /* arg to first MORECORE or mmap call */
  char*           brk;            /* return value from MORECORE */

  long            correction;     /* arg to 2nd MORECORE call */
  char*           snd_brk;        /* 2nd return val */

  INTERNAL_SIZE_T front_misalign; /* unusable bytes at front of new space */
  INTERNAL_SIZE_T end_misalign;   /* partial page left at end of new space */
  char*           aligned_brk;    /* aligned offset into brk */

  mchunkptr       p;              /* the allocated/returned chunk */
  mchunkptr       remainder;      /* remainder from allocation */
  CHUNK_SIZE_T    remainder_size; /* its size */

  CHUNK_SIZE_T    sum;            /* for updating stats */

  size_t          pagemask  = mp_.pagesize - 1;

  /*
    If there is space available in fastbins, consolidate and retry
    malloc from scratch rather than getting memory from system.  This
    can occur only if nb is in smallbin range so we didn't consolidate
    upon entry to malloc. It is much easier to handle this case here
    than in malloc proper.
  */

  if (have_fastchunks(av)) {
    assert(in_smallbin_range(nb));
    malloc_consolidate(av);
    return public_mALLOc(nb - MALLOC_ALIGN_MASK);
  }


#if HAVE_MMAP

  /*
    If have mmap, and the request size meets the mmap threshold, and
    the system supports mmap, and there are few enough currently
    allocated mmapped regions, try to directly map this request
    rather than expanding top.
  */

  if ((CHUNK_SIZE_T)(nb) >= (CHUNK_SIZE_T)(mp_.mmap_threshold) &&
      (mp_.n_mmaps < mp_.n_mmaps_max)) {

    char* mm;             /* return value from mmap call*/

    /*
      Round up size to nearest page.  For mmapped chunks, the overhead
      is one SIZE_SZ unit larger than for normal chunks, because there
      is no following chunk whose prev_size field could be used.
    */
    size = (nb + SIZE_SZ + MALLOC_ALIGN_MASK + pagemask) & ~pagemask;

    /* Don't try if size wraps around 0 */
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb)) {

      mm = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE));

      if (mm != MAP_FAILED) {

        /*
          The offset to the start of the mmapped region is stored
          in the prev_size field of the chunk. This allows us to adjust
          returned start address to meet alignment requirements here
          and in memalign(), and still be able to compute proper
          address argument for later munmap in free() and realloc().
        */

        front_misalign = (INTERNAL_SIZE_T)chunk2mem(mm) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {
          correction = MALLOC_ALIGNMENT - front_misalign;
          p = (mchunkptr)(mm + correction);
          p->prev_size = correction;
          set_head(p, (size - correction) |IS_MMAPPED);
        }
        else {
          p = (mchunkptr)mm;
          p->prev_size = 0;
          set_head(p, size|IS_MMAPPED);
        }

        /* update statistics */

        if (++mp_.n_mmaps > mp_.max_n_mmaps)
          mp_.max_n_mmaps = mp_.n_mmaps;

        sum = mp_.mmapped_mem += size;
        if (sum > (CHUNK_SIZE_T)(mp_.max_mmapped_mem))
          mp_.max_mmapped_mem = sum;
#ifdef NO_THREADS
        sum += av->system_mem;
        if (sum > (CHUNK_SIZE_T)(mp_.max_total_mem))
          mp_.max_total_mem = sum;
#endif

        memory_hook->inc_heap(size);
        check_chunk(av, p);

        return chunk2mem(p);
      }
    }
  }
#endif

  /* Record incoming configuration of top */

  old_top  = av->top;
  old_size = chunksize(old_top);
  old_end  = (char*)(chunk_at_offset(old_top, old_size));

  brk = snd_brk = (char*)(MORECORE_FAILURE); 

  /* 
     If not the first time through, we require old_size to be
     at least MINSIZE and to have prev_inuse set.
  */

  assert((old_top == initial_top(av) && old_size == 0) || 
         ((CHUNK_SIZE_T) (old_size) >= MINSIZE &&
          prev_inuse(old_top) &&
          ((CHUNK_SIZE_T)old_end & pagemask) == 0));

  /* Precondition: not enough current space to satisfy nb request */
  assert((CHUNK_SIZE_T)(old_size) < (CHUNK_SIZE_T)(nb + MINSIZE));

  /* Precondition: all fastbins are consolidated */
  assert(!have_fastchunks(av));


  if (av != &main_arena) {

    heap_info *old_heap, *heap;
    size_t old_heap_size;

    /* First try to extend the current heap. */
    old_heap = heap_for_ptr(old_top);
    old_heap_size = old_heap->size;
    if (grow_heap(old_heap, MINSIZE + nb - old_size) == 0) {
      av->system_mem += old_heap->size - old_heap_size;
      arena_mem += old_heap->size - old_heap_size;
#if 0
      if(mmapped_mem + arena_mem + sbrked_mem > max_total_mem)
        max_total_mem = mmapped_mem + arena_mem + sbrked_mem;
#endif
      set_head(old_top, (((char *)old_heap + old_heap->size) - (char *)old_top)
               | PREV_INUSE);
    }
    else if ((heap = new_heap(nb + (MINSIZE + sizeof(*heap)), mp_.top_pad))) {
      /* Use a newly allocated heap.  */
      heap->ar_ptr = av;
      heap->prev = old_heap;
      av->system_mem += heap->size;
      arena_mem += heap->size;
#if 0
      if((CHUNK_SIZE_T)(mmapped_mem + arena_mem + sbrked_mem) > max_total_mem)
        max_total_mem = mmapped_mem + arena_mem + sbrked_mem;
#endif
      /* Set up the new top.  */
      top(av) = chunk_at_offset(heap, sizeof(*heap));
      set_head(top(av), (heap->size - sizeof(*heap)) | PREV_INUSE);

      /* Setup fencepost and free the old top chunk. */
      /* The fencepost takes at least MINSIZE bytes, because it might
         become the top chunk again later.  Note that a footer is set
         up, too, although the chunk is marked in use. */
      old_size -= MINSIZE;
      set_head(chunk_at_offset(old_top, old_size + 2*SIZE_SZ), 0|PREV_INUSE);
      if (old_size >= MINSIZE) {
        set_head(chunk_at_offset(old_top, old_size), (2*SIZE_SZ)|PREV_INUSE);
        set_foot(chunk_at_offset(old_top, old_size), (2*SIZE_SZ));
        set_head(old_top, old_size|PREV_INUSE|NON_MAIN_ARENA);
        _int_free(av, chunk2mem(old_top));
      } else {
        set_head(old_top, (old_size + 2*SIZE_SZ)|PREV_INUSE);
        set_foot(old_top, (old_size + 2*SIZE_SZ));
      }
    }

  } else { /* av == main_arena */


  /* Request enough space for nb + pad + overhead */

  size = nb + mp_.top_pad + MINSIZE;

  /*
    If contiguous, we can subtract out existing space that we hope to
    combine with new space. We add it back later only if
    we don't actually get contiguous space.
  */

  if (contiguous(av))
    size -= old_size;

  /*
    Round to a multiple of page size.
    If MORECORE is not contiguous, this ensures that we only call it
    with whole-page arguments.  And if MORECORE is contiguous and
    this is not first time through, this preserves page-alignment of
    previous calls. Otherwise, we correct to page-align below.
  */

  size = (size + pagemask) & ~pagemask;

  /*
    Don't try to call MORECORE if argument is so big as to appear
    negative. Note that since mmap takes size_t arg, it may succeed
    below even if we cannot call MORECORE.
  */

  if (size > 0) 
    brk = (char*)(MORECORE(size));

  if (brk != (char*)(MORECORE_FAILURE)) {
    /* Call the `morecore' hook if necessary.  */
    if (__after_morecore_hook)
      (*__after_morecore_hook) ();
    memory_hook->inc_heap(size);
  } else {
  /*
    If have mmap, try using it as a backup when MORECORE fails or
    cannot be used. This is worth doing on systems that have "holes" in
    address space, so sbrk cannot extend to give contiguous space, but
    space is available elsewhere.  Note that we ignore mmap max count
    and threshold limits, since the space will not be used as a
    segregated mmap region.
  */

#if HAVE_MMAP
    /* Cannot merge with old top, so add its size back in */
    if (contiguous(av))
      size = (size + old_size + pagemask) & ~pagemask;

    /* If we are relying on mmap as backup, then use larger units */
    if ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(MMAP_AS_MORECORE_SIZE))
      size = MMAP_AS_MORECORE_SIZE;

    /* Don't try if size wraps around 0 */
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb)) {

      char *mbrk = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE));

      if (mbrk != MAP_FAILED) {

        /* We do not need, and cannot use, another sbrk call to find end */
        brk = mbrk;
        snd_brk = brk + size;

        /*
           Record that we no longer have a contiguous sbrk region.
           After the first time mmap is used as backup, we do not
           ever rely on contiguous space since this could incorrectly
           bridge regions.
        */
        set_noncontiguous(av);
      }
    }
#endif
  }

  if (brk != (char*)(MORECORE_FAILURE)) {
    if (mp_.sbrk_base == 0)
      mp_.sbrk_base = brk;
    av->system_mem += size;

    /*
      If MORECORE extends previous space, we can likewise extend top size.
    */

    if (brk == old_end && snd_brk == (char*)(MORECORE_FAILURE))
      set_head(old_top, (size + old_size) | PREV_INUSE);

    else if (contiguous(av) && old_size && brk < old_end) {
      /* Oops!  Someone else killed our space..  Can't touch anything.  */
      assert(0);
    }

    /*
      Otherwise, make adjustments:
      
      * If the first time through or noncontiguous, we need to call sbrk
        just to find out where the end of memory lies.

      * We need to ensure that all returned chunks from malloc will meet
        MALLOC_ALIGNMENT

      * If there was an intervening foreign sbrk, we need to adjust sbrk
        request size to account for fact that we will not be able to
        combine new space with existing space in old_top.

      * Almost all systems internally allocate whole pages at a time, in
        which case we might as well use the whole last page of request.
        So we allocate enough more memory to hit a page boundary now,
        which in turn causes future contiguous calls to page-align.
    */
    
    else {
      /* Count foreign sbrk as system_mem.  */
      if (old_size)
        av->system_mem += brk - old_end;
      front_misalign = 0;
      end_misalign = 0;
      correction = 0;
      aligned_brk = brk;

      /*
        If MORECORE returns an address lower than we have seen before,
        we know it isn't really contiguous.  This and some subsequent
        checks help cope with non-conforming MORECORE functions and
        the presence of "foreign" calls to MORECORE from outside of
        malloc or by other threads.  We cannot guarantee to detect
        these in all cases, but cope with the ones we do detect.
      */
      if (contiguous(av) && old_size != 0 && brk < old_end) {
        set_noncontiguous(av);
      }
      
      /* handle contiguous cases */
      if (contiguous(av)) { 

        /* Guarantee alignment of first new chunk made from this space */

        front_misalign = (INTERNAL_SIZE_T)chunk2mem(brk) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {

          /*
            Skip over some bytes to arrive at an aligned position.
            We don't need to specially mark these wasted front bytes.
            They will never be accessed anyway because
            prev_inuse of av->top (and any chunk created from its start)
            is always true after initialization.
          */

          correction = MALLOC_ALIGNMENT - front_misalign;
          aligned_brk += correction;
        }
        
        /*
          If this isn't adjacent to existing space, then we will not
          be able to merge with old_top space, so must add to 2nd request.
        */
        
        correction += old_size;
        
        /* Extend the end address to hit a page boundary */
        end_misalign = (INTERNAL_SIZE_T)(brk + size + correction);
        correction += ((end_misalign + pagemask) & ~pagemask) - end_misalign;
        
        assert(correction >= 0);
        snd_brk = (char*)(MORECORE(correction));

        /*
          If can't allocate correction, try to at least find out current
          brk.  It might be enough to proceed without failing.

          Note that if second sbrk did NOT fail, we assume that space
          is contiguous with first sbrk. This is a safe assumption unless
          program is multithreaded but doesn't use locks and a foreign sbrk
          occurred between our first and second calls.
        */

        if (snd_brk == (char*)(MORECORE_FAILURE)) {
          /*
            If can't allocate correction, try to at least find out current
            brk.  It might be enough to proceed without failing.
          */
          correction = 0;
          snd_brk = (char*)(MORECORE(0));
        }
        else if (snd_brk < brk) {
          /*
            If the second call gives noncontiguous space even though
            it says it won't, the only course of action is to ignore
            results of second call, and conservatively estimate where
            the first call left us. Also set noncontiguous, so this
            won't happen again, leaving at most one hole.
            
            Note that this check is intrinsically incomplete.  Because
            MORECORE is allowed to give more space than we ask for,
            there is no reliable way to detect a noncontiguity
            producing a forward gap for the second call.
          */
          snd_brk = brk + size;
          correction = 0;
          set_noncontiguous(av);
        } else {
          /* Call the `morecore' hook if necessary.  */
          if (__after_morecore_hook)
            (*__after_morecore_hook) ();
          memory_hook->inc_heap(correction);
        }
      }

      /* handle non-contiguous cases */
      else {
        /* MORECORE/mmap must correctly align */
        assert(aligned_OK(chunk2mem(brk)));

        /* Find out current end of memory */
        if (snd_brk == (char*)(MORECORE_FAILURE)) {
          snd_brk = (char*)(MORECORE(0));
        }
      }

      /* Adjust top based on results of second sbrk */
      if (snd_brk != (char*)(MORECORE_FAILURE)) {
        av->top = (mchunkptr)aligned_brk;
        set_head(av->top, (snd_brk - aligned_brk + correction) | PREV_INUSE);
        av->system_mem += correction;

        /*
          If not the first time through, we either have a
          gap due to foreign sbrk or a non-contiguous region.  Insert a
          double fencepost at old_top to prevent consolidation with space
          we don't own. These fenceposts are artificial chunks that are
          marked as inuse and are in any case too small to use.  We need
          two to make sizes and alignments work out.
        */
   
        if (old_size != 0) {
          /* 
             Shrink old_top to insert fenceposts, keeping size a
             multiple of MALLOC_ALIGNMENT. We know there is at least
             enough space in old_top to do this.
          */
          old_size = (old_size - 4*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
          set_head(old_top, old_size | PREV_INUSE);
          
          /*
            Note that the following assignments completely overwrite
            old_top when old_size was previously MINSIZE.  This is
            intentional. We need the fencepost, even if old_top otherwise gets
            lost.
          */
          chunk_at_offset(old_top, old_size            )->size =
            (2*SIZE_SZ)|PREV_INUSE;

          chunk_at_offset(old_top, old_size + 2*SIZE_SZ)->size =
            (2*SIZE_SZ)|PREV_INUSE;

          /* If possible, release the rest. */
          if (old_size >= MINSIZE) {
            _int_free(av, chunk2mem(old_top));
          }

        }
      }
    }

    /* Update statistics */
#ifdef NO_THREADS
    sum = av->system_mem + mp_.mmapped_mem;
    if (sum > (CHUNK_SIZE_T)(mp_.max_total_mem))
      mp_.max_total_mem = sum;
#endif

  }

  } /* if (av !=  &main_arena) */

  if ((CHUNK_SIZE_T)av->system_mem > (CHUNK_SIZE_T)(av->max_system_mem))
    av->max_system_mem = av->system_mem;
  check_malloc_state(av);
    
    /* finally, do the allocation */

    p = av->top;
    size = chunksize(p);
    
    /* check that one of the above allocation paths succeeded */
    if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      av->top = remainder;
    set_head(p, nb | PREV_INUSE | (av != &main_arena ? NON_MAIN_ARENA : 0));
      set_head(remainder, remainder_size | PREV_INUSE);
    check_malloced_chunk(av, p, nb);
      return chunk2mem(p);
    }


  /* catch all failure paths */
  MALLOC_FAILURE_ACTION;
  return 0;
}




/*
  sYSTRIm is an inverse of sorts to sYSMALLOc.  It gives memory back
  to the system (via negative arguments to sbrk) if there is unused
  memory at the `high' end of the malloc pool. It is called
  automatically by free() when top space exceeds the trim
  threshold. It is also called by the public malloc_trim routine.  It
  returns 1 if it actually released any memory, else 0.
*/

#if __STD_C
static int sYSTRIm(size_t pad, mstate av)
#else
static int sYSTRIm(pad, av) size_t pad; mstate av;
#endif
{
  long  top_size;        /* Amount of top-most memory */
  long  extra;           /* Amount to release */
  long  released;        /* Amount actually released */
  char* current_brk;     /* address returned by pre-check sbrk call */
  char* new_brk;         /* address returned by post-check sbrk call */
  size_t pagesz;

  pagesz = mp_.pagesize;
  top_size = chunksize(av->top);
  
  /* Release in pagesize units, keeping at least one page */
  extra = ((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;
  
  if (extra > 0) {
    
    /*
      Only proceed if end of memory is where we last set it.
      This avoids problems if there were foreign sbrk calls.
    */
    current_brk = (char*)(MORECORE(0));
    if (current_brk == (char*)(av->top) + top_size) {
      
      /*
        Attempt to release memory. We ignore MORECORE return value,
        and instead call again to find out where new end of memory is.
        This avoids problems if first call releases less than we asked,
        of if failure somehow altered brk value. (We could still
        encounter problems if it altered brk in some very bad way,
        but the only thing we can do is adjust anyway, which will cause
        some downstream failure.)
      */
      
      MORECORE(-extra);
      /* Call the `morecore' hook if necessary.  */
      if (__after_morecore_hook)
        (*__after_morecore_hook) ();
      memory_hook->dec_heap(extra);
      new_brk = (char*)(MORECORE(0));
      
      if (new_brk != (char*)MORECORE_FAILURE) {
        released = (long)(current_brk - new_brk);
        
        if (released != 0) {
          /* Success. Adjust top. */
          av->system_mem -= released;
          set_head(av->top, (top_size - released) | PREV_INUSE);
          check_malloc_state(av);
          return 1;
        }
      }
    }
  }
  return 0;
}

#ifdef HAVE_MMAP

static void
internal_function
#if __STD_C
munmap_chunk(mchunkptr p)
#else
munmap_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T size = chunksize(p);
  int ret;

  assert (chunk_is_mmapped(p));
#if 0
  assert(! ((char*)p >= mp_.sbrk_base && (char*)p < mp_.sbrk_base + mp_.sbrked_mem));
  assert((mp_.n_mmaps > 0));
#endif
  assert(((p->prev_size + size) & (mp_.pagesize-1)) == 0);

  mp_.n_mmaps--;
  mp_.mmapped_mem -= (size + p->prev_size);

  memory_hook->dec_heap(size + p->prev_size);
  ret = munmap((char *)p - p->prev_size, size + p->prev_size);

  /* munmap returns non-zero on failure */
  assert(ret == 0);
}

#if HAVE_MREMAP

static mchunkptr
internal_function
#if __STD_C
mremap_chunk(mchunkptr p, size_t new_size)
#else
mremap_chunk(p, new_size) mchunkptr p; size_t new_size;
#endif
{
  size_t page_mask = mp_.pagesize - 1;
  INTERNAL_SIZE_T offset = p->prev_size;
  INTERNAL_SIZE_T size = chunksize(p);
  char *cp;

  assert (chunk_is_mmapped(p));
#if 0
  assert(! ((char*)p >= mp_.sbrk_base && (char*)p < mp_.sbrk_base + mp_.sbrked_mem));
  assert((mp_.n_mmaps > 0));
#endif
  assert(((size + offset) & (mp_.pagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_size = (new_size + offset + SIZE_SZ + page_mask) & ~page_mask;

  cp = (char *)mremap((char *)p - offset, size + offset, new_size,
                      MREMAP_MAYMOVE);

  if (cp == MAP_FAILED) return 0;

  p = (mchunkptr)(cp + offset);

  assert(aligned_OK(chunk2mem(p)));

  assert((p->prev_size == offset));
  set_head(p, (new_size - offset)|IS_MMAPPED);

  mp_.mmapped_mem -= size + offset;
  mp_.mmapped_mem += new_size;
  if ((unsigned long)mp_.mmapped_mem > (unsigned long)mp_.max_mmapped_mem)
    mp_.max_mmapped_mem = mp_.mmapped_mem;
#ifdef NO_THREADS
  if ((unsigned long)(mp_.mmapped_mem + arena_mem + main_arena.system_mem) >
      mp_.max_total_mem)
    mp_.max_total_mem = mp_.mmapped_mem + arena_mem + main_arena.system_mem;
#endif
  return p;
}

#endif /* HAVE_MREMAP */

#endif /* HAVE_MMAP */

/*------------------------ Public wrappers. --------------------------------*/

Void_t*
public_mALLOc(size_t bytes)
{
  mstate ar_ptr;
  Void_t *victim;

  __malloc_ptr_t (*hook) __MALLOC_P ((size_t, __const __malloc_ptr_t)) =
    __malloc_hook;
  if (hook != nullptr)
    return (*hook)(bytes, RETURN_ADDRESS (0));

  arena_get(ar_ptr, bytes);
  if(!ar_ptr)
    return 0;
  victim = _int_malloc(ar_ptr, bytes);
  if(!victim) {
    /* Maybe the failure is due to running out of mmapped areas. */
    if(ar_ptr != &main_arena) {
      (void)mutex_unlock(&ar_ptr->mutex);
      (void)mutex_lock(&main_arena.mutex);
      victim = _int_malloc(&main_arena, bytes);
      (void)mutex_unlock(&main_arena.mutex);
    } else {
#if USE_ARENAS
      /* ... or sbrk() has failed and there is still a chance to mmap() */
      ar_ptr = arena_get2(ar_ptr->next ? ar_ptr : 0, bytes);
      (void)mutex_unlock(&main_arena.mutex);
      if(ar_ptr) {
        victim = _int_malloc(ar_ptr, bytes);
        (void)mutex_unlock(&ar_ptr->mutex);
      }
#endif
    }
  } else
    (void)mutex_unlock(&ar_ptr->mutex);
  assert(!victim || chunk_is_mmapped(mem2chunk(victim)) ||
         ar_ptr == arena_for_chunk(mem2chunk(victim)));
  return victim;
}

void
public_fREe(Void_t* mem)
{
  mstate ar_ptr;
  mchunkptr p;                          /* chunk corresponding to mem */

  void (*hook) __MALLOC_P ((__malloc_ptr_t, __const __malloc_ptr_t)) =
    __free_hook;
  if (hook != nullptr) {
    (*hook)(mem, RETURN_ADDRESS (0));
    return;
  }

  if (mem == 0)                              /* free(0) has no effect */
    return;

  p = mem2chunk(mem);

#if HAVE_MMAP
  if (chunk_is_mmapped(p))                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    return;
  }
#endif

  ar_ptr = arena_for_chunk(p);
#if THREAD_STATS
  if(!mutex_trylock(&ar_ptr->mutex))
    ++(ar_ptr->stat_lock_direct);
  else {
    (void)mutex_lock(&ar_ptr->mutex);
    ++(ar_ptr->stat_lock_wait);
  }
#else
  (void)mutex_lock(&ar_ptr->mutex);
#endif
  _int_free(ar_ptr, mem);
  (void)mutex_unlock(&ar_ptr->mutex);
}

Void_t*
public_rEALLOc(Void_t* oldmem, size_t bytes)
{
  mstate ar_ptr;
  INTERNAL_SIZE_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  INTERNAL_SIZE_T    oldsize; /* its size */

  Void_t* newp;             /* chunk to return */

  __malloc_ptr_t (*hook) __MALLOC_P ((__malloc_ptr_t, size_t,
                                      __const __malloc_ptr_t)) =
    __realloc_hook;
  if (hook != nullptr)
    return (*hook)(oldmem, bytes, RETURN_ADDRESS (0));

#if REALLOC_ZERO_BYTES_FREES
  if (bytes == 0 && oldmem != nullptr) { public_fREe(oldmem); return 0; }
#endif

  /* realloc of null is supposed to be same as malloc */
  if (oldmem == 0) return public_mALLOc(bytes);

  oldp    = mem2chunk(oldmem);
  oldsize = chunksize(oldp);

  checked_request2size(bytes, nb);

#if HAVE_MMAP
  if (chunk_is_mmapped(oldp))
  {
    Void_t* newmem;

#if HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    if(newp) return chunk2mem(newp);
#endif
    /* Note the extra SIZE_SZ overhead. */
    if(oldsize - SIZE_SZ >= nb) return oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = public_mALLOc(bytes);
    if (newmem == 0) return 0; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
    munmap_chunk(oldp);
    return newmem;
  }
#endif

  ar_ptr = arena_for_chunk(oldp);
#if THREAD_STATS
  if(!mutex_trylock(&ar_ptr->mutex))
    ++(ar_ptr->stat_lock_direct);
  else {
    (void)mutex_lock(&ar_ptr->mutex);
    ++(ar_ptr->stat_lock_wait);
  }
#else
  (void)mutex_lock(&ar_ptr->mutex);
#endif

#ifndef NO_THREADS
  /* As in malloc(), remember this arena for the next allocation. */
  tsd_setspecific(arena_key, (Void_t *)ar_ptr);
#endif

  newp = _int_realloc(ar_ptr, oldmem, bytes);

  (void)mutex_unlock(&ar_ptr->mutex);
  assert(!newp || chunk_is_mmapped(mem2chunk(newp)) ||
         ar_ptr == arena_for_chunk(mem2chunk(newp)));
  return newp;
}

Void_t*
public_mEMALIGn(size_t alignment, size_t bytes)
{
  mstate ar_ptr;
  Void_t *p;

  __malloc_ptr_t (*hook) __MALLOC_PMT ((size_t, size_t,
                                        __const __malloc_ptr_t)) =
    __memalign_hook;
  if (hook != nullptr)
    return (*hook)(alignment, bytes, RETURN_ADDRESS (0));

  /* If need less alignment than we give anyway, just relay to malloc */
  if (alignment <= MALLOC_ALIGNMENT) return public_mALLOc(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */
  if (alignment <  MINSIZE) alignment = MINSIZE;

  arena_get(ar_ptr, bytes + alignment + MINSIZE);
  if(!ar_ptr)
    return 0;
  p = _int_memalign(ar_ptr, alignment, bytes);
  (void)mutex_unlock(&ar_ptr->mutex);
  if(!p) {
    /* Maybe the failure is due to running out of mmapped areas. */
    if(ar_ptr != &main_arena) {
      (void)mutex_lock(&main_arena.mutex);
      p = _int_memalign(&main_arena, alignment, bytes);
      (void)mutex_unlock(&main_arena.mutex);
    } else {
#if USE_ARENAS
      /* ... or sbrk() has failed and there is still a chance to mmap() */
      ar_ptr = arena_get2(ar_ptr->next ? ar_ptr : 0, bytes);
      if(ar_ptr) {
        p = _int_memalign(ar_ptr, alignment, bytes);
        (void)mutex_unlock(&ar_ptr->mutex);
      }
#endif
    }
  }
  assert(!p || chunk_is_mmapped(mem2chunk(p)) ||
         ar_ptr == arena_for_chunk(mem2chunk(p)));
  return p;
}

Void_t*
public_vALLOc(size_t bytes)
{
  mstate ar_ptr;
  Void_t *p;

  if(__malloc_initialized < 0)
    ptmalloc_init ();
  arena_get(ar_ptr, bytes + mp_.pagesize + MINSIZE);
  if(!ar_ptr)
    return 0;
  p = _int_valloc(ar_ptr, bytes);
  (void)mutex_unlock(&ar_ptr->mutex);
  return p;
}

Void_t*
public_pVALLOc(size_t bytes)
{
  mstate ar_ptr;
  Void_t *p;

  if(__malloc_initialized < 0)
    ptmalloc_init ();
  arena_get(ar_ptr, bytes + 2*mp_.pagesize + MINSIZE);
  p = _int_pvalloc(ar_ptr, bytes);
  (void)mutex_unlock(&ar_ptr->mutex);
  return p;
}

Void_t*
public_cALLOc(size_t n, size_t elem_size)
{
  mstate av;
  mchunkptr oldtop, p;
  INTERNAL_SIZE_T sz, csz, oldtopsize;
  Void_t* mem;
  unsigned long clearsize;
  unsigned long nclears;
  INTERNAL_SIZE_T* d;

  __malloc_ptr_t (*hook) __MALLOC_PMT ((size_t, __const __malloc_ptr_t)) =
    __malloc_hook;
  if (hook != nullptr) {
    sz = n * elem_size;
    mem = (*hook)(sz, RETURN_ADDRESS (0));
    if(mem == 0)
      return 0;
#ifdef HAVE_MEMCPY
    return memset(mem, 0, sz);
#else
    while(sz > 0) ((char*)mem)[--sz] = 0; /* rather inefficient */
    return mem;
#endif
  }

  /* FIXME: check for overflow on multiplication.  */
  sz = n * elem_size;

  arena_get(av, sz);
  if(!av)
    return 0;

  /* Check if we hand out the top chunk, in which case there may be no
     need to clear. */
#if MORECORE_CLEARS
  oldtop = top(av);
  oldtopsize = chunksize(top(av));
#if MORECORE_CLEARS < 2
  /* Only newly allocated memory is guaranteed to be cleared.  */
  if (av == &main_arena &&
      oldtopsize < mp_.sbrk_base + av->max_system_mem - (char *)oldtop)
    oldtopsize = (mp_.sbrk_base + av->max_system_mem - (char *)oldtop);
#endif
#endif
  mem = _int_malloc(av, sz);

  /* Only clearing follows, so we can unlock early. */
  (void)mutex_unlock(&av->mutex);

  assert(!mem || chunk_is_mmapped(mem2chunk(mem)) ||
         av == arena_for_chunk(mem2chunk(mem)));

  if (mem == 0) {
    /* Maybe the failure is due to running out of mmapped areas. */
    if(av != &main_arena) {
      (void)mutex_lock(&main_arena.mutex);
      mem = _int_malloc(&main_arena, sz);
      (void)mutex_unlock(&main_arena.mutex);
    } else {
#if USE_ARENAS
      /* ... or sbrk() has failed and there is still a chance to mmap() */
      (void)mutex_lock(&main_arena.mutex);
      av = arena_get2(av->next ? av : 0, sz);
      (void)mutex_unlock(&main_arena.mutex);
      if(av) {
        mem = _int_malloc(av, sz);
        (void)mutex_unlock(&av->mutex);
      }
#endif
    }
    if (mem == 0) return 0;
  }
  p = mem2chunk(mem);

  /* Two optional cases in which clearing not necessary */
#if HAVE_MMAP
  if (chunk_is_mmapped(p))
    return mem;
#endif

  csz = chunksize(p);

#if MORECORE_CLEARS
  if (p == oldtop && csz > oldtopsize) {
    /* clear only the bytes from non-freshly-sbrked memory */
    csz = oldtopsize;
  }
#endif

  /* Unroll clear of <= 36 bytes (72 if 8byte sizes).  We know that
     contents have an odd number of INTERNAL_SIZE_T-sized words;
     minimally 3.  */
  d = (INTERNAL_SIZE_T*)mem;
  clearsize = csz - SIZE_SZ;
  nclears = clearsize / sizeof(INTERNAL_SIZE_T);
  assert(nclears >= 3);

  if (nclears > 9)
    MALLOC_ZERO(d, clearsize);

  else {
    *(d+0) = 0;
    *(d+1) = 0;
    *(d+2) = 0;
    if (nclears > 4) {
      *(d+3) = 0;
      *(d+4) = 0;
      if (nclears > 6) {
        *(d+5) = 0;
        *(d+6) = 0;
        if (nclears > 8) {
          *(d+7) = 0;
          *(d+8) = 0;
        }
      }
    }
  }

  return mem;
}

Void_t**
public_iCALLOc(size_t n, size_t elem_size, Void_t** chunks)
{
  mstate ar_ptr;
  Void_t** m;

  arena_get(ar_ptr, n*elem_size);
  if(!ar_ptr)
    return 0;

  m = _int_icalloc(ar_ptr, n, elem_size, chunks);
  (void)mutex_unlock(&ar_ptr->mutex);
  return m;
}

Void_t**
public_iCOMALLOc(size_t n, size_t sizes[], Void_t** chunks)
{
  mstate ar_ptr;
  Void_t** m;

  arena_get(ar_ptr, 0);
  if(!ar_ptr)
    return 0;

  m = _int_icomalloc(ar_ptr, n, sizes, chunks);
  (void)mutex_unlock(&ar_ptr->mutex);
  return m;
}

#ifndef _LIBC

void
public_cFREe(Void_t* m)
{
  public_fREe(m);
}

#endif /* _LIBC */

int
public_mTRIm(size_t s)
{
  int result;

  (void)mutex_lock(&main_arena.mutex);
  result = mTRIm(s);
  (void)mutex_unlock(&main_arena.mutex);
  return result;
}

size_t
public_mUSABLe(Void_t* m)
{
  size_t result;

  result = mUSABLe(m);
  return result;
}

void
public_mSTATs()
{
  mSTATs();
}

struct mallinfo public_mALLINFo()
{
  struct mallinfo m;

  (void)mutex_lock(&main_arena.mutex);
  m = mALLINFo(&main_arena);
  (void)mutex_unlock(&main_arena.mutex);
  return m;
}

int
public_mALLOPt(int p, int v)
{
  int result;
  result = mALLOPt(p, v);
  return result;
}

/*
  ------------------------------ malloc ------------------------------
*/

Void_t*
_int_malloc __MALLOC_P((mstate av, size_t bytes))
{
  INTERNAL_SIZE_T nb;               /* normalized request size */
  unsigned int    idx;              /* associated bin index */
  mbinptr         bin;              /* associated bin */
  mfastbinptr*    fb;               /* associated fastbin */

  mchunkptr       victim;           /* inspected/selected chunk */
  INTERNAL_SIZE_T size;             /* its size */
  int             victim_index;     /* its bin index */

  mchunkptr       remainder;        /* remainder from a split */
  CHUNK_SIZE_T    remainder_size;   /* its size */

  unsigned int    block;            /* bit map traverser */
  unsigned int    bit;              /* bit map traverser */
  unsigned int    map;              /* current word of binmap */

  mchunkptr       fwd;              /* misc temp for linking */
  mchunkptr       bck;              /* misc temp for linking */

  /*
    Convert request size to internal form by adding SIZE_SZ bytes
    overhead plus possibly more to obtain necessary alignment and/or
    to obtain a size of at least MINSIZE, the smallest allocatable
    size. Also, checked_request2size traps (returning 0) request sizes
    that are so large that they wrap around zero when padded and
    aligned.
  */

  checked_request2size(bytes, nb);

  /*
    Bypass search if no frees yet
   */
  if (!have_anychunks(av)) {
    if (av->max_fast == 0) /* initialization check */
      malloc_consolidate(av);
    goto use_top;
  }

  /*
    If the size qualifies as a fastbin, first check corresponding bin.
  */

  if ((CHUNK_SIZE_T)(nb) <= (CHUNK_SIZE_T)(av->max_fast)) { 
    fb = &(av->fastbins[(fastbin_index(nb))]);
    if ( (victim = *fb) != 0) {
      *fb = victim->fd;
      check_remalloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
  }

  /*
    If a small request, check regular bin.  Since these "smallbins"
    hold one size each, no searching within bins is necessary.
    (For a large request, we need to wait until unsorted chunks are
    processed to find best fit. But for small ones, fits are exact
    anyway, so we can check now, which is faster.)
  */

  if (in_smallbin_range(nb)) {
    idx = smallbin_index(nb);
    bin = bin_at(av,idx);

    if ( (victim = last(bin)) != bin) {
      if (victim == 0) /* initialization check */
        malloc_consolidate(av);
      else {
      bck = victim->bk;
      set_inuse_bit_at_offset(victim, nb);
      bin->bk = bck;
      bck->fd = bin;
      
        if (av != &main_arena)
          victim->size |= NON_MAIN_ARENA;
        check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
      }
    }
  }

  /* 
     If this is a large request, consolidate fastbins before continuing.
     While it might look excessive to kill all fastbins before
     even seeing if there is space available, this avoids
     fragmentation problems normally associated with fastbins.
     Also, in practice, programs tend to have runs of either small or
     large requests, but less often mixtures, so consolidation is not 
     invoked all that often in most programs. And the programs that
     it is called frequently in otherwise tend to fragment.
  */

  else {
    idx = largebin_index(nb);
    if (have_fastchunks(av)) 
      malloc_consolidate(av);
  }

  /*
    Process recently freed or remaindered chunks, taking one only if
    it is exact fit, or, if this a small request, the chunk is remainder from
    the most recent non-exact fit.  Place other traversed chunks in
    bins.  Note that this step is the only place in any routine where
    chunks are placed in bins.

    The outer loop here is needed because we might not realize until
    near the end of malloc that we should have consolidated, so must
    do so and retry. This happens at most once, and only when we would
    otherwise need to expand memory to service a "small" request.
  */

  for(;;) {

  while ( (victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) {
    bck = victim->bk;
    size = chunksize(victim);
    
    /* 
       If a small request, try to use last remainder if it is the
       only chunk in unsorted bin.  This helps promote locality for
       runs of consecutive small requests. This is the only
       exception to best-fit, and applies only when there is
       no exact fit for a small chunk.
    */
    
    if (in_smallbin_range(nb) && 
        bck == unsorted_chunks(av) &&
        victim == av->last_remainder &&
        (CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      
      /* split and reattach remainder */
      remainder_size = size - nb;
      remainder = chunk_at_offset(victim, nb);
      unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
      av->last_remainder = remainder; 
      remainder->bk = remainder->fd = unsorted_chunks(av);
      
        set_head(victim, nb | PREV_INUSE |
                 (av != &main_arena ? NON_MAIN_ARENA : 0));
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      
        check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
    
    /* remove from unsorted list */
    unsorted_chunks(av)->bk = bck;
    bck->fd = unsorted_chunks(av);
    
    /* Take now instead of binning if exact fit */
    
    if (size == nb) {
      set_inuse_bit_at_offset(victim, size);
        if (av != &main_arena)
          victim->size |= NON_MAIN_ARENA;
        check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
    
    /* place chunk in bin */
    
    if (in_smallbin_range(size)) {
      victim_index = smallbin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
    }
    else {
      victim_index = largebin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
      
      if (fwd != bck) {
        /* if smaller than smallest, place first */
          assert((bck->bk->size & NON_MAIN_ARENA) == 0);
        if ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(bck->bk->size)) {
          fwd = bck;
          bck = bck->bk;
        }
        else if ((CHUNK_SIZE_T)(size) >= 
                 (CHUNK_SIZE_T)(FIRST_SORTED_BIN_SIZE)) {
          
          /* maintain large bins in sorted order */
          size |= PREV_INUSE; /* Or with inuse bit to speed comparisons */
            assert((fwd->size & NON_MAIN_ARENA) == 0);
                while ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(fwd->size)) {
            fwd = fwd->fd;
              assert((fwd->size & NON_MAIN_ARENA) == 0);
            }
          bck = fwd->bk;
        }
      }
    }
      
    mark_bin(av, victim_index);
    victim->bk = bck;
    victim->fd = fwd;
    fwd->bk = victim;
    bck->fd = victim;
  }
  
  /*
      If a large request, scan through the chunks of current bin in
      sorted order to find smallest that fits.  This is the only step
      where an unbounded number of chunks might be scanned without doing
      anything useful with them. However the lists tend to be short.
  */
  
  if (!in_smallbin_range(nb)) {
    bin = bin_at(av, idx);
    
    for (victim = last(bin); victim != bin; victim = victim->bk) {
      size = chunksize(victim);
      
      if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb)) {
        remainder_size = size - nb;
        unlink(victim, bck, fwd);
        
        /* Exhaust */
        if (remainder_size < MINSIZE)  {
          set_inuse_bit_at_offset(victim, size);
            if (av != &main_arena)
              victim->size |= NON_MAIN_ARENA;
            check_malloced_chunk(av, victim, nb);
          return chunk2mem(victim);
        }
        /* Split */
        else {
          remainder = chunk_at_offset(victim, nb);
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
            set_head(victim, nb | PREV_INUSE |
                     (av != &main_arena ? NON_MAIN_ARENA : 0));
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
            check_malloced_chunk(av, victim, nb);
          return chunk2mem(victim);
        } 
      }
    }    
  }

  /*
    Search for a chunk by scanning bins, starting with next largest
    bin. This search is strictly by best-fit; i.e., the smallest
    (with ties going to approximately the least recently used) chunk
    that fits is selected.
    
    The bitmap avoids needing to check that most blocks are nonempty.
      The particular case of skipping all bins during warm-up phases
      when no chunks have been returned yet is faster than it might look.
  */
    
  ++idx;
  bin = bin_at(av,idx);
  block = idx2block(idx);
  map = av->binmap[block];
  bit = idx2bit(idx);
  
  for (;;) {
    
    /* Skip rest of block if there are no more set bits in this block.  */
    if (bit > map || bit == 0) {
      do {
        if (++block >= BINMAPSIZE)  /* out of bins */
          goto use_top;
      } while ( (map = av->binmap[block]) == 0);
      
      bin = bin_at(av, (block << BINMAPSHIFT));
      bit = 1;
    }
    
    /* Advance to bin with set bit. There must be one. */
    while ((bit & map) == 0) {
      bin = next_bin(bin);
      bit <<= 1;
      assert(bit != 0);
    }
    
    /* Inspect the bin. It is likely to be non-empty */
    victim = last(bin);
    
    /*  If a false alarm (empty bin), clear the bit. */
    if (victim == bin) {
      av->binmap[block] = map &= ~bit; /* Write through */
      bin = next_bin(bin);
      bit <<= 1;
    }
    
    else {
      size = chunksize(victim);
      
      /*  We know the first chunk in this bin is big enough to use. */
      assert((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb));
      
      remainder_size = size - nb;
      
      /* unlink */
      bck = victim->bk;
      bin->bk = bck;
      bck->fd = bin;
      
      /* Exhaust */
      if (remainder_size < MINSIZE) {
        set_inuse_bit_at_offset(victim, size);
          if (av != &main_arena)
            victim->size |= NON_MAIN_ARENA;
          check_malloced_chunk(av, victim, nb);
        return chunk2mem(victim);
      }
      
      /* Split */
      else {
        remainder = chunk_at_offset(victim, nb);
        
        unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
        remainder->bk = remainder->fd = unsorted_chunks(av);
        /* advertise as last remainder */
        if (in_smallbin_range(nb)) 
          av->last_remainder = remainder; 
        
          set_head(victim, nb | PREV_INUSE |
                   (av != &main_arena ? NON_MAIN_ARENA : 0));
        set_head(remainder, remainder_size | PREV_INUSE);
        set_foot(remainder, remainder_size);
          check_malloced_chunk(av, victim, nb);
        return chunk2mem(victim);
      }
    }
  }

  use_top:    
  /*
    If large enough, split off the chunk bordering the end of memory
    (held in av->top). Note that this is in accord with the best-fit
    search rule.  In effect, av->top is treated as larger (and thus
    less well fitting) than any other available chunk since it can
    be extended to be as large as necessary (up to system
    limitations).
    
    We require that av->top always exists (i.e., has size >=
    MINSIZE) after initialization, so if it would otherwise be
    exhuasted by current request, it is replenished. (The main
    reason for ensuring it exists is that we may need MINSIZE space
    to put in fenceposts in sysmalloc.)
  */
  
  victim = av->top;
  size = chunksize(victim);
  
  if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
    remainder_size = size - nb;
    remainder = chunk_at_offset(victim, nb);
    av->top = remainder;
      set_head(victim, nb | PREV_INUSE |
               (av != &main_arena ? NON_MAIN_ARENA : 0));
    set_head(remainder, remainder_size | PREV_INUSE);
    
      check_malloced_chunk(av, victim, nb);
    return chunk2mem(victim);
  }
  
  /* 
      If there is space available in fastbins, consolidate and retry,
      to possibly avoid expanding memory. This can occur only if nb is
      in smallbin range so we didn't consolidate upon entry.
    */

    else if (have_fastchunks(av)) {
      assert(in_smallbin_range(nb));
      malloc_consolidate(av);
      idx = smallbin_index(nb); /* restore original bin index */
    }

    /*
       Otherwise, relay to handle system-dependent cases
    */
    else
      return sYSMALLOc(nb, av);
  }
}

/*
  ------------------------------ free ------------------------------
*/

void
_int_free __MALLOC_P((mstate av, Void_t* mem))
{
  mchunkptr       p;           /* chunk corresponding to mem */
  INTERNAL_SIZE_T size;        /* its size */
  mfastbinptr*    fb;          /* associated fastbin */
  mchunkptr       nextchunk;   /* next contiguous chunk */
  INTERNAL_SIZE_T nextsize;    /* its size */
  int             nextinuse;   /* true if nextchunk is used */
  INTERNAL_SIZE_T prevsize;    /* size of previous contiguous chunk */
  mchunkptr       bck;         /* misc temp for linking */
  mchunkptr       fwd;         /* misc temp for linking */

  /* free(0) has no effect */
  if (mem != 0) {
    p = mem2chunk(mem);
    size = chunksize(p);

    check_inuse_chunk(av, p);

    /*
      If eligible, place chunk on a fastbin so it can be found
      and used quickly in malloc.
    */

    if ((CHUNK_SIZE_T)(size) <= (CHUNK_SIZE_T)(av->max_fast)

#if TRIM_FASTBINS
        /* 
           If TRIM_FASTBINS set, don't place chunks
           bordering top into fastbins
        */
        && (chunk_at_offset(p, size) != av->top)
#endif
        ) {

      set_fastchunks(av);
      fb = &(av->fastbins[fastbin_index(size)]);
      p->fd = *fb;
      *fb = p;
    }

    /*
       Consolidate other non-mmapped chunks as they arrive.
    */

    else if (!chunk_is_mmapped(p)) {
      nextchunk = chunk_at_offset(p, size);
      nextsize = chunksize(nextchunk);
      assert(nextsize > 0);

      /* consolidate backward */
      if (!prev_inuse(p)) {
        prevsize = p->prev_size;
        size += prevsize;
        p = chunk_at_offset(p, -((long) prevsize));
        unlink(p, bck, fwd);
      }

      if (nextchunk != av->top) {
        /* get and clear inuse bit */
        nextinuse = inuse_bit_at_offset(nextchunk, nextsize);

        /* consolidate forward */
        if (!nextinuse) {
          unlink(nextchunk, bck, fwd);
          size += nextsize;
        } else
          clear_inuse_bit_at_offset(nextchunk, 0);

        /*
          Place the chunk in unsorted chunk list. Chunks are
          not placed into regular bins until after they have
          been given one chance to be used in malloc.
        */

        bck = unsorted_chunks(av);
        fwd = bck->fd;
        p->bk = bck;
        p->fd = fwd;
        bck->fd = p;
        fwd->bk = p;

        set_head(p, size | PREV_INUSE);
        set_foot(p, size);
        
        check_free_chunk(av, p);
      }

      /*
         If the chunk borders the current high end of memory,
         consolidate into top
      */

      else {
        size += nextsize;
        set_head(p, size | PREV_INUSE);
        av->top = p;
        check_chunk(av, p);
      }

      /*
        If freeing a large space, consolidate possibly-surrounding
        chunks. Then, if the total unused topmost memory exceeds trim
        threshold, ask malloc_trim to reduce top.

        Unless max_fast is 0, we don't know if there are fastbins
        bordering top, so we cannot tell for sure whether threshold
        has been reached unless fastbins are consolidated.  But we
        don't want to consolidate on each free.  As a compromise,
        consolidation is performed if FASTBIN_CONSOLIDATION_THRESHOLD
        is reached.
      */

      if ((CHUNK_SIZE_T)(size) >= FASTBIN_CONSOLIDATION_THRESHOLD) { 
        if (have_fastchunks(av)) 
          malloc_consolidate(av);

        if (av == &main_arena) {
#ifndef MORECORE_CANNOT_TRIM
        if ((CHUNK_SIZE_T)(chunksize(av->top)) >= 
              (CHUNK_SIZE_T)(mp_.trim_threshold))
            sYSTRIm(mp_.top_pad, av);
#endif
        } else {
          /* Always try heap_trim(), even if the top chunk is not
             large, because the corresponding heap might go away.  */
          heap_info *heap = heap_for_ptr(top(av));

          assert(heap->ar_ptr == av);
          heap_trim(heap, mp_.top_pad);
        }
      }

    }
    /*
      If the chunk was allocated via mmap, release via munmap(). Note
      that if HAVE_MMAP is false but chunk_is_mmapped is true, then
      user must have overwritten memory. There's nothing we can do to
      catch this error unless MALLOC_DEBUG is set, in which case
      check_inuse_chunk (above) will have triggered error.
    */

    else {
#if HAVE_MMAP
      int ret;
      INTERNAL_SIZE_T offset = p->prev_size;
      mp_.n_mmaps--;
      mp_.mmapped_mem -= (size + offset);
      ret = munmap((char*)p - offset, size + offset);
      /* munmap returns non-zero on failure */
      assert(ret == 0);
#endif
    }
  }
}

/*
  ------------------------- malloc_consolidate -------------------------

  malloc_consolidate is a specialized version of free() that tears
  down chunks held in fastbins.  Free itself cannot be used for this
  purpose since, among other things, it might place chunks back onto
  fastbins.  So, instead, we need to use a minor variant of the same
  code.
  
  Also, because this routine needs to be called the first time through
  malloc anyway, it turns out to be the perfect place to trigger
  initialization code.
*/

#if __STD_C
static void malloc_consolidate(mstate av)
#else
static void malloc_consolidate(av) mstate av;
#endif
{
  mfastbinptr*    fb;                 /* current fastbin being consolidated */
  mfastbinptr*    maxfb;              /* last fastbin (for loop control) */
  mchunkptr       p;                  /* current chunk being consolidated */
  mchunkptr       nextp;              /* next chunk to consolidate */
  mchunkptr       unsorted_bin;       /* bin header */
  mchunkptr       first_unsorted;     /* chunk to link to */

  /* These have same use as in free() */
  mchunkptr       nextchunk;
  INTERNAL_SIZE_T size;
  INTERNAL_SIZE_T nextsize;
  INTERNAL_SIZE_T prevsize;
  int             nextinuse;
  mchunkptr       bck;
  mchunkptr       fwd;

  /*
    If max_fast is 0, we know that av hasn't
    yet been initialized, in which case do so below
  */

  if (av->max_fast != 0) {
    clear_fastchunks(av);

    unsorted_bin = unsorted_chunks(av);

    /*
      Remove each chunk from fast bin and consolidate it, placing it
      then in unsorted bin. Among other reasons for doing this,
      placing in unsorted bin avoids needing to calculate actual bins
      until malloc is sure that chunks aren't immediately going to be
      reused anyway.
    */
    
    maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
    fb = &(av->fastbins[0]);
    do {
      if ( (p = *fb) != 0) {
        *fb = 0;
        
        do {
          check_inuse_chunk(av, p);
          nextp = p->fd;
          
          /* Slightly streamlined version of consolidation code in free() */
          size = p->size & ~(PREV_INUSE|NON_MAIN_ARENA);
          nextchunk = chunk_at_offset(p, size);
          nextsize = chunksize(nextchunk);
          
          if (!prev_inuse(p)) {
            prevsize = p->prev_size;
            size += prevsize;
            p = chunk_at_offset(p, -((long) prevsize));
            unlink(p, bck, fwd);
          }
          
          if (nextchunk != av->top) {
            nextinuse = inuse_bit_at_offset(nextchunk, nextsize);

            if (!nextinuse) {
              size += nextsize;
              unlink(nextchunk, bck, fwd);
            } else
              clear_inuse_bit_at_offset(nextchunk, 0);
            
            first_unsorted = unsorted_bin->fd;
            unsorted_bin->fd = p;
            first_unsorted->bk = p;
            
            set_head(p, size | PREV_INUSE);
            p->bk = unsorted_bin;
            p->fd = first_unsorted;
            set_foot(p, size);
          }
          
          else {
            size += nextsize;
            set_head(p, size | PREV_INUSE);
            av->top = p;
          }
          
        } while ( (p = nextp) != 0);
        
      }
    } while (fb++ != maxfb);
  }
  else {
    malloc_init_state(av);
    check_malloc_state(av);
  }
}

/*
  ------------------------------ realloc ------------------------------
*/

Void_t*
_int_realloc __MALLOC_P((mstate av, Void_t* oldmem, size_t bytes))
{
  INTERNAL_SIZE_T  nb;              /* padded request size */

  mchunkptr        oldp;            /* chunk corresponding to oldmem */
  INTERNAL_SIZE_T  oldsize;         /* its size */

  mchunkptr        newp;            /* chunk to return */
  INTERNAL_SIZE_T  newsize;         /* its size */
  Void_t*          newmem;          /* corresponding user mem */

  mchunkptr        next;            /* next contiguous chunk after oldp */

  mchunkptr        remainder;       /* extra space at end of newp */
  CHUNK_SIZE_T     remainder_size;  /* its size */

  mchunkptr        bck;             /* misc temp for linking */
  mchunkptr        fwd;             /* misc temp for linking */

  CHUNK_SIZE_T     copysize;        /* bytes to copy */
  unsigned int     ncopies;         /* INTERNAL_SIZE_T words to copy */
  INTERNAL_SIZE_T* s;               /* copy source */ 
  INTERNAL_SIZE_T* d;               /* copy destination */


#ifdef REALLOC_ZERO_BYTES_FREES
  if (bytes == 0) {
    _int_free(av, oldmem);
    return 0;
  }
#endif

  /* realloc of null is supposed to be same as malloc */
  if (oldmem == 0) return _int_malloc(av, bytes);

  checked_request2size(bytes, nb);

  oldp    = mem2chunk(oldmem);
  oldsize = chunksize(oldp);

  check_inuse_chunk(av, oldp);

  if (!chunk_is_mmapped(oldp)) {

    if ((CHUNK_SIZE_T)(oldsize) >= (CHUNK_SIZE_T)(nb)) {
      /* already big enough; split below */
      newp = oldp;
      newsize = oldsize;
    }

    else {
      next = chunk_at_offset(oldp, oldsize);

      /* Try to expand forward into top */
      if (next == av->top &&
          (CHUNK_SIZE_T)(newsize = oldsize + chunksize(next)) >=
          (CHUNK_SIZE_T)(nb + MINSIZE)) {
        set_head_size(oldp, nb | (av != &main_arena ? NON_MAIN_ARENA : 0));
        av->top = chunk_at_offset(oldp, nb);
        set_head(av->top, (newsize - nb) | PREV_INUSE);
        check_inuse_chunk(av, oldp);
        return chunk2mem(oldp);
      }
      
      /* Try to expand forward into next chunk;  split off remainder below */
      else if (next != av->top && 
               !inuse(next) &&
               (CHUNK_SIZE_T)(newsize = oldsize + chunksize(next)) >=
               (CHUNK_SIZE_T)(nb)) {
        newp = oldp;
        unlink(next, bck, fwd);
      }

      /* allocate, copy, free */
      else {
        newmem = _int_malloc(av, nb - MALLOC_ALIGN_MASK);
        if (newmem == 0)
          return 0; /* propagate failure */
      
        newp = mem2chunk(newmem);
        newsize = chunksize(newp);
        
        /*
          Avoid copy if newp is next chunk after oldp.
        */
        if (newp == next) {
          newsize += oldsize;
          newp = oldp;
        }
        else {
          /*
            Unroll copy of <= 36 bytes (72 if 8byte sizes)
            We know that contents have an odd number of
            INTERNAL_SIZE_T-sized words; minimally 3.
          */
          
          copysize = oldsize - SIZE_SZ;
          s = (INTERNAL_SIZE_T*)(oldmem);
          d = (INTERNAL_SIZE_T*)(newmem);
          ncopies = copysize / sizeof(INTERNAL_SIZE_T);
          assert(ncopies >= 3);
          
          if (ncopies > 9)
            MALLOC_COPY(d, s, copysize);
          
          else {
            *(d+0) = *(s+0);
            *(d+1) = *(s+1);
            *(d+2) = *(s+2);
            if (ncopies > 4) {
              *(d+3) = *(s+3);
              *(d+4) = *(s+4);
              if (ncopies > 6) {
                *(d+5) = *(s+5);
                *(d+6) = *(s+6);
                if (ncopies > 8) {
                  *(d+7) = *(s+7);
                  *(d+8) = *(s+8);
                }
              }
            }
          }
          
          _int_free(av, oldmem);
          check_inuse_chunk(av, newp);
          return chunk2mem(newp);
        }
      }
    }

    /* If possible, free extra space in old or extended chunk */

    assert((CHUNK_SIZE_T)(newsize) >= (CHUNK_SIZE_T)(nb));

    remainder_size = newsize - nb;

    if (remainder_size < MINSIZE) { /* not enough extra to split off */
      set_head_size(newp, newsize | (av != &main_arena ? NON_MAIN_ARENA : 0));
      set_inuse_bit_at_offset(newp, newsize);
    }
    else { /* split remainder */
      remainder = chunk_at_offset(newp, nb);
      set_head_size(newp, nb | (av != &main_arena ? NON_MAIN_ARENA : 0));
      set_head(remainder, remainder_size | PREV_INUSE |
               (av != &main_arena ? NON_MAIN_ARENA : 0));
      /* Mark remainder as inuse so free() won't complain */
      set_inuse_bit_at_offset(remainder, remainder_size);
      _int_free(av, chunk2mem(remainder));
    }

    check_inuse_chunk(av, newp);
    return chunk2mem(newp);
  }

  /*
    Handle mmap cases
  */

  else {
#if HAVE_MMAP

#if HAVE_MREMAP
    INTERNAL_SIZE_T offset = oldp->prev_size;
    size_t pagemask = mp_.pagesize - 1;
    char *cp;
    CHUNK_SIZE_T  sum;
    
    /* Note the extra SIZE_SZ overhead */
    newsize = (nb + offset + SIZE_SZ + pagemask) & ~pagemask;

    /* don't need to remap if still within same page */
    if (oldsize == newsize - offset) 
      return oldmem;

    cp = (char*)mremap((char*)oldp - offset, oldsize + offset, newsize, 1);
    
    if (cp != MAP_FAILED) {

      newp = (mchunkptr)(cp + offset);
      set_head(newp, (newsize - offset)|IS_MMAPPED);
      
      assert(aligned_OK(chunk2mem(newp)));
      assert((newp->prev_size == offset));
      
      /* update statistics */
      sum = mp_.mmapped_mem += newsize - oldsize;
      if (sum > (CHUNK_SIZE_T)(mp_.max_mmapped_mem))
        mp_.max_mmapped_mem = sum;
#ifdef NO_THREADS
      sum += main_arena.system_mem;
      if (sum > (CHUNK_SIZE_T)(mp_.max_total_mem))
        mp_.max_total_mem = sum;
#endif

      return chunk2mem(newp);
    }
#endif

    /* Note the extra SIZE_SZ overhead. */
    if ((CHUNK_SIZE_T)(oldsize) >= (CHUNK_SIZE_T)(nb + SIZE_SZ)) 
      newmem = oldmem; /* do nothing */
    else {
      /* Must alloc, copy, free. */
      newmem = _int_malloc(av, nb - MALLOC_ALIGN_MASK);
      if (newmem != 0) {
        MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
        _int_free(av, oldmem);
      }
    }
    return newmem;

#else
    /* If !HAVE_MMAP, but chunk_is_mmapped, user must have overwritten mem */
    check_malloc_state(av);
    MALLOC_FAILURE_ACTION;
    return 0;
#endif
  }
}

/*
  ------------------------------ memalign ------------------------------
*/

Void_t*
_int_memalign __MALLOC_P((mstate av, size_t alignment, size_t bytes))
{
  INTERNAL_SIZE_T nb;             /* padded  request size */
  char*           m;              /* memory returned by malloc call */
  mchunkptr       p;              /* corresponding chunk */
  char*           brk;            /* alignment point within p */
  mchunkptr       newp;           /* chunk to return */
  INTERNAL_SIZE_T newsize;        /* its size */
  INTERNAL_SIZE_T leadsize;       /* leading space before alignment point */
  mchunkptr       remainder;      /* spare room at end to split off */
  CHUNK_SIZE_T    remainder_size; /* its size */
  INTERNAL_SIZE_T size;

  /* If need less alignment than we give anyway, just relay to malloc */

  if (alignment <= MALLOC_ALIGNMENT) return _int_malloc(av, bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  if (alignment <  MINSIZE) alignment = MINSIZE;

  /* Make sure alignment is power of 2 (in case MINSIZE is not).  */
  if ((alignment & (alignment - 1)) != 0) {
    size_t a = MALLOC_ALIGNMENT * 2;
    while ((CHUNK_SIZE_T)a < (CHUNK_SIZE_T)alignment) a <<= 1;
    alignment = a;
  }

  checked_request2size(bytes, nb);

  /*
    Strategy: find a spot within that chunk that meets the alignment
    request, and then possibly free the leading and trailing space.
  */


  /* Call malloc with worst case padding to hit alignment. */

  m  = (char*)(_int_malloc(av, nb + alignment + MINSIZE));

  if (m == 0) return 0; /* propagate failure */

  p = mem2chunk(m);

  if ((((PTR_UINT)(m)) % alignment) != 0) { /* misaligned */

    /*
      Find an aligned spot inside chunk.  Since we need to give back
      leading space in a chunk of at least MINSIZE, if the first
      calculation places us at a spot with less than MINSIZE leader,
      we can move to the next aligned spot -- we've allocated enough
      total room so that this is always possible.
    */

    brk = (char*)mem2chunk((PTR_UINT)((PTR_UINT)(m + alignment - 1)) &
                           -((signed long) alignment));
    if ((CHUNK_SIZE_T)(brk - (char*)(p)) < MINSIZE)
      brk += alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (char*)(p);
    newsize = chunksize(p) - leadsize;

    /* For mmapped chunks, just adjust offset */
    if (chunk_is_mmapped(p)) {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      return chunk2mem(newp);
    }

    /* Otherwise, give back leader, use the rest */
    set_head(newp, newsize | PREV_INUSE |
             (av != &main_arena ? NON_MAIN_ARENA : 0));
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize | (av != &main_arena ? NON_MAIN_ARENA : 0));
    _int_free(av, chunk2mem(p));
    p = newp;

    assert (newsize >= nb &&
            (((PTR_UINT)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */
  if (!chunk_is_mmapped(p)) {
    size = chunksize(p);
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      set_head(remainder, remainder_size | PREV_INUSE |
               (av != &main_arena ? NON_MAIN_ARENA : 0));
      set_head_size(p, nb);
      _int_free(av, chunk2mem(remainder));
    }
  }

  check_inuse_chunk(av, p);
  return chunk2mem(p);
}

#if 0
/*
  ------------------------------ calloc ------------------------------
*/

#if __STD_C
Void_t* cALLOc(size_t n_elements, size_t elem_size)
#else
Void_t* cALLOc(n_elements, elem_size) size_t n_elements; size_t elem_size;
#endif
{
  mchunkptr p;
  CHUNK_SIZE_T  clearsize;
  CHUNK_SIZE_T  nclears;
  INTERNAL_SIZE_T* d;

  Void_t* mem = mALLOc(n_elements * elem_size);

  if (mem != 0) {
    p = mem2chunk(mem);

#if MMAP_CLEARS
    if (!chunk_is_mmapped(p)) /* don't need to clear mmapped space */
#endif
    {
      /*
        Unroll clear of <= 36 bytes (72 if 8byte sizes)
        We know that contents have an odd number of
        INTERNAL_SIZE_T-sized words; minimally 3.
      */

      d = (INTERNAL_SIZE_T*)mem;
      clearsize = chunksize(p) - SIZE_SZ;
      nclears = clearsize / sizeof(INTERNAL_SIZE_T);
      assert(nclears >= 3);

      if (nclears > 9)
        MALLOC_ZERO(d, clearsize);

      else {
        *(d+0) = 0;
        *(d+1) = 0;
        *(d+2) = 0;
        if (nclears > 4) {
          *(d+3) = 0;
          *(d+4) = 0;
          if (nclears > 6) {
            *(d+5) = 0;
            *(d+6) = 0;
            if (nclears > 8) {
              *(d+7) = 0;
              *(d+8) = 0;
            }
          }
        }
      }
    }
#if ! MMAP_CLEARS
    else
    {
      d = (INTERNAL_SIZE_T*)mem;
      /*
        Note the additional SIZE_SZ
      */
      clearsize = chunksize(p) - 2*SIZE_SZ;
      MALLOC_ZERO(d, clearsize);
    }
#endif
  }
  return mem;
}

/*
  ------------------------------ cfree ------------------------------
*/

#if __STD_C
void cFREe(Void_t *mem)
#else
void cFREe(mem) Void_t *mem;
#endif
{
  fREe(mem);
}
#endif /* 0 */

/*
  ------------------------- independent_calloc -------------------------
*/

Void_t**
#if __STD_C
_int_icalloc(mstate av, size_t n_elements, size_t elem_size, Void_t* chunks[])
#else
_int_icalloc(av, n_elements, elem_size, chunks)
mstate av; size_t n_elements; size_t elem_size; Void_t* chunks[];
#endif
{
  size_t sz = elem_size; /* serves as 1-element array */
  /* opts arg of 3 means all elements are same size, and should be cleared */
  return iALLOc(av, n_elements, &sz, 3, chunks);
}

/*
  ------------------------- independent_comalloc -------------------------
*/

Void_t**
#if __STD_C
_int_icomalloc(mstate av, size_t n_elements, size_t sizes[], Void_t* chunks[])
#else
_int_icomalloc(av, n_elements, sizes, chunks)
mstate av; size_t n_elements; size_t sizes[]; Void_t* chunks[];
#endif
{
  return iALLOc(av, n_elements, sizes, 0, chunks);
}


/*
  ------------------------------ ialloc ------------------------------
  ialloc provides common support for independent_X routines, handling all of
  the combinations that can result.

  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/


static Void_t**
#if __STD_C
iALLOc(mstate av, size_t n_elements, size_t* sizes, int opts, Void_t* chunks[])
#else
iALLOc(av, n_elements, sizes, opts, chunks)
mstate av; size_t n_elements; size_t* sizes; int opts; Void_t* chunks[];
#endif
{
  INTERNAL_SIZE_T element_size;   /* chunksize of each element, if all same */
  INTERNAL_SIZE_T contents_size;  /* total size of elements */
  INTERNAL_SIZE_T array_size;     /* request size of pointer array */
  Void_t*         mem;            /* malloced aggregate space */
  mchunkptr       p;              /* corresponding chunk */
  INTERNAL_SIZE_T remainder_size; /* remaining bytes while splitting */
  Void_t**        marray;         /* either "chunks" or malloced ptr array */
  mchunkptr       array_chunk;    /* chunk for malloced ptr array */
  int             mmx;            /* to disable mmap */
  INTERNAL_SIZE_T size;           
  INTERNAL_SIZE_T size_flags;
  size_t          i;

  /* Ensure initialization/consolidation */
  if (have_fastchunks(av)) malloc_consolidate(av);

  /* compute array length, if needed */
  if (chunks != 0) {
    if (n_elements == 0)
      return chunks; /* nothing to do */
    marray = chunks;
    array_size = 0;
  }
  else {
    /* if empty req, must still return chunk representing empty array */
    if (n_elements == 0) 
      return (Void_t**) _int_malloc(av, 0);
    marray = 0;
    array_size = request2size(n_elements * (sizeof(Void_t*)));
  }

  /* compute total element size */
  if (opts & 0x1) { /* all-same-size */
    element_size = request2size(*sizes);
    contents_size = n_elements * element_size;
  }
  else { /* add up all the sizes */
    element_size = 0;
    contents_size = 0;
    for (i = 0; i != n_elements; ++i) 
      contents_size += request2size(sizes[i]);     
  }

  /* subtract out alignment bytes from total to minimize overallocation */
  size = contents_size + array_size - MALLOC_ALIGN_MASK;
  
  /* 
     Allocate the aggregate chunk.
     But first disable mmap so malloc won't use it, since
     we would not be able to later free/realloc space internal
     to a segregated mmap region.
 */
  mmx = mp_.n_mmaps_max;   /* disable mmap */
  mp_.n_mmaps_max = 0;
  mem = _int_malloc(av, size);
  mp_.n_mmaps_max = mmx;   /* reset mmap */
  if (mem == 0) 
    return 0;

  p = mem2chunk(mem);
  assert(!chunk_is_mmapped(p)); 
  remainder_size = chunksize(p);

  if (opts & 0x2) {       /* optionally clear the elements */
    MALLOC_ZERO(mem, remainder_size - SIZE_SZ - array_size);
  }

  size_flags = PREV_INUSE | (av != &main_arena ? NON_MAIN_ARENA : 0);

  /* If not provided, allocate the pointer array as final part of chunk */
  if (marray == 0) {
    array_chunk = chunk_at_offset(p, contents_size);
    marray = (Void_t**) (chunk2mem(array_chunk));
    set_head(array_chunk, (remainder_size - contents_size) | size_flags);
    remainder_size = contents_size;
  }

  /* split out elements */
  for (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    if (i != n_elements-1) {
      if (element_size != 0)
        size = element_size;
      else
        size = request2size(sizes[i]);
      remainder_size -= size;
      set_head(p, size | size_flags);
      p = chunk_at_offset(p, size);
    }
    else { /* the final element absorbs any overallocation slop */
      set_head(p, remainder_size | size_flags);
      break;
    }
  }

#if MALLOC_DEBUG
  if (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    if (element_size != 0) 
      assert(remainder_size == element_size);
    else
      assert(remainder_size == request2size(sizes[i]));
    check_inuse_chunk(av, mem2chunk(marray));
  }

  for (i = 0; i != n_elements; ++i)
    check_inuse_chunk(av, mem2chunk(marray[i]));
#endif

  return marray;
}


/*
  ------------------------------ valloc ------------------------------
*/

Void_t*
#if __STD_C
_int_valloc(mstate av, size_t bytes)
#else
_int_valloc(av, bytes) mstate av; size_t bytes;
#endif
{
  /* Ensure initialization/consolidation */
  if (have_fastchunks(av)) malloc_consolidate(av);
  return _int_memalign(av, mp_.pagesize, bytes);
}

/*
  ------------------------------ pvalloc ------------------------------
*/


Void_t*
#if __STD_C
_int_pvalloc(mstate av, size_t bytes)
#else
_int_pvalloc(av, bytes) mstate av, size_t bytes;
#endif
{
  size_t pagesz;

  /* Ensure initialization/consolidation */
  if (have_fastchunks(av)) malloc_consolidate(av);
  pagesz = mp_.pagesize;
  return _int_memalign(av, pagesz, (bytes + pagesz - 1) & ~(pagesz - 1));
}


/*
  ------------------------------ malloc_trim ------------------------------
*/

#if __STD_C
int mTRIm(size_t pad)
#else
int mTRIm(pad) size_t pad;
#endif
{
  mstate av = &main_arena; /* already locked */
  /* Ensure initialization/consolidation */
  malloc_consolidate(av);

#ifndef MORECORE_CANNOT_TRIM        
  return sYSTRIm(pad, av);
#else
  return 0;
#endif
}


/*
  ------------------------- malloc_usable_size -------------------------
*/

#if __STD_C
size_t mUSABLe(Void_t* mem)
#else
size_t mUSABLe(mem) Void_t* mem;
#endif
{
  mchunkptr p;
  if (mem != 0) {
    p = mem2chunk(mem);
    if (chunk_is_mmapped(p))
      return chunksize(p) - 2*SIZE_SZ;
    else if (inuse(p))
      return chunksize(p) - SIZE_SZ;
  }
  return 0;
}

/*
  ------------------------------ mallinfo ------------------------------
*/

struct mallinfo mALLINFo(mstate av)
{
  struct mallinfo mi;
  int i;
  mbinptr b;
  mchunkptr p;
  INTERNAL_SIZE_T avail;
  INTERNAL_SIZE_T fastavail;
  int nblocks;
  int nfastblocks;

  /* Ensure initialization */
  if (av->top == 0)  malloc_consolidate(av);

  check_malloc_state(av);

  /* Account for top */
  avail = chunksize(av->top);
  nblocks = 1;  /* top always exists */

  /* traverse fastbins */
  nfastblocks = 0;
  fastavail = 0;

  for (i = 0; i < NFASTBINS; ++i) {
    for (p = av->fastbins[i]; p != 0; p = p->fd) {
      ++nfastblocks;
      fastavail += chunksize(p);
    }
  }

  avail += fastavail;

  /* traverse regular bins */
  for (i = 1; i < NBINS; ++i) {
    b = bin_at(av, i);
    for (p = last(b); p != b; p = p->bk) {
      ++nblocks;
      avail += chunksize(p);
    }
  }

  mi.smblks = nfastblocks;
  mi.ordblks = nblocks;
  mi.fordblks = avail;
  mi.uordblks = av->system_mem - avail;
  mi.arena = av->system_mem;
  mi.hblks = mp_.n_mmaps;
  mi.hblkhd = mp_.mmapped_mem;
  mi.fsmblks = fastavail;
  mi.keepcost = chunksize(av->top);
  mi.usmblks = mp_.max_total_mem;
  return mi;
}

/*
  ------------------------------ malloc_stats ------------------------------
*/

void mSTATs()
{
  int i;
  mstate ar_ptr;
  struct mallinfo mi;
  unsigned int in_use_b = mp_.mmapped_mem, system_b = in_use_b;
#if THREAD_STATS
  long stat_lock_direct = 0, stat_lock_loop = 0, stat_lock_wait = 0;
#endif

#ifdef WIN32
  {
    CHUNK_SIZE_T  free, reserved, committed;
    vminfo (&free, &reserved, &committed);
    fprintf(stderr, "free bytes       = %10lu\n", 
            free);
    fprintf(stderr, "reserved bytes   = %10lu\n", 
            reserved);
    fprintf(stderr, "committed bytes  = %10lu\n", 
            committed);
  }
#endif

  for (i=0, ar_ptr = &main_arena;; i++) {
    (void)mutex_lock(&ar_ptr->mutex);
    mi = mALLINFo(ar_ptr);
    fprintf(stderr, "Arena %d:\n", i);
    fprintf(stderr, "system bytes     = %10u\n", (unsigned int)mi.arena);
    fprintf(stderr, "in use bytes     = %10u\n", (unsigned int)mi.uordblks);
#if MALLOC_DEBUG > 1
    if (i > 0)
      dump_heap(heap_for_ptr(top(ar_ptr)));
#endif
    system_b += mi.arena;
    in_use_b += mi.uordblks;
#if THREAD_STATS
    stat_lock_direct += ar_ptr->stat_lock_direct;
    stat_lock_loop += ar_ptr->stat_lock_loop;
    stat_lock_wait += ar_ptr->stat_lock_wait;
#endif
    (void)mutex_unlock(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
#if HAVE_MMAP
  fprintf(stderr, "Total (incl. mmap):\n");
#else
  fprintf(stderr, "Total:\n");
#endif
  fprintf(stderr, "system bytes     = %10u\n", system_b);
  fprintf(stderr, "in use bytes     = %10u\n", in_use_b);
#ifdef NO_THREADS
  fprintf(stderr, "max system bytes = %10u\n", (unsigned int)mp_.max_total_mem);
#endif
#if HAVE_MMAP
  fprintf(stderr, "max mmap regions = %10u\n", (unsigned int)mp_.max_n_mmaps);
  fprintf(stderr, "max mmap bytes   = %10lu\n",
          (unsigned long)mp_.max_mmapped_mem);
#endif
#if THREAD_STATS
  fprintf(stderr, "heaps created    = %10d\n",  stat_n_heaps);
  fprintf(stderr, "locked directly  = %10ld\n", stat_lock_direct);
  fprintf(stderr, "locked in loop   = %10ld\n", stat_lock_loop);
  fprintf(stderr, "locked waiting   = %10ld\n", stat_lock_wait);
  fprintf(stderr, "locked total     = %10ld\n",
          stat_lock_direct + stat_lock_loop + stat_lock_wait);
#endif

#ifdef WIN32 
  {
    CHUNK_SIZE_T  kernel, user;
    if (cpuinfo (TRUE, &kernel, &user)) {
      fprintf(stderr, "kernel ms        = %10lu\n", 
              kernel);
      fprintf(stderr, "user ms          = %10lu\n", 
              user);
    }
  }
#endif
}


/*
  ------------------------------ mallopt ------------------------------
*/

#if __STD_C
int mALLOPt(int param_number, int value)
#else
int mALLOPt(param_number, value) int param_number; int value;
#endif
{
  mstate av = &main_arena;
  int res = 1;

  (void)mutex_lock(&av->mutex);
  /* Ensure initialization/consolidation */
  malloc_consolidate(av);

  switch(param_number) {
  case M_MXFAST:
    if (value >= 0 && value <= MAX_FAST_SIZE) {
      set_max_fast(av, value);
    }
    else
      res = 0;
    break;

  case M_TRIM_THRESHOLD:
    mp_.trim_threshold = value;
    break;

  case M_TOP_PAD:
    mp_.top_pad = value;
    break;

  case M_MMAP_THRESHOLD:
#if USE_ARENAS
    /* Forbid setting the threshold too high. */
    if((unsigned long)value > HEAP_MAX_SIZE/2)
      res = 0;
    else
#endif
      mp_.mmap_threshold = value;
    break;

  case M_MMAP_MAX:
#if !HAVE_MMAP
    if (value != 0)
      res = 0;
    else
#endif
      mp_.n_mmaps_max = value;
    break;

  case M_CHECK_ACTION:
    check_action = value;
    break;
  }
  (void)mutex_unlock(&av->mutex);
  return res;
}


/* 
  -------------------- Alternative MORECORE functions --------------------
*/


/*
  General Requirements for MORECORE.

  The MORECORE function must have the following properties:

  If MORECORE_CONTIGUOUS is false:

    * MORECORE must allocate in multiples of pagesize. It will
      only be called with arguments that are multiples of pagesize.

    * MORECORE(0) must return an address that is at least 
      MALLOC_ALIGNMENT aligned. (Page-aligning always suffices.)

  else (i.e. If MORECORE_CONTIGUOUS is true):

    * Consecutive calls to MORECORE with positive arguments
      return increasing addresses, indicating that space has been
      contiguously extended. 

    * MORECORE need not allocate in multiples of pagesize.
      Calls to MORECORE need not have args of multiples of pagesize.

    * MORECORE need not page-align.

  In either case:

    * MORECORE may allocate more memory than requested. (Or even less,
      but this will generally result in a malloc failure.)

    * MORECORE must not allocate memory when given argument zero, but
      instead return one past the end address of memory from previous
      nonzero call. This malloc does NOT call MORECORE(0)
      until at least one call with positive arguments is made, so
      the initial value returned is not important.

    * Even though consecutive calls to MORECORE need not return contiguous
      addresses, it must be OK for malloc'ed chunks to span multiple
      regions in those cases where they do happen to be contiguous.

    * MORECORE need not handle negative arguments -- it may instead
      just return MORECORE_FAILURE when given negative arguments.
      Negative arguments are always multiples of pagesize. MORECORE
      must not misinterpret negative args as large positive unsigned
      args. You can suppress all such calls from even occurring by defining
      MORECORE_CANNOT_TRIM,

  There is some variation across systems about the type of the
  argument to sbrk/MORECORE. If size_t is unsigned, then it cannot
  actually be size_t, because sbrk supports negative args, so it is
  normally the signed type of the same width as size_t (sometimes
  declared as "intptr_t", and sometimes "ptrdiff_t").  It doesn't much
  matter though. Internally, we use "long" as arguments, which should
  work across all reasonable possibilities.

  Additionally, if MORECORE ever returns failure for a positive
  request, and HAVE_MMAP is true, then mmap is used as a noncontiguous
  system allocator. This is a useful backup strategy for systems with
  holes in address spaces -- in this case sbrk cannot contiguously
  expand the heap, but mmap may be able to map noncontiguous space.

  If you'd like mmap to ALWAYS be used, you can define MORECORE to be
  a function that always returns MORECORE_FAILURE.

  Malloc only has limited ability to detect failures of MORECORE
  to supply contiguous space when it says it can. In particular,
  multithreaded programs that do not use locks may result in
  rece conditions across calls to MORECORE that result in gaps
  that cannot be detected as such, and subsequent corruption.

  If you are using this malloc with something other than sbrk (or its
  emulation) to supply memory regions, you probably want to set
  MORECORE_CONTIGUOUS as false.  As an example, here is a custom
  allocator kindly contributed for pre-OSX macOS.  It uses virtually
  but not necessarily physically contiguous non-paged memory (locked
  in, present and won't get swapped out).  You can use it by
  uncommenting this section, adding some #includes, and setting up the
  appropriate defines above:

      #define MORECORE osMoreCore
      #define MORECORE_CONTIGUOUS 0

  There is also a shutdown routine that should somehow be called for
  cleanup upon program exit.

  #define MAX_POOL_ENTRIES 100
  #define MINIMUM_MORECORE_SIZE  (64 * 1024)
  static int next_os_pool;
  void *our_os_pools[MAX_POOL_ENTRIES];

  void *osMoreCore(int size)
  {
    void *ptr = 0;
    static void *sbrk_top = 0;

    if (size > 0)
    {
      if (size < MINIMUM_MORECORE_SIZE)
         size = MINIMUM_MORECORE_SIZE;
      if (CurrentExecutionLevel() == kTaskLevel)
         ptr = PoolAllocateResident(size + RM_PAGE_SIZE, 0);
      if (ptr == 0)
      {
        return (void *) MORECORE_FAILURE;
      }
      // save ptrs so they can be freed during cleanup
      our_os_pools[next_os_pool] = ptr;
      next_os_pool++;
      ptr = (void *) ((((CHUNK_SIZE_T) ptr) + RM_PAGE_MASK) & ~RM_PAGE_MASK);
      sbrk_top = (char *) ptr + size;
      return ptr;
    }
    else if (size < 0)
    {
      // we don't currently support shrink behavior
      return (void *) MORECORE_FAILURE;
    }
    else
    {
      return sbrk_top;
    }
  }

  // cleanup any allocated memory pools
  // called as last thing before shutting down driver

  void osCleanupMem(void)
  {
    void **ptr;

    for (ptr = our_os_pools; ptr < &our_os_pools[MAX_POOL_ENTRIES]; ptr++)
      if (*ptr)
      {
         PoolDeallocate(*ptr);
         *ptr = 0;
      }
  }

*/


/* 
  -------------------------------------------------------------- 

  Emulation of sbrk for win32. 
  Donated by J. Walter <Walter@GeNeSys-e.de>.
  For additional information about this code, and malloc on Win32, see 
     http://www.genesys-e.de/jwalter/
*/

#ifdef MORECORE_IS_MMAP
/* Deliberately fails to force use of mmap() */
static void *sbrkfail (long size)
{
        return MORECORE_FAILURE;
}
#endif

#ifdef WIN32

#ifdef _DEBUG
/* #define TRACE */
#endif

/* Wait for spin lock */
static long slwait (long *sl) {
    while (InterlockedCompareExchange ((LONG volatile *) sl, 1, 0) != 0) 
            Sleep (0);
    return 0;
}

/* Try waiting for spin lock */
static long sltrywait (long *sl) {
    return (InterlockedCompareExchange ((LONG volatile *) sl, 1, 0) != 0);
}

/* Release spin lock */
static long slrelease (long *sl) {
    InterlockedExchange (sl, 0);
    return 0;
}

/* Spin lock for emulation code */
static long g_sl;

/* getpagesize for windows */
static long getpagesize (void) {
    static long g_pagesize = 0;
    if (! g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}
static long getregionsize (void) {
    static long g_regionsize = 0;
    if (! g_regionsize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_regionsize = system_info.dwAllocationGranularity;
    }
    return g_regionsize;
}

/* A region list entry */
typedef struct _region_list_entry {
    void *top_allocated;
    void *top_committed;
    void *top_reserved;
    long reserve_size;
    struct _region_list_entry *previous;
} region_list_entry;

/* Allocate and link a region entry in the region list */
static int region_list_append (region_list_entry **last, void *base_reserved, long reserve_size) {
    region_list_entry *next = (region_list_entry *) HeapAlloc (GetProcessHeap (), 0, sizeof (region_list_entry));
    if (! next)
        return FALSE;
    next->top_allocated = (char *) base_reserved;
    next->top_committed = (char *) base_reserved;
    next->top_reserved = (char *) base_reserved + reserve_size;
    next->reserve_size = reserve_size;
    next->previous = *last;
    *last = next;
    return TRUE;
}
/* Free and unlink the last region entry from the region list */
static int region_list_remove (region_list_entry **last) {
    region_list_entry *previous = (*last)->previous;
    if (! HeapFree (GetProcessHeap (), sizeof (region_list_entry), *last))
        return FALSE;
    *last = previous;
    return TRUE;
}

#define CEIL(size,to)   (((size)+(to)-1)&~((to)-1))
#define FLOOR(size,to)  ((size)&~((to)-1))

#define SBRK_SCALE  0
/* #define SBRK_SCALE  1 */
/* #define SBRK_SCALE  2 */
/* #define SBRK_SCALE  4  */

/* sbrk for windows */
static void *sbrk (long size) {
    static long g_pagesize, g_my_pagesize;
    static long g_regionsize, g_my_regionsize;
    static region_list_entry *g_last;
    void *result = (void *) MORECORE_FAILURE;
#ifdef TRACE
    printf ("sbrk %d\n", size);
#endif
    /* Wait for spin lock */
    slwait (&g_sl);
   /* First time initialization */
    if (! g_pagesize) {
        g_pagesize = getpagesize ();
        g_my_pagesize = g_pagesize << SBRK_SCALE;
    }
    if (! g_regionsize) {
        g_regionsize = getregionsize ();
        g_my_regionsize = g_regionsize << SBRK_SCALE;
    }
    if (! g_last) {
        if (! region_list_append (&g_last, 0, 0)) 
           goto sbrk_exit;
    }
    /* Assert invariants */
    assert (g_last);
    assert ((char *) g_last->top_reserved - g_last->reserve_size <= (char *) g_last->top_allocated &&
            g_last->top_allocated <= g_last->top_committed);
    assert ((char *) g_last->top_reserved - g_last->reserve_size <= (char *) g_last->top_committed &&
            g_last->top_committed <= g_last->top_reserved &&
            (unsigned) g_last->top_committed % g_pagesize == 0);
    assert ((unsigned) g_last->top_reserved % g_regionsize == 0);
    assert ((unsigned) g_last->reserve_size % g_regionsize == 0);
    /* Allocation requested? */
    if (size >= 0) {
        /* Allocation size is the requested size */
        long allocate_size = size;
        /* Compute the size to commit */
        long to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;
        /* Do we reach the commit limit? */
        if (to_commit > 0) {
            /* Round size to commit */
            long commit_size = CEIL (to_commit, g_my_pagesize);
            /* Compute the size to reserve */
            long to_reserve = (char *) g_last->top_committed + commit_size - (char *) g_last->top_reserved;
            /* Do we reach the reserve limit? */
            if (to_reserve > 0) {
                /* Compute the remaining size to commit in the current region */
                long remaining_commit_size = (char *) g_last->top_reserved - (char *) g_last->top_committed;
                if (remaining_commit_size > 0) {
                    /* Assert preconditions */
                    assert ((unsigned) g_last->top_committed % g_pagesize == 0);
                    assert (0 < remaining_commit_size && remaining_commit_size % g_pagesize == 0); {
                        /* Commit this */
                        void *base_committed = VirtualAlloc (g_last->top_committed, remaining_commit_size,
                                                                                         MEM_COMMIT, PAGE_READWRITE);
                        /* Check returned pointer for consistency */
                        if (base_committed != g_last->top_committed)
                            goto sbrk_exit;
                        /* Assert postconditions */
                        assert ((unsigned) base_committed % g_pagesize == 0);
#ifdef TRACE
                        printf ("Commit %p %d\n", base_committed, remaining_commit_size);
#endif
                        /* Adjust the regions commit top */
                        g_last->top_committed = (char *) base_committed + remaining_commit_size;
                    }
                } {
                    /* Now we are going to search and reserve. */
                    int contiguous = -1;
                    int found = FALSE;
                    MEMORY_BASIC_INFORMATION memory_info;
                    void *base_reserved;
                    long reserve_size;
                    do {
                        /* Assume contiguous memory */
                        contiguous = TRUE;
                        /* Round size to reserve */
                        reserve_size = CEIL (to_reserve, g_my_regionsize);
                        /* Start with the current region's top */
                        memory_info.BaseAddress = g_last->top_reserved;
                        /* Assert preconditions */
                        assert ((unsigned) memory_info.BaseAddress % g_pagesize == 0);
                        assert (0 < reserve_size && reserve_size % g_regionsize == 0);
                        while (VirtualQuery (memory_info.BaseAddress, &memory_info, sizeof (memory_info))) {
                            /* Assert postconditions */
                            assert ((unsigned) memory_info.BaseAddress % g_pagesize == 0);
#ifdef TRACE
                            printf ("Query %p %d %s\n", memory_info.BaseAddress, memory_info.RegionSize, 
                                    memory_info.State == MEM_FREE ? "FREE": 
                                    (memory_info.State == MEM_RESERVE ? "RESERVED":
                                     (memory_info.State == MEM_COMMIT ? "COMMITTED": "?")));
#endif
                            /* Region is free, well aligned and big enough: we are done */
                            if (memory_info.State == MEM_FREE &&
                                (unsigned) memory_info.BaseAddress % g_regionsize == 0 &&
                                memory_info.RegionSize >= (unsigned) reserve_size) {
                                found = TRUE;
                                break;
                            }
                            /* From now on we can't get contiguous memory! */
                            contiguous = FALSE;
                            /* Recompute size to reserve */
                            reserve_size = CEIL (allocate_size, g_my_regionsize);
                            memory_info.BaseAddress = (char *) memory_info.BaseAddress + memory_info.RegionSize;
                            /* Assert preconditions */
                            assert ((unsigned) memory_info.BaseAddress % g_pagesize == 0);
                            assert (0 < reserve_size && reserve_size % g_regionsize == 0);
                        }
                        /* Search failed? */
                        if (! found) 
                            goto sbrk_exit;
                        /* Assert preconditions */
                        assert ((unsigned) memory_info.BaseAddress % g_regionsize == 0);
                        assert (0 < reserve_size && reserve_size % g_regionsize == 0);
                        /* Try to reserve this */
                        base_reserved = VirtualAlloc (memory_info.BaseAddress, reserve_size, 
                                                                          MEM_RESERVE, PAGE_NOACCESS);
                        if (! base_reserved) {
                            int rc = GetLastError ();
                            if (rc != ERROR_INVALID_ADDRESS) 
                                goto sbrk_exit;
                        }
                        /* A null pointer signals (hopefully) a race condition with another thread. */
                        /* In this case, we try again. */
                    } while (! base_reserved);
                    /* Check returned pointer for consistency */
                    if (memory_info.BaseAddress && base_reserved != memory_info.BaseAddress)
                        goto sbrk_exit;
                    /* Assert postconditions */
                    assert ((unsigned) base_reserved % g_regionsize == 0);
#ifdef TRACE
                    printf ("Reserve %p %d\n", base_reserved, reserve_size);
#endif
                    /* Did we get contiguous memory? */
                    if (contiguous) {
                        long start_size = (char *) g_last->top_committed - (char *) g_last->top_allocated;
                        /* Adjust allocation size */
                        allocate_size -= start_size;
                        /* Adjust the regions allocation top */
                        g_last->top_allocated = g_last->top_committed;
                        /* Recompute the size to commit */
                        to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;
                        /* Round size to commit */
                        commit_size = CEIL (to_commit, g_my_pagesize);
                    } 
                    /* Append the new region to the list */
                    if (! region_list_append (&g_last, base_reserved, reserve_size))
                        goto sbrk_exit;
                    /* Didn't we get contiguous memory? */
                    if (! contiguous) {
                        /* Recompute the size to commit */
                        to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;
                        /* Round size to commit */
                        commit_size = CEIL (to_commit, g_my_pagesize);
                    }
                }
            } 
            /* Assert preconditions */
            assert ((unsigned) g_last->top_committed % g_pagesize == 0);
            assert (0 < commit_size && commit_size % g_pagesize == 0); {
                /* Commit this */
                void *base_committed = VirtualAlloc (g_last->top_committed, commit_size, 
                                                                             MEM_COMMIT, PAGE_READWRITE);
                /* Check returned pointer for consistency */
                if (base_committed != g_last->top_committed)
                    goto sbrk_exit;
                /* Assert postconditions */
                assert ((unsigned) base_committed % g_pagesize == 0);
#ifdef TRACE
                printf ("Commit %p %d\n", base_committed, commit_size);
#endif
                /* Adjust the regions commit top */
                g_last->top_committed = (char *) base_committed + commit_size;
            }
        } 
        /* Adjust the regions allocation top */
        g_last->top_allocated = (char *) g_last->top_allocated + allocate_size;
        result = (char *) g_last->top_allocated - size;
    /* Deallocation requested? */
    } else if (size < 0) {
        long deallocate_size = - size;
        /* As long as we have a region to release */
        while ((char *) g_last->top_allocated - deallocate_size < (char *) g_last->top_reserved - g_last->reserve_size) {
            /* Get the size to release */
            long release_size = g_last->reserve_size;
            /* Get the base address */
            void *base_reserved = (char *) g_last->top_reserved - release_size;
            /* Assert preconditions */
            assert ((unsigned) base_reserved % g_regionsize == 0); 
            assert (0 < release_size && release_size % g_regionsize == 0); {
                /* Release this */
                int rc = VirtualFree (base_reserved, 0, 
                                      MEM_RELEASE);
                /* Check returned code for consistency */
                if (! rc)
                    goto sbrk_exit;
#ifdef TRACE
                printf ("Release %p %d\n", base_reserved, release_size);
#endif
            }
            /* Adjust deallocation size */
            deallocate_size -= (char *) g_last->top_allocated - (char *) base_reserved;
            /* Remove the old region from the list */
            if (! region_list_remove (&g_last))
                goto sbrk_exit;
        } {
            /* Compute the size to decommit */
            long to_decommit = (char *) g_last->top_committed - ((char *) g_last->top_allocated - deallocate_size);
            if (to_decommit >= g_my_pagesize) {
                /* Compute the size to decommit */
                long decommit_size = FLOOR (to_decommit, g_my_pagesize);
                /*  Compute the base address */
                void *base_committed = (char *) g_last->top_committed - decommit_size;
                /* Assert preconditions */
                assert ((unsigned) base_committed % g_pagesize == 0);
                assert (0 < decommit_size && decommit_size % g_pagesize == 0); {
                    /* Decommit this */
                    int rc = VirtualFree ((char *) base_committed, decommit_size, 
                                          MEM_DECOMMIT);
                    /* Check returned code for consistency */
                    if (! rc)
                        goto sbrk_exit;
#ifdef TRACE
                    printf ("Decommit %p %d\n", base_committed, decommit_size);
#endif
                }
                /* Adjust deallocation size and regions commit and allocate top */
                deallocate_size -= (char *) g_last->top_allocated - (char *) base_committed;
                g_last->top_committed = base_committed;
                g_last->top_allocated = base_committed;
            }
        }
        /* Adjust regions allocate top */
        g_last->top_allocated = (char *) g_last->top_allocated - deallocate_size;
        /* Check for underflow */
        if ((char *) g_last->top_reserved - g_last->reserve_size > (char *) g_last->top_allocated ||
            g_last->top_allocated > g_last->top_committed) {
            /* Adjust regions allocate top */
            g_last->top_allocated = (char *) g_last->top_reserved - g_last->reserve_size;
            goto sbrk_exit;
        }
        result = g_last->top_allocated;
    }
    /* Assert invariants */
    assert (g_last);
    assert ((char *) g_last->top_reserved - g_last->reserve_size <= (char *) g_last->top_allocated &&
            g_last->top_allocated <= g_last->top_committed);
    assert ((char *) g_last->top_reserved - g_last->reserve_size <= (char *) g_last->top_committed &&
            g_last->top_committed <= g_last->top_reserved &&
            (unsigned) g_last->top_committed % g_pagesize == 0);
    assert ((unsigned) g_last->top_reserved % g_regionsize == 0);
    assert ((unsigned) g_last->reserve_size % g_regionsize == 0);

sbrk_exit:
    /* Release spin lock */
    slrelease (&g_sl);
    return result;
}

/* mmap for windows */
static void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) {
  static long g_pagesize;
  static long g_regionsize;
  DWORD alloc=MEM_RESERVE|MEM_TOP_DOWN;
  DWORD ntprot=0;
  long rounding=0;
#ifdef TRACE
  printf ("mmap %p %d %d %d\n", ptr, size, prot, type);
#endif
  /* Wait for spin lock */
  slwait (&g_sl);
  /* First time initialization */
  if (! g_pagesize) 
    g_pagesize = getpagesize ();
  if (! g_regionsize) 
    g_regionsize = getregionsize ();
  /* Assert preconditions */
  assert ((unsigned) ptr % g_pagesize == 0);
  assert (size % g_pagesize == 0);
  /* Allocate this */
  if(!(type & MAP_NORESERVE)) alloc|=MEM_COMMIT;
  if((prot & (PROT_READ|PROT_WRITE))==(PROT_READ|PROT_WRITE)) ntprot|=PAGE_READWRITE;
  else if(prot & PROT_READ) ntprot|=PAGE_READONLY;
  else if(prot & PROT_WRITE) ntprot|=PAGE_READWRITE;
  else
    {
      ntprot|=PAGE_NOACCESS;
      if(size==HEAP_MAX_SIZE)
        {
          rounding=size;
          size<<=1;
#ifdef TRACE
          printf("Rounding to multiple of %d\n", rounding);
#endif
        }
      if(ptr)
        {       /* prot==PROT_NONE also appears to be a euphemism for free */
          MEMORY_BASIC_INFORMATION mbi;
          DWORD read=0;
          char *p;
          for(p=((char *)ptr)+read; read<size && VirtualQuery(p, &mbi, sizeof(mbi)); read+=mbi.RegionSize)
            {
              if(mbi.State & MEM_COMMIT)
                {
                  if(!VirtualFree((LPVOID) p, mbi.RegionSize, MEM_DECOMMIT))
                    goto mmap_exit;
#ifdef TRACE
                  printf ("Release %p %d\n", p, mbi.RegionSize);
#endif
                }
            }
          ptr=0; /* success */
          goto mmap_exit;
        }
    }
  ptr = VirtualAlloc (ptr, size, alloc, ntprot);
  if (! ptr) {
    ptr = (void *) MORECORE_FAILURE;
    goto mmap_exit;
  }
  if(rounding)
    {
      VirtualFree(ptr, 0, MEM_RELEASE);
      ptr=(void *)(((unsigned long)ptr + (rounding-1)) & ~(rounding-1));
      if(!(ptr=VirtualAlloc(ptr, rounding, alloc, ntprot)))
        {
          ptr = (void *) MORECORE_FAILURE;
          goto mmap_exit;
        }
      assert ((unsigned) ptr % rounding == 0);
      size=rounding;
    }
  else
    {
      /* Assert postconditions */
      assert ((unsigned) ptr % g_regionsize == 0);
    }
#ifdef TRACE
  printf ("%s %p %d %d %d\n", (type & MAP_NORESERVE) ? "Reserve" : "Commit", ptr, size, prot, type);
#endif
 mmap_exit:
    /* Release spin lock */
  slrelease (&g_sl);
  return ptr;
}

/* munmap for windows */
static long munmap (void *ptr, long size) {
  static long g_pagesize;
  int rc = MUNMAP_FAILURE;
#ifdef TRACE
  printf ("munmap %p %d\n", ptr, size);
#endif
  /* Wait for spin lock */
  /* slwait (&g_sl); */
  /* First time initialization */
  if (! g_pagesize) 
    g_pagesize = getpagesize ();
  /* Assert preconditions */
  assert (size % g_pagesize == 0);
  /* Free this */
  if (! VirtualFree (ptr, 0, 
                     MEM_RELEASE))
    goto munmap_exit;
  rc = 0;
#ifdef TRACE
  printf ("Release %p %d\n", ptr, size);
#endif
 munmap_exit:
  /* Release spin lock */
  /* slrelease (&g_sl); */
  return rc;
}

static int mprotect(const void *addr, long len, int prot)
{
  static long g_pagesize;
  static long g_regionsize;
  DWORD ntprot=0, oldntprot=0;
  int rc = -1;
#ifdef TRACE
  printf ("mprotect %p %d %d\n", addr, len, prot);
#endif
  /* Wait for spin lock */
  /* slwait (&g_sl); */
  /* First time initialization */
  if (! g_pagesize) 
    g_pagesize = getpagesize ();
  if (! g_regionsize) 
    g_regionsize = getregionsize ();
  /* Assert preconditions */
  assert ((unsigned) addr % g_pagesize == 0);
  assert (len% g_pagesize == 0);
  
  if((prot & (PROT_READ|PROT_WRITE))==(PROT_READ|PROT_WRITE)) ntprot|=PAGE_READWRITE;
  else if(prot & PROT_READ) ntprot|=PAGE_READONLY;
  else if(prot & PROT_WRITE) ntprot|=PAGE_READWRITE;
  else ntprot|=PAGE_NOACCESS;
  if(prot)
    {   /* Do we need to commit any? */
      MEMORY_BASIC_INFORMATION mbi;
      DWORD read=0;
      for(; read<len && VirtualQuery(((char *)(addr))+read, &mbi, sizeof(mbi)); read+=mbi.RegionSize)
        {
          if(!(mbi.State & MEM_COMMIT))
            {   /* Might as well do the lot */
              if(!VirtualAlloc((LPVOID) addr, len, MEM_COMMIT, ntprot))
                goto mprotect_exit;
#ifdef TRACE
              printf ("Commit (mprotect) %p %d\n", addr, len);
#endif
              break;
            }
        }
    }
  else
    {   /* prot==PROT_NONE also appears to be a euphemism for free */
      MEMORY_BASIC_INFORMATION mbi;
      DWORD read=0;
      char *p;
      for(p=((char *)addr)+read; read<len && VirtualQuery(p, &mbi, sizeof(mbi)); read+=mbi.RegionSize)
        {
          if(mbi.State & MEM_COMMIT)
            {
              if(!VirtualFree((LPVOID) p, mbi.RegionSize, MEM_DECOMMIT))
                goto mprotect_exit;
#ifdef TRACE
              printf ("Release (mprotect) %p %d\n", p, mbi.RegionSize);
#endif
            }
        }
    }
  /* Change */
  if (! VirtualProtect ((LPVOID) addr, len, ntprot, &oldntprot))
    goto mprotect_exit;
  rc = 0;
#ifdef TRACE
  printf ("Protect %p %d %d\n", addr, len, prot);
#endif
 mprotect_exit:
  /* Release spin lock */
  /* slrelease (&g_sl); */
  return rc;
}

static void vminfo (CHUNK_SIZE_T  *free, CHUNK_SIZE_T  *reserved, CHUNK_SIZE_T  *committed) {
  MEMORY_BASIC_INFORMATION memory_info;
  memory_info.BaseAddress = 0;
  *free = *reserved = *committed = 0;
  while (VirtualQuery (memory_info.BaseAddress, &memory_info, sizeof (memory_info))) {
    switch (memory_info.State) {
    case MEM_FREE:
      *free += memory_info.RegionSize;
      break;
    case MEM_RESERVE:
      *reserved += memory_info.RegionSize;
      break;
    case MEM_COMMIT:
      *committed += memory_info.RegionSize;
      break;
    }
    memory_info.BaseAddress = (char *) memory_info.BaseAddress + memory_info.RegionSize;
  }
}

static int cpuinfo (int whole, CHUNK_SIZE_T  *kernel, CHUNK_SIZE_T  *user) {
    if (whole) {
        __int64 creation64, exit64, kernel64, user64;
        int rc = GetProcessTimes (GetCurrentProcess (), 
                                  (FILETIME *) &creation64,  
                                  (FILETIME *) &exit64, 
                                  (FILETIME *) &kernel64, 
                                  (FILETIME *) &user64);
        if (! rc) {
            *kernel = 0;
            *user = 0;
            return FALSE;
        } 
        *kernel = (CHUNK_SIZE_T) (kernel64 / 10000);
        *user = (CHUNK_SIZE_T) (user64 / 10000);
        return TRUE;
    } else {
        __int64 creation64, exit64, kernel64, user64;
        int rc = GetThreadTimes (GetCurrentThread (), 
                                 (FILETIME *) &creation64,  
                                 (FILETIME *) &exit64, 
                                 (FILETIME *) &kernel64, 
                                 (FILETIME *) &user64);
        if (! rc) {
            *kernel = 0;
            *user = 0;
            return FALSE;
        } 
        *kernel = (CHUNK_SIZE_T) (kernel64 / 10000);
        *user = (CHUNK_SIZE_T) (user64 / 10000);
        return TRUE;
    }
}

#endif /* WIN32 */

/* ------------------------------------------------------------
History:
    Hacked 19th Jan 2004 Niall Douglas (s_sourceforge <at symbol> nedprod.com)
          * Merged Wolfram Gloger's SMP changes in with Doug Lea's v2.7.2 version
          * Copy & pasted Gloger's external C file includes directly into this file
          * Made various adjustments to let this compile as C++ & PTMALLOC_IN_CPPNAMESPACE
          * Merged back in Win32 support, added MORECORE_IS_MMAP
          * Added Win32 emulation of mprotect() plus made mmap() set page protection
          correctly. No longer always commits arenas, now reserves and commits on demand
          * Made all Win32 emulations decommit memory with PROT_NONE
          * Added MSVC assembler op for largebin_index()

    V2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)
      * Fix malloc_state bitmap array misdeclaration

    V2.7.1 Thu Jul 25 10:58:03 2002  Doug Lea  (dl at gee)
      * Allow tuning of FIRST_SORTED_BIN_SIZE
      * Use PTR_UINT as type for all ptr->int casts. Thanks to John Belmonte.
      * Better detection and support for non-contiguousness of MORECORE. 
        Thanks to Andreas Mueller, Conal Walsh, and Wolfram Gloger
      * Bypass most of malloc if no frees. Thanks To Emery Berger.
      * Fix freeing of old top non-contiguous chunk im sysmalloc.
      * Raised default trim and map thresholds to 256K.
      * Fix mmap-related #defines. Thanks to Lubos Lunak.
      * Fix copy macros; added LACKS_FCNTL_H. Thanks to Neal Walfield.
      * Branch-free bin calculation
      * Default trim and mmap thresholds now 256K.

    V2.7.0 Sun Mar 11 14:14:06 2001  Doug Lea  (dl at gee)
      * Introduce independent_comalloc and independent_calloc.
        Thanks to Michael Pachos for motivation and help.
      * Make optional .h file available
      * Allow > 2GB requests on 32bit systems.
      * new WIN32 sbrk, mmap, munmap, lock code from <Walter@GeNeSys-e.de>.
        Thanks also to Andreas Mueller <a.mueller at paradatec.de>,
        and Anonymous.
      * Allow override of MALLOC_ALIGNMENT (Thanks to Ruud Waij for 
        helping test this.)
      * memalign: check alignment arg
      * realloc: don't try to shift chunks backwards, since this
        leads to  more fragmentation in some programs and doesn't
        seem to help in any others.
      * Collect all cases in malloc requiring system memory into sYSMALLOc
      * Use mmap as backup to sbrk
      * Place all internal state in malloc_state
      * Introduce fastbins (although similar to 2.5.1)
      * Many minor tunings and cosmetic improvements
      * Introduce USE_PUBLIC_MALLOC_WRAPPERS, USE_MALLOC_LOCK 
      * Introduce MALLOC_FAILURE_ACTION, MORECORE_CONTIGUOUS
        Thanks to Tony E. Bennett <tbennett@nvidia.com> and others.
      * Include errno.h to support default failure action.

    V2.6.6 Sun Dec  5 07:42:19 1999  Doug Lea  (dl at gee)
      * return null for negative arguments
      * Added Several WIN32 cleanups from Martin C. Fong <mcfong at yahoo.com>
         * Add 'LACKS_SYS_PARAM_H' for those systems without 'sys/param.h'
          (e.g. WIN32 platforms)
         * Cleanup header file inclusion for WIN32 platforms
         * Cleanup code to avoid Microsoft Visual C++ compiler complaints
         * Add 'USE_DL_PREFIX' to quickly allow co-existence with existing
           memory allocation routines
         * Set 'malloc_getpagesize' for WIN32 platforms (needs more work)
         * Use 'assert' rather than 'ASSERT' in WIN32 code to conform to
           usage of 'assert' in non-WIN32 code
         * Improve WIN32 'sbrk()' emulation's 'findRegion()' routine to
           avoid infinite loop
      * Always call 'fREe()' rather than 'free()'

    V2.6.5 Wed Jun 17 15:57:31 1998  Doug Lea  (dl at gee)
      * Fixed ordering problem with boundary-stamping

    V2.6.3 Sun May 19 08:17:58 1996  Doug Lea  (dl at gee)
      * Added pvalloc, as recommended by H.J. Liu
      * Added 64bit pointer support mainly from Wolfram Gloger
      * Added anonymously donated WIN32 sbrk emulation
      * Malloc, calloc, getpagesize: add optimizations from Raymond Nijssen
      * malloc_extend_top: fix mask error that caused wastage after
        foreign sbrks
      * Add linux mremap support code from HJ Liu

    V2.6.2 Tue Dec  5 06:52:55 1995  Doug Lea  (dl at gee)
      * Integrated most documentation with the code.
      * Add support for mmap, with help from
        Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Use last_remainder in more cases.
      * Pack bins using idea from  colin@nyx10.cs.du.edu
      * Use ordered bins instead of best-fit threshhold
      * Eliminate block-local decls to simplify tracing and debugging.
      * Support another case of realloc via move into top
      * Fix error occuring when initial sbrk_base not word-aligned.
      * Rely on page size for units instead of SBRK_UNIT to
        avoid surprises about sbrk alignment conventions.
      * Add mallinfo, mallopt. Thanks to Raymond Nijssen
        (raymond@es.ele.tue.nl) for the suggestion.
      * Add `pad' argument to malloc_trim and top_pad mallopt parameter.
      * More precautions for cases where other routines call sbrk,
        courtesy of Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Added macros etc., allowing use in linux libc from
        H.J. Lu (hjl@gnu.ai.mit.edu)
      * Inverted this history list

    V2.6.1 Sat Dec  2 14:10:57 1995  Doug Lea  (dl at gee)
      * Re-tuned and fixed to behave more nicely with V2.6.0 changes.
      * Removed all preallocation code since under current scheme
        the work required to undo bad preallocations exceeds
        the work saved in good cases for most test programs.
      * No longer use return list or unconsolidated bins since
        no scheme using them consistently outperforms those that don't
        given above changes.
      * Use best fit for very large chunks to prevent some worst-cases.
      * Added some support for debugging

    V2.6.0 Sat Nov  4 07:05:23 1995  Doug Lea  (dl at gee)
      * Removed footers when chunks are in use. Thanks to
        Paul Wilson (wilson@cs.texas.edu) for the suggestion.

    V2.5.4 Wed Nov  1 07:54:51 1995  Doug Lea  (dl at gee)
      * Added malloc_trim, with help from Wolfram Gloger
        (wmglo@Dent.MED.Uni-Muenchen.DE).

    V2.5.3 Tue Apr 26 10:16:01 1994  Doug Lea  (dl at g)

    V2.5.2 Tue Apr  5 16:20:40 1994  Doug Lea  (dl at g)
      * realloc: try to expand in both directions
      * malloc: swap order of clean-bin strategy;
      * realloc: only conditionally expand backwards
      * Try not to scavenge used bins
      * Use bin counts as a guide to preallocation
      * Occasionally bin return list chunks in first scan
      * Add a few optimizations from colin@nyx10.cs.du.edu

    V2.5.1 Sat Aug 14 15:40:43 1993  Doug Lea  (dl at g)
      * faster bin computation & slightly different binning
      * merged all consolidations to one part of malloc proper
         (eliminating old malloc_find_space & malloc_clean_bin)
      * Scan 2 returns chunks (not just 1)
      * Propagate failure in realloc if malloc returns 0
      * Add stuff to allow compilation on non-ANSI compilers
          from kpv@research.att.com

    V2.5 Sat Aug  7 07:41:59 1993  Doug Lea  (dl at g.oswego.edu)
      * removed potential for odd address access in prev_chunk
      * removed dependency on getpagesize.h
      * misc cosmetics and a bit more internal documentation
      * anticosmetics: mangled names in macros to evade debugger strangeness
      * tested on sparc, hp-700, dec-mips, rs6000
          with gcc & native cc (hp, dec only) allowing
          Detlefs & Zorn comparison study (in SIGPLAN Notices.)

    Trial version Fri Aug 28 13:14:29 1992  Doug Lea  (dl at g.oswego.edu)
      * Based loosely on libg++-1.2X malloc. (It retains some of the overall
         structure of old version,  but most details differ.)

*/

#endif
