// Filename: buttonEvent.h
// Created by:  drose (01Mar00)
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

#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include <pandabase.h>

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonEvent
// Description : Records a transition of one button from up to down or
//               vice-versa.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEvent {
public:
  INLINE ButtonEvent();
  INLINE ButtonEvent(ButtonHandle button, bool down);
  INLINE ButtonEvent(const ButtonEvent &copy);
  INLINE void operator = (const ButtonEvent &copy);

  INLINE bool operator == (const ButtonEvent &other) const;
  INLINE bool operator != (const ButtonEvent &other) const;
  INLINE bool operator < (const ButtonEvent &other) const;

  void output(ostream &out) const;

  ButtonHandle _button;
  bool _down;
};

INLINE ostream &operator << (ostream &out, const ButtonEvent &be) {
  be.output(out);
  return out;
}

#include "buttonEvent.I"

#endif

