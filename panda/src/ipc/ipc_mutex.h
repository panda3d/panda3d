// Filename: ipc_mutex.h
// Created by:  cary (16Sep98)
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
