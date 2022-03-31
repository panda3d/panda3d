/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcherRegion.h
 * @author drose
 * @date 2000-07-13
 */

#ifndef MOUSEWATCHERREGION_H
#define MOUSEWATCHERREGION_H

#include "pandabase.h"

#include "namable.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "buttonHandle.h"
#include "modifierButtons.h"

class MouseWatcherParameter;

/**
 * This is the class that defines a rectangular region on the screen for the
 * MouseWatcher.
 */
class EXPCL_PANDA_TFORM MouseWatcherRegion : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  INLINE explicit MouseWatcherRegion(const std::string &name, PN_stdfloat left, PN_stdfloat right,
                                     PN_stdfloat bottom, PN_stdfloat top);
  INLINE explicit MouseWatcherRegion(const std::string &name, const LVecBase4 &frame);

  INLINE void set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_frame(const LVecBase4 &frame);
  INLINE const LVecBase4 &get_frame() const;
  INLINE PN_stdfloat get_area() const;

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

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

PUBLISHED:
  MAKE_PROPERTY(frame, get_frame, set_frame);
  MAKE_PROPERTY(area, get_area);

  MAKE_PROPERTY(sort, get_sort, set_sort);
  MAKE_PROPERTY(active, get_active, set_active);
  MAKE_PROPERTY(keyboard, get_keyboard, set_keyboard);
  MAKE_PROPERTY(suppress_flags, get_suppress_flags, set_suppress_flags);

public:
  INLINE bool operator < (const MouseWatcherRegion &other) const;

  virtual void enter_region(const MouseWatcherParameter &param);
  virtual void exit_region(const MouseWatcherParameter &param);
  virtual void within_region(const MouseWatcherParameter &param);
  virtual void without_region(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);
  virtual void keystroke(const MouseWatcherParameter &param);
  virtual void candidate(const MouseWatcherParameter &param);
  virtual void move(const MouseWatcherParameter &param);

private:
  LVecBase4 _frame;
  PN_stdfloat _area;
  int _sort;

  enum Flags {
    // F_suppress_flags is the union of all of the SuppressFlags, above.
    // Presently, we reserve 8 bits for suppress flags.
    F_suppress_flags = 0x0ff,
    F_active         = 0x100,
    F_keyboard       = 0x200,
  };
  int _flags;

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

INLINE std::ostream &operator << (std::ostream &out, const MouseWatcherRegion &region) {
  region.output(out);
  return out;
}

#include "mouseWatcherRegion.I"

#endif
