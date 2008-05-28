// Filename: partBundleHandle.h
// Created by:  drose (01Oct07)
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
