// Filename: buttonEventDataTransition.h
// Created by:  drose (08Feb99)
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

#ifndef BUTTONEVENTDATATRANSITION_H
#define BUTTONEVENTDATATRANSITION_H

#include <pandabase.h>

#include <nodeTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : ButtonEventDataTransition
// Description : A ButtonEventDataAttribute is a collection of buttons
//               that have been pressed recently, in the order they
//               were pressed.  The ButtonEventDataTransition does
//               nothing, since there's no sensible transformation to
//               apply to this collection of buttons.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEventDataTransition : public NodeTransition {
public:
  INLINE ButtonEventDataTransition();
  INLINE ButtonEventDataTransition(const ButtonEventDataTransition &copy);
  INLINE void operator = (const ButtonEventDataTransition &copy);

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle, "ButtonEventDataTransition",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "buttonEventDataTransition.I"

#endif
