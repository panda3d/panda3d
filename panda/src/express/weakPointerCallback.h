// Filename: weakPointerCallback.h
// Created by:  drose (06Oct06)
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

#ifndef WEAKPOINTERCALLBACK_H
#define WEAKPOINTERCALLBACK_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakPointerCallback
// Description : Derive from this class and override the callback()
//               method if you want to get an immediate callback from
//               a WeakPointerTo object when its referenced pointer is
//               deleted.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS WeakPointerCallback {
public:
  virtual void wp_callback(void *pointer)=0;
};

#include "weakPointerCallback.I"

#endif
