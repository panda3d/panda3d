// Filename: asyncUtility.h
// Created by:  mike (09Jan97)
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
#ifndef ASYNCUTILITY_H
#define ASYNCUTILITY_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"
#include "notify.h"
#include <typedef.h>

#ifdef OLD_HAVE_IPC
#include <ipc_mutex.h>
#include <ipc_condition.h>
#include <ipc_thread.h>
#endif


////////////////////////////////////////////////////////////////////
//       Class : AsyncUtility
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS AsyncUtility {
PUBLISHED:
  INLINE void set_frequency(float frequency);
  INLINE float get_frequency(void) const;

  void create_thread(void);

public:
  AsyncUtility(float frequency = 0.2);
  virtual ~AsyncUtility(void);

protected:
  void destroy_thread(void);
  static void* st_callback(void *arg);
  void callback(void);
  virtual bool process_request(void) = 0;
  void nap(void) const;

protected:
  int _next_token;
  bool _shutdown;
  bool _threaded;
  float _frequency;
  bool _threads_enabled;

#ifdef OLD_HAVE_IPC
  mutex _lock;
  condition_variable *_request_cond;
  thread *_thread;
#endif
};

#include "asyncUtility.I"

#endif
