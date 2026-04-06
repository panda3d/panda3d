/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcherGroup.h
 * @author drose
 * @date 2001-07-02
 */

#ifndef MOUSEWATCHERGROUP_H
#define MOUSEWATCHERGROUP_H

#include "pandabase.h"
#include "mouseWatcherBase.h"
#include "referenceCount.h"

/**
 * This represents a collection of MouseWatcherRegions that may be managed as
 * a group.  The implementation for this is in MouseWatcherBase; this class
 * exists so that we can inherit from ReferenceCount.
 */
class EXPCL_PANDA_TFORM MouseWatcherGroup : public MouseWatcherBase,
                                            public ReferenceCount {
public:
  INLINE MouseWatcherGroup() {};

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    MouseWatcherBase::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "MouseWatcherGroup",
                  MouseWatcherBase::get_class_type(),
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
