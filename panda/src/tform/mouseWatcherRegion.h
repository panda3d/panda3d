// Filename: mouseWatcherRegion.h
// Created by:  drose (13Jul00)
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

#ifndef MOUSEWATCHERREGION_H
#define MOUSEWATCHERREGION_H

#include "pandabase.h"

#include "namable.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "buttonHandle.h"
#include "modifierButtons.h"

class MouseWatcherParameter;

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherRegion
// Description : This is the class that defines a rectangular region
//               on the screen for the MouseWatcher.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseWatcherRegion : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  INLINE MouseWatcherRegion(const string &name, float left, float right,
                            float bottom, float top);
  INLINE MouseWatcherRegion(const string &name, const LVecBase4f &frame);

  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE const LVecBase4f &get_frame() const;
  INLINE float get_area() const;

  INLINE void set_sort(int sort);
  INLINE int get_sort() const;

  INLINE void set_active(bool active);
  INLINE bool get_active() const;

  INLINE void set_keyboard(bool keyboard);
  INLINE bool get_keyboard() const;

  enum SuppressFlags {
    SF_mouse_button       = 0x001,
    SF_other_button       = 0x002,
    SF_any_button         = 0x003,
    SF_mouse_position     = 0x004,
  };

  INLINE void set_suppress_flags(int suppress_flags);
  INLINE int get_suppress_flags() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE bool operator < (const MouseWatcherRegion &other) const;

  virtual void enter(const MouseWatcherParameter &param);
  virtual void exit(const MouseWatcherParameter &param);
  virtual void within(const MouseWatcherParameter &param);
  virtual void without(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);
  virtual void keystroke(const MouseWatcherParameter &param);

private:
  LVecBase4f _frame;
  float _area;
  int _sort;

  enum Flags {
    // F_suppress_flags is the union of all of the SuppressFlags,
    // above.  Presently, we reserve 8 bits for suppress flags.
    F_suppress_flags = 0x0ff,
    F_active         = 0x100,
    F_keyboard       = 0x200,
  };
  int _flags;
  ModifierButtons _mods;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MouseWatcherRegion",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const MouseWatcherRegion &region) {
  region.output(out);
  return out;
}

#include "mouseWatcherRegion.I"

#endif
