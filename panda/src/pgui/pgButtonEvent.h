// Filename: pgButtonEvent.h
// Created by:  drose (05Jul01)
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

#ifndef PGBUTTONEVENT_H
#define PGBUTTONEVENT_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : PGButtonEvent
// Description : This is sent along as a parameter to a button_down or
//               button_up event for an item to indicate which button
//               was involved, and what the current mouse position
//               was.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGButtonEvent : public TypedReferenceCount {
public:
  INLINE PGButtonEvent();
  INLINE PGButtonEvent(ButtonHandle button, float mouse_x, float mouse_y);
  virtual ~PGButtonEvent();

PUBLISHED:
  INLINE ButtonHandle get_button() const;
  INLINE string get_button_name() const;

  INLINE float get_mouse_x() const;
  INLINE float get_mouse_y() const;

  void output(ostream &out) const;

public:
  ButtonHandle _button;
  float _mouse_x;
  float _mouse_y;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PGButtonEvent",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const PGButtonEvent &event);

#include "pgButtonEvent.I"

#endif
