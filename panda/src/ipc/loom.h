// Filename: loom.h
// Created by:  cary (23Sep98)
//
////////////////////////////////////////////////////////////////////

#ifndef __LOOM_H__
#define __LOOM_H__

#include <pandabase.h>

#include "ipc_mutex.h"
#include "ipc_condition.h"

extern mutex main_thread_print_mutex;

// interface to modules for being runable from app.

enum Action { RESERVICE, DONE, YIELD, SLEEP, WAIT };

typedef void (*vv_func)(void);
typedef Action (*av_func)(unsigned long&, unsigned long&, condition_variable*&);

// parameters in order are:
//    initialization function - setup and data that the service function might
//                              need in order to run
//    service function - perform the task of this thing.  Either incrementally
//                       (thus returning RESERVICE), or all at once (handling
//                       yielding and waiting internally and returning DONE
//                       when all it's work is complete).
//    cleanup function - when the service function returns DONE, if this is not
//                       NULL, this function will be called to cleanup whatever
//                       is left to be done by this task.
//    info function - if not NULL and the main thread is asked to tell about
//                    what's running, this function will be called.  It is
//                    expected that it will output something usefull about what
//                    the task is and what it's doing.
extern void RegisterAppService(vv_func, av_func, vv_func = (vv_func)0L,
                               vv_func = (vv_func)0L);


template <class str = std::string>
class main_thread_message_base {
   public:
      enum message_t { LOAD, RESCAN, INFO };
      INLINE main_thread_message_base(const message_t message, str s = "") :
        _m(message), _lib(s) {}
      INLINE main_thread_message_base(const main_thread_message_base& m) :
        _m(m._m), _lib(m._lib) {}
      INLINE ~main_thread_message_base(void) {}
      INLINE message_t get_message(void) const { return _m; }
      INLINE str get_lib(void) const { return _lib; }
   private:
      message_t _m;
      str _lib;
};

typedef main_thread_message_base<> main_thread_message;

extern void SendMainThreadMessage(main_thread_message&);

#endif /* __LOOM_H__ */
