// Filename: objectDeletor.h
// Created by:  drose (10Apr06)
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

#ifndef OBJECTDELETOR_H
#define OBJECTDELETOR_H

#include "pandabase.h"
#include "atomicAdjust.h"

////////////////////////////////////////////////////////////////////
//       Class : ObjectDeletor
// Description : This class is used to collect together pointers to
//               objects that are ready to be destructed and freed.
//               The actual destruction may be performed immediately,
//               or it can be performed at some later time, when it is
//               convenient for the application.
//
//               This is particularly useful for a multithreaded
//               application; the destruction may be performed in a
//               sub-thread with a lower priority.
//
//               This class body is just an interface; the
//               ObjectDeletor class simply deletes its pointers
//               immediately.  More sophisticated deletors are
//               implemented elsewhere.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ObjectDeletor {
public:
  virtual ~ObjectDeletor();

  typedef void DeleteFunc(void *ptr);

  virtual void delete_object(DeleteFunc *func, void *ptr);
  virtual void flush();

  INLINE static ObjectDeletor *get_global_ptr();
  INLINE static ObjectDeletor *set_global_ptr(ObjectDeletor *ptr);

  static void register_subclass(ObjectDeletor *deletor, const string &name);

protected:
  class DeleteToken {
  public:
    INLINE DeleteToken(DeleteFunc *func, void *ptr);
    INLINE void do_delete();

    DeleteFunc *_func;
    void *_ptr;
  };

private:
  static void *_global_ptr;
};

#include "objectDeletor.I"

#endif
