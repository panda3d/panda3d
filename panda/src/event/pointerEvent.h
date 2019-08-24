/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEvent.h
 * @author jyelon
 * @date 2007-09-20
 */

#ifndef POINTEREVENT_H
#define POINTEREVENT_H

#include "pandabase.h"
#include "pointerData.h"

/**
 * Records a pointer movement event.
 */
class EXPCL_PANDA_EVENT PointerEvent : public TypedWritableReferenceCount {
public:
  PointerEvent() = default;
  PointerEvent(PointerData data, int sequence, double time = ClockObject::get_global_clock()->get_frame_time());

  // INLINE bool operator == (const PointerEvent &other) const;
  // INLINE bool operator != (const PointerEvent &other) const;
  // INLINE bool operator < (const PointerEvent &other) const;

  void output(std::ostream &out) const;

public:
  /**
   * Captured copy of the pointer data for this event.
   */
  PointerData _data;
  
  /**
   * Time in seconds.
   */
  double _time = 0.0;

  /**
   * nth touch since the primary pointer touched the screen.
   */
  int _sequence = 0;
  
  /**
   * Change in positon since the last PointerEvent with this id. Will be (0,0)
   * if the _data.get_phase() is PointerPhase::began.
   */
  LVecBase2 _pos_change;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PointerEvent",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const PointerEvent &pe) {
  pe.output(out);
  return out;
}

#include "pointerEvent.I"

#endif
