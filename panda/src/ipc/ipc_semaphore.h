// Filename: ipc_semaphore.h
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

#ifndef __IPC_SEMAPHORE_H__
#define __IPC_SEMAPHORE_H__

#include <pandabase.h>

#include "ipc_traits.h"

class EXPCL_PANDAEXPRESS base_semaphore {
   public:
      typedef ipc_traits traits;
      typedef ipc_traits::semaphore_class semaphore_class;

      static base_semaphore* const Null;
      INLINE base_semaphore(const unsigned int initial = 1) :
        _semaphore(traits::make_semaphore(initial)) {}
      INLINE ~base_semaphore(void) { delete _semaphore; }
      INLINE void wait(void) { _semaphore->wait(); }
      INLINE int trywait(void) { return _semaphore->trywait(); }
      INLINE void post(void) { _semaphore->post(); }
      INLINE semaphore_class* get_semaphore(void) { return _semaphore; }
   private:
      semaphore_class *_semaphore;
};

class base_semaphore_lock {
   public:
      typedef base_semaphore semaphore;
      INLINE base_semaphore_lock(semaphore& s) : _semaphore(s) {
         _semaphore.wait();
      }
      INLINE ~base_semaphore_lock(void) { _semaphore.post(); }
   private:
      semaphore& _semaphore;
};

typedef base_semaphore semaphore;
typedef base_semaphore_lock semaphore_lock;

#endif /* __IPC_SEMAPHORE_H__ */
