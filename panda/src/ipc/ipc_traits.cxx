// Filename: ipc_traits.cxx
// Created by:  frang (06Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "ipc_traits.h"

Configure(ipc_trait);

#if (__IPC_FLAVOR_DUJOUR_TOKEN__ == 0)

// mach

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class*)0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class*)0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class*)0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class*)0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class*)0L;

int ipc_traits::normal_priority;
int ipc_traits::highest_priority;

ConfigureFn(ipc_trait) {
  // find base and max priority.  This is the inital thread, so the max
  // priority of this thread also applies to any newly created thread.
  kern_return_t error;
  struct thread_sched_info info;
  unsigned int info_count = THREAD_SCHED_INFO_COUNT;

  error = thread_info(thread_self(), THREAD_SCHED_INFO, (thread_info_t)&info,
                      &info_count);
  if (error != KERN_SUCCESS) {
    FATAL() << "ipc_traits(mach) initialization: error determining thread_info"
            << nend;
    ::exit(1);
  } else {
    ipc_traits::__set_normal_priority(info.base_priority);
    ipc_traits::__set_highest_priority(info.max_priority);
  }
}

void* ipc_traits::thread_class::thread_wrapper(void* data) {
   ipc_traits::thread_class* me = (ipc_traits::thread_class*)data;
   return (*me->_fn)(me->_b_ptr);
}

#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 1)

// NT

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class*)0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class*)0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class*)0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class*)0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class*)0L;

DWORD ipc_traits::self_tls_index;
ipc_traits::condition_class::_internal_thread_dummy*
  ipc_traits::condition_class::_internal_thread_helper::cache =
    (ipc_traits::condition_class::_internal_thread_dummy*)0L;
ipc_traits::mutex_class*
  ipc_traits::condition_class::_internal_thread_helper::cachelock =
    (ipc_traits::mutex_class*)0L;

ConfigureFn(ipc_trait) {
  DWORD tmp = TlsAlloc();
  if (tmp == 0xffffffff)
    throw ipc_fatal(GetLastError());
  ipc_traigs::__set_self_tls_index(tmp);
}

#ifndef __BCPLUSPLUS__
unsigned _stdcall ipc_traits::thread_class::thread_wrapper(void* data) {
#else /* __BCPLUSPLUS__ */
void _USERENTRY ipc_traits::thread_class::thread_wrapper(void* data) {
#endif /* __BCPLUSPLUS__ */
  ipc_traits::thread_class* me = (ipc_traits::thread_class*)data;
  me->_return_value = (*me->_fn)(me->_b_ptr);
#ifndef __BCPLUSPLUS__
  return 0;
#endif /* __BCPLUSPLUS__ */
}

#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 2)

// posix

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class*)0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class*)0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class*)0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class*)0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class*)0L;

pthread_key_t ipc_traits::self_key;
#ifdef PthreadSupportThreadPriority
int ipc_traits::lowest_priority;
int ipc_traits::normal_priority;
int ipc_traits::highest_priority;
#endif /* PthreadSupportThreadPriority */

ConfigureFn(ipc_trait) {
#ifdef NeedPthreadInit
  pthread_init();
#endif /* NeedPthreadInit */

  pthread_key_t ktmp;
#if (PthreadDraftVersion == 4)
  THROW_ERRORS_G(pthread_keycreate(&ktmp, NULL));
#else /* PthreadDraftVersion 4 */
  THROW_ERRORS_G(pthread_key_create(&ktmp, NULL));
#endif /* PthreadDraftVersion 4 */
  ipc_traits::__set_self_key(ktmp);

#ifdef PthreadSupportThreadPriority
  int plow, pnorm, phigh;
#if defined(__osf1__) && defined(__alpha__) || defined(__VMS)
  plow = PRI_OTHER_MIN;
  phigh = PRI_OTHER_MAX;
#elif defined(__hpux__)
  plow = PRI_OTHER_MIN;
  phigh = PRI_OTHER_MAX;
#elif defined(__sunos__) && (__OSVERSION__ == 5)
  // a bug in pthread_attr_setschedparam means lowest priority is 1 not 0
  plow = 1;
  phigh = 3;
#else /* os testing */
  plow = sched_get_priority_min(SCHED_FIFO);
  phigh = sched_get_priority_max(SCHED_FIFO);
#endif /* os testing */
  switch (phigh - plow) {
  case 0:
  case 1:
    pnorm = plow;
    break;
  default:
    pnorm = plow + 1;
    break;
  }
  ipc_traits::__set_lowest_priority(plow);
  ipc_traits::__set_normal_priority(pnorm);
  ipc_traits::__set_highest_priority(phigh);
#endif /* PthreadSupportThreadPriority */
}

void* ipc_traits::thread_class::thread_wrapper(void* data) {
  ipc_traits::thread_class* me = (ipc_traits::thread_class*)data;
  return (*me->_fn)(me->_b_ptr);
}

#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 3)

// solaris

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class*)0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class*)0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class*)0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class*)0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class*)0L;

thread_key_t ipc_traits::self_key;

ConfigureFn(ipc_trait) {
  thread_key_t ktmp;
  THROW_ERRORS_G(thr_keycreate(&ktmp, NULL));
  ipc_traits::__set_self_key(ktmp);
}

void* ipc_traits::thread_class::thread_wrapper(void* data) {
  ipc_traits::thread_class* me = (ipc_traits::thread_class*)data;
  return (*me->_fn)(me->_b_ptr);
}

#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 4)

// NSPR

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class*)0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class*)0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class*)0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class*)0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class*)0L;

PRUintn ipc_traits::_data_index;

ConfigureFn(ipc_trait) {
  PRUintn tmp;
  PR_NewThreadPrivateIndex(&tmp, (PRThreadPrivateDTOR)0L);
  ipc_traits::__set_data_index(tmp);
}

void ipc_traits::thread_class::thread_wrapper(void* data) {
  ipc_traits::thread_class* me = (ipc_traits::thread_class*)data;
  me->_return_value = (*me->_fn)(me->_b_ptr);
}

#elif (__IPC_FLAVOR_DUJOUR_TOKEN__ == 5)

// PS2

#include <eekernel.h>
#include "pmap.h"

ConfigureFn(ipc_trait) {
  ipc_traits::thread_class::BuildRootThread();
}

pvector<ipc_traits::thread_class::pointer_lookup> ipc_traits::thread_class::thread_addr_vector;
ipc_traits::thread_class *ipc_traits::thread_class::m_root_thread;

ipc_traits::mutex_class* const ipc_traits::mutex_class::Null =
  (ipc_traits::mutex_class *) 0L;
ipc_traits::condition_class* const ipc_traits::condition_class::Null =
  (ipc_traits::condition_class *) 0L;
ipc_traits::semaphore_class* const ipc_traits::semaphore_class::Null =
  (ipc_traits::semaphore_class *) 0L;
ipc_traits::thread_class* const ipc_traits::thread_class::Null =
  (ipc_traits::thread_class *) 0L;
ipc_traits::library_class* const ipc_traits::library_class::Null =
  (ipc_traits::library_class *) 0L;

void ipc_traits::thread_class::thread_wrapper(void *data) {
  ipc_traits::thread_class *me = (ipc_traits::thread_class *) data;
  (*me->_fn)(me->_b_ptr);
}

#else /* out of flavor tokens */
#error "invalid ipc flavor token"
#endif /* out of flavor tokens */


