// Filename: ipc_mutex.h
// Created by:  cary (16Sep98)
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

#ifndef __IPC_MUTEX_H__
#define __IPC_MUTEX_H__

#include <pandabase.h>

#include "ipc_traits.h"

class EXPCL_PANDAEXPRESS base_mutex {
   public:
      typedef ipc_traits traits;
      typedef traits::mutex_class mutex_class;

      static base_mutex* const Null;
      INLINE base_mutex(void) : _mutex(traits::make_mutex()) {}
      INLINE ~base_mutex(void) { delete _mutex; }
      INLINE void lock(void) { _mutex->lock(); }
      INLINE void unlock(void) { _mutex->unlock(); }
      INLINE mutex_class* get_mutex(void) { return _mutex; }
   private:
      mutex_class* _mutex;
};

class base_mutex_lock {
   public:
      typedef base_mutex mutex;
      INLINE base_mutex_lock(mutex& m) : _mutex(m) { _mutex.lock(); }
      INLINE ~base_mutex_lock(void) { _mutex.unlock(); }
   private:
      mutex& _mutex;
};

typedef base_mutex mutex;
typedef base_mutex_lock mutex_lock;

#endif /* __IPC_MUTEX_H__ */
