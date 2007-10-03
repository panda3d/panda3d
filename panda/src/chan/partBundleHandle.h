// Filename: partBundleHandle.h
// Created by:  drose (01Oct07)
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

#ifndef PARTBUNDLEHANDLE_H
#define PARTBUNDLEHANDLE_H

#include "pandabase.h"

#include "partBundle.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PartBundleHandle
// Description : This is a trivial class returned by
//               PartBundleNode::get_bundle().  Its purpose is to hold
//               the actual PartBundle pointer contained within the
//               PartBundleNode, so that scene graph flatten
//               operations can safely combine or duplicate
//               PartBundles as necessary without affecting high-level
//               bundle operations.
//
//               The high-level Actor class defined in
//               direct/src/actor, for instance, will store a list of
//               PartBundleHandles instead of on actual PartBundles,
//               so that it will be immune to changes from these
//               flatten operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAN PartBundleHandle : public ReferenceCount {
PUBLISHED:
  INLINE PartBundleHandle(PartBundle *bundle);
  INLINE ~PartBundleHandle();

  INLINE PartBundle *get_bundle();
  INLINE void set_bundle(PartBundle *bundle);

private:
  PT(PartBundle) _bundle;
};

#include "partBundleHandle.I"

#endif
