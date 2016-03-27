/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherGroup.h
 * @author drose
 * @date 2001-07-09
 */

#ifndef PGMOUSEWATCHERGROUP_H
#define PGMOUSEWATCHERGROUP_H

#include "pandabase.h"

#include "mouseWatcherGroup.h"
#include "pointerTo.h"

class PGTop;

/**
 * This is a specialization on MouseWatcherGroup, to associate it with a
 * PGTop.  Originally we had PGTop multiply inheriting from NamedNode and
 * MouseWatcherGroup, but this causes problems with circular reference counts.
 */
class EXPCL_PANDA_PGUI PGMouseWatcherGroup : public MouseWatcherGroup {
public:
  INLINE PGMouseWatcherGroup(PGTop *top);
  virtual ~PGMouseWatcherGroup();

  INLINE void clear_top(PGTop *top);

private:
  PGTop *_top;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseWatcherGroup::init_type();
    register_type(_type_handle, "PGMouseWatcherGroup",
                  MouseWatcherGroup::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgMouseWatcherGroup.I"

#endif
