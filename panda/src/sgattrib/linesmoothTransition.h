// Filename: linesmoothTransition.h
// Created by:  mike (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef LINESMOOTHTRANSITION_H
#define LINESMOOTHTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : LinesmoothTransition
// Description : This enables or disables the antialiasing of lines.
//               It has no additional properties; it's simply an
//               on-or-off thing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LinesmoothTransition : public OnOffTransition {
public:
  INLINE LinesmoothTransition();
  INLINE static LinesmoothTransition off();

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffTransition::init_type();
    register_type(_type_handle, "LinesmoothTransition",
                  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "linesmoothTransition.I"

#endif
