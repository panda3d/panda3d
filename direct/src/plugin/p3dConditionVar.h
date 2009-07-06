// Filename: p3dConditionVar.h
// Created by:  drose (02Jul09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef P3DCONDITIONVAR_H
#define P3DCONDITIONVAR_H

#include "p3d_plugin_common.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DConditionVar
// Description : A simple condition-variable like object.  It doesn't
//               support the full condition-var semantics, but it
//               works well enough with one waiter and one signaller.
////////////////////////////////////////////////////////////////////
class P3DConditionVar {
public:
  P3DConditionVar();
  ~P3DConditionVar();

  void acquire();
  void wait();
  void wait(double timeout);
  void notify();
  void release();
  
private:
#ifdef _WIN32
  CRITICAL_SECTION _lock;
  HANDLE _event_signal;

#else  // _WIN32
  pthread_mutex_t _lock;
  pthread_cond_t _cvar;
#endif  // _WIN32
};

#include "p3dConditionVar.I"

#endif
