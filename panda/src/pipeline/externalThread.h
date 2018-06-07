/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file externalThread.h
 * @author drose
 * @date 2006-01-30
 */

#ifndef EXTERNALTHREAD_H
#define EXTERNALTHREAD_H

#include "pandabase.h"
#include "thread.h"

/**
 * The special "external thread" class.  There is one instance of these in the
 * world, and it is returned by Thread::get_external_thread().
 */
class EXPCL_PANDA_PIPELINE ExternalThread : public Thread {
private:
  ExternalThread();
  ExternalThread(const std::string &name, const std::string &sync_name);
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
