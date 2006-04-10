// Filename: nonDeletor.h
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

#ifndef NONDELETOR_H
#define NONDELETOR_H

#include "pandabase.h"
#include "objectDeletor.h"

////////////////////////////////////////////////////////////////////
//       Class : NonDeletor
// Description : This specialization of ObjectDeletor serves a very
//               specific function: it *doesn't* delete pointers it is
//               given.  This is useful mainly for testing, for
//               instance to determine if there is a problem with a
//               destructor somewhere.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NonDeletor : public ObjectDeletor {
public:
  virtual void delete_object(DeleteFunc *func, void *ptr);

  static void register_deletor();
};

#endif
