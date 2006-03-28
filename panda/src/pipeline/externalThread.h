// Filename: externalThread.h
// Created by:  drose (30Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EXTERNALTHREAD_H
#define EXTERNALTHREAD_H

#include "pandabase.h"
#include "thread.h"

////////////////////////////////////////////////////////////////////
//       Class : ExternalThread
// Description : The special "external thread" class.  There is one
//               instance of these in the world, and it is returned by
//               Thread::get_external_thread().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ExternalThread : public Thread {
private:
  ExternalThread();
  ExternalThread(const string &name, const string &sync_name);
  virtual void thread_main();

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Thread::init_type();
    register_type(_type_handle, "ExternalThread",
                  Thread::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Thread;
};

#endif
