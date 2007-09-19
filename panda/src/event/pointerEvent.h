// Filename: pointerEvent.h
// Created by: jyelon (20Sep2007)
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
  INLINE PointerEvent(int pointer, const MouseData &data, double time);
  INLINE void operator = (const PointerEvent &copy);

  INLINE bool operator == (const PointerEvent &other) const;
  INLINE bool operator != (const PointerEvent &other) const;
  INLINE bool operator < (const PointerEvent &other) const;

  void output(ostream &out) const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

public:
  int       _pointer;
  MouseData _data;
  double    _time;
};

INLINE ostream &operator << (ostream &out, const PointerEvent &pe) {
  pe.output(out);
  return out;
}

#include "pointerEvent.I"

#endif

