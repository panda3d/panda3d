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
class EXPCL_PANDA_EVENT PointerEvent {
public:
  PointerEvent(PointerData data, double time = ClockObject::get_global_clock()->get_frame_time());

  // INLINE bool operator == (const PointerEvent &other) const;
  // INLINE bool operator != (const PointerEvent &other) const;
  // INLINE bool operator < (const PointerEvent &other) const;

  void output(std::ostream &out) const;

public:
  PointerData _data;
  double _time = 0.0;
};

INLINE std::ostream &operator << (std::ostream &out, const PointerEvent &pe) {
  pe.output(out);
  return out;
}

#include "pointerEvent.I"

#endif
