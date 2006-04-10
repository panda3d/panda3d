// Filename: deferredDeletor.h
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

#ifndef DEFERREDDELETOR_H
#define DEFERREDDELETOR_H

#include "pandabase.h"
#include "objectDeletor.h"
#include "pvector.h"
#include "pmutex.h"

////////////////////////////////////////////////////////////////////
//       Class : DeferredDeletor
// Description : The DeferredDeletor does all its deleting between
//               frames, where it can be observed by PStats and where
//               it won't interfere with rendering.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DeferredDeletor : public ObjectDeletor {
public:
  DeferredDeletor();

  virtual void delete_object(DeleteFunc *func, void *ptr);
  virtual void flush();

  static void register_deletor();

private:
  typedef pvector<DeleteToken> Tokens;
  Tokens _tokens;
  Mutex _lock;
};

#endif
