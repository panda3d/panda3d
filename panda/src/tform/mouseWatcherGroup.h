// Filename: mouseWatcherGroup.h
// Created by:  drose (02Jul01)
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

#ifndef MOUSEWATCHERGROUP_H
#define MOUSEWATCHERGROUP_H

#include "pandabase.h"
#include "mouseWatcherBase.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherGroup
// Description : This represents a collection of MouseWatcherRegions
//               that may be managed as a group.  The implementation
//               for this is in MouseWatcherBase; this class exists
//               so that we can inherit from ReferenceCount.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TFORM MouseWatcherGroup : public MouseWatcherBase,
                                            public ReferenceCount {
public:
  INLINE MouseWatcherGroup() {};

public:
  static TypeHandle get_class_type() {
    ReferenceCount::init_type();
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "MouseWatcherGroup",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
