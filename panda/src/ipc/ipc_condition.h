// Filename: ipc_condition.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_CONDITION_H__
#define __IPC_CONDITION_H__

#include <pandabase.h>

#include "ipc_traits.h"
#include "ipc_mutex.h"

class EXPCL_PANDAEXPRESS base_condition_variable {
   public:
      typedef ipc_traits traits;
      typedef traits::condition_class condition_class;
      typedef base_mutex mutex;

      static base_condition_variable* const Null;
      INLINE base_condition_variable(mutex& m) : _mutex(m),
	_condition(traits::make_condition(m.get_mutex())) {}
      INLINE ~base_condition_variable(void) { delete _condition; }
      INLINE void wait(void) { _condition->wait(); }
      INLINE int timedwait(const unsigned long secs,
			   const unsigned long nsecs = 0) {
	 return _condition->timedwait(secs, nsecs);
      }
      INLINE void signal(void) { _condition->signal(); }
      INLINE void broadcast(void) { _condition->broadcast(); }
      INLINE condition_class* get_condition(void) { return _condition; }
   private:
      mutex& _mutex;
      condition_class *_condition;
};

typedef base_condition_variable condition_variable;

#endif /* __IPC_CONDITION_H__ */
