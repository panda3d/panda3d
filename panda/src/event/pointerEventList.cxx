// Filename: pointerEventList.cxx
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

#include "pointerEventList.h"
#include "indent.h"
#include "clockObject.h"
#include "mathNumbers.h"
#include <math.h>

TypeHandle PointerEventList::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PointerEventList::
output(ostream &out) const {
  if (_events.empty()) {
    out << "(no pointers)";
  } else {
    Events::const_iterator ei;
    ei = _events.begin();
    out << "(" << (*ei);
    ++ei;
    while (ei != _events.end()) {
      out << " " << (*ei);
      ++ei;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PointerEventList::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _events.size() << " events:\n";
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    indent(out, indent_level + 2) << (*ei) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::add_event
//       Access: Published
//  Description: Adds a new event to the end of the list.
//               Automatically calculates the dx, dy, length,
//               direction, and rotation for all but the first event.
////////////////////////////////////////////////////////////////////
void PointerEventList::
add_event(bool in_win, int xpos, int ypos, int seq, double time) {
  PointerEvent pe;
  pe._in_window = in_win;
  pe._xpos = xpos;
  pe._ypos = ypos;
  pe._sequence = seq;
  pe._time = time;
  if (_events.size() > 0) {
    pe._dx = xpos - _events.back()._xpos;
    pe._dy = ypos - _events.back()._ypos;
    double ddx = pe._dx;
    double ddy = pe._dy;
    pe._length = sqrt(ddx*ddx + ddy*ddy);
    if (pe._length > 0.0) {
      pe._direction = atan2(-ddy,ddx) * (180.0 / MathNumbers::pi);
      if (pe._direction < 0.0) pe._direction += 360.0;
    } else {
      pe._direction = _events.back()._direction;
    }
    pe._rotation = pe._direction - _events.back()._direction;
    if (pe._rotation >  180.0) pe._rotation -= 360.0;
    if (pe._rotation < -180.0) pe._rotation += 360.0;
  } else {
    pe._dx = 0;
    pe._dy = 0;
    pe._length = 0.0;
    pe._direction = 0.0;
    pe._rotation = 0.0;
  }
  _events.push_back(pe);
}       

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::encircles
//       Access: Published
//  Description: Returns true if the trail loops around the
//               specified point.
////////////////////////////////////////////////////////////////////
bool PointerEventList::
encircles(int x, int y) const {
  int tot_events = _events.size();
  if (tot_events < 3) {
    return false;
  }
  double dx = _events[0]._xpos - x;
  double dy = _events[0]._ypos - y;
  double lastang = atan2(dy, dx) * (180.0/MathNumbers::pi);
  double total = 0.0;
  for (int i=1; (i<tot_events) && (total < 360.0) && (total > -360.0); i++) {
    dx = _events[i]._xpos - x;
    dy = _events[i]._ypos - y;
    if ((dx==0.0)&&(dy==0.0)) {
      continue;
    }
    double angle = atan2(dy,dx) * (180.0/MathNumbers::pi);
    double deltang = angle - lastang;
    if (deltang < -180.0) deltang += 360.0;
    if (deltang >  180.0) deltang -= 360.0;
    if (deltang * total < 0.0) {
      total = 0.0;
    }
    total += deltang;
    lastang = angle;
  }
  return (total > 360.0) || (total < -360.0);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::total_turns
//       Access: Published
//  Description: returns the total angular deviation that the trail
//               has made in the specified time period.  A small
//               number means that the trail is moving in a relatively
//               straight line, a large number means that the trail
//               is zig-zagging or spinning.  The result is in degrees.
////////////////////////////////////////////////////////////////////
double PointerEventList::
total_turns(double sec) const {
  double old = ClockObject::get_global_clock()->get_frame_time() - sec;
  int pos = _events.size()-1;
  double tot = 0.0;
  while ((pos >= 0)&&(_events[pos]._time >= old)) {
    double rot = _events[pos]._rotation;
    if (rot < 0.0) rot = -rot;
    tot += rot;
  }
  return tot;
}
