// Filename: loom_internal.h
// Created by:  cary (23Sep98)
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

#ifndef __LOOM_INTERNAL_H__
#define __LOOM_INTERNAL_H__

#include <pandabase.h>

#include "loom.h"
#include "ipc_mutex.h"
#include "ipc_condition.h"
#include "ipc_thread.h"
#include "plist.h"

class app_service;

typedef std::plist<app_service*> service_list;

extern service_list *task_list;
extern mutex *main_thread_mutex;

class app_service {
   private:
      vv_func _init, _cleanup, _info;
      av_func _service;
      thread* _thread;

      INLINE app_service(void) {}
      INLINE ~app_service(void) {}
      static void DoIt(void*);
   public:
      INLINE app_service(vv_func init, av_func service, vv_func cleanup,
                         vv_func info) : _init(init), _service(service),
                         _cleanup(cleanup), _info(info), _thread(thread::Null)
         {}
      INLINE bool started() {
         return (_thread != thread::Null);
      }
      INLINE void start(void) {
         if (!this->started())
            thread::create(DoIt, (void*)this);
      }
      INLINE void info(void) {
         // this probably should mutex lock the task_list while checking things
         if (_info != (vv_func)0L)
            (*_info)();
         if (_thread != thread::Null) {
            cerr << "thread is currently running.  Id = " << _thread->get_id()
                 << ".  Current state = ";
            switch (_thread->get_state()) {
            case thread::STATE_NEW:
               cerr << "New.";
               break;
            case thread::STATE_RUNNING:
               cerr << "Running.";
               break;
            case thread::STATE_TERMINATED:
               cerr << "Terminated";
               break;
            }
            cerr << endl;
         } else
            cerr << "thread not started or created yet." << endl;
      }
};

extern main_thread_message* message_to_main_thread;
extern mutex* message_mutex;
extern condition_variable* main_thread_full;
extern condition_variable* main_thread_empty;
extern bool main_thread_empty_flag;

#endif /* __LOOM_INTERNAL_H__ */
