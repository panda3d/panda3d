// Filename: pgMouseWatcherGroup.h
// Created by:  drose (09Jul01)
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

#ifndef PGMOUSEWATCHERGROUP_H
#define PGMOUSEWATCHERGROUP_H

#include "pandabase.h"

#include "mouseWatcherGroup.h"
#include "pointerTo.h"

class PGTop;

////////////////////////////////////////////////////////////////////
//       Class : PGMouseWatcherGroup
// Description : This is a specialization on MouseWatcherGroup, to
//               associate it with a PGTop.  Originally we had PGTop
//               multiply inheriting from NamedNode and
//               MouseWatcherGroup, but this causes problems with
//               circular reference counts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGMouseWatcherGroup : public MouseWatcherGroup {
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
