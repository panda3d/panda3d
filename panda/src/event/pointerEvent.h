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

class Datagram;
class DatagramIterator;

/**
 * Records a pointer movement event.
 */
class EXPCL_PANDA_EVENT PointerEvent {
public:
  PointerEvent() = default;

  INLINE bool operator == (const PointerEvent &other) const;
  INLINE bool operator != (const PointerEvent &other) const;
  INLINE bool operator < (const PointerEvent &other) const;

  void output(std::ostream &out) const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

public:
  bool _in_window = false;
  int _id = 0;
  PointerType _type = PointerType::unknown;
  double _xpos = 0.0;
  double _ypos = 0.0;
  double _dx = 0.0;
  double _dy = 0.0;
  double _length = 0.0;
  double _direction = 0.0;
  double _pressure = 0.0;
  double _rotation = 0.0;
  int _sequence = 0;
  double _time = 0.0;
};

INLINE std::ostream &operator << (std::ostream &out, const PointerEvent &pe) {
  pe.output(out);
  return out;
}

#include "pointerEvent.I"

#endif
