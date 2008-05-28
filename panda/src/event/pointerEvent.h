// Filename: pointerEvent.h
// Created by: jyelon (20Sep2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef POINTEREVENT_H
#define POINTEREVENT_H

#include "pandabase.h"
#include "mouseData.h"

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : PointerEvent
// Description : Records a pointer movement event.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT PointerEvent {
public:

  INLINE PointerEvent();
  INLINE PointerEvent(const PointerEvent &copy);
  INLINE void operator = (const PointerEvent &copy);

  INLINE bool operator == (const PointerEvent &other) const;
  INLINE bool operator != (const PointerEvent &other) const;
  INLINE bool operator < (const PointerEvent &other) const;

  void output(ostream &out) const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

public:
  bool      _in_window;
  int       _xpos;
  int       _ypos;
  int       _dx;
  int       _dy;
  double    _length;
  double    _direction;
  double    _rotation;
  int       _sequence;
  double    _time;
};

INLINE ostream &operator << (ostream &out, const PointerEvent &pe) {
  pe.output(out);
  return out;
}

#include "pointerEvent.I"

#endif

