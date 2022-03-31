/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEventList.cxx
 * @author jyelon
 * @date 2007-09-20
 */

#include "pointerEventList.h"
#include "indent.h"
#include "config_event.h"
#include "clockObject.h"
#include "deg_2_rad.h"
#include "cmath.h"

TypeHandle PointerEventList::_type_handle;

/**
 * Compute the difference between two angles.  Returns a value in the range
 * -180 to 180.
 */
INLINE double delta_angle(double angle1, double angle2) {
  double deltang = angle2 - angle1;
  while (deltang < -180.0) deltang += 360.0;
  while (deltang >  180.0) deltang -= 360.0;
  return deltang;
}


/**
 * Compute the difference between two angles.  Returns a value in the range
 * -180 to 180.
 */
INLINE double normalize_angle(double angle) {
  while (angle <   0.0) angle += 360.0;
  while (angle > 360.0) angle -= 360.0;
  return angle;
}



/**
 *
 */
void PointerEventList::
output(std::ostream &out) const {
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

/**
 *
 */
void PointerEventList::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << _events.size() << " events:\n";
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    indent(out, indent_level + 2) << (*ei) << "\n";
  }
}

/**
 * Adds a new event from the given PointerData object.
 */
void PointerEventList::
add_event(const PointerData &data, int seq, double time) {
  PointerEvent pe;
  pe._in_window = data._in_window;
  pe._type = data._type;
  pe._id = data._id;
  pe._xpos = data._xpos;
  pe._ypos = data._ypos;
  pe._pressure = data._pressure;
  pe._sequence = seq;
  pe._time = time;
  if (_events.size() > 0) {
    pe._dx = data._xpos - _events.back()._xpos;
    pe._dy = data._ypos - _events.back()._ypos;
    double ddx = pe._dx;
    double ddy = pe._dy;
    pe._length = csqrt(ddx*ddx + ddy*ddy);
    if (pe._length > 0.0) {
      pe._direction = normalize_angle(rad_2_deg(catan2(-ddy,ddx)));
    } else {
      pe._direction = _events.back()._direction;
    }
    pe._rotation = delta_angle(_events.back()._direction, pe._direction);
  } else {
    pe._dx = 0;
    pe._dy = 0;
    pe._length = 0.0;
    pe._direction = 0.0;
    pe._rotation = 0.0;
  }
  _events.push_back(pe);
}

/**
 * Adds a new event to the end of the list.  Automatically calculates the dx,
 * dy, length, direction, and rotation for all but the first event.
 */
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
    pe._length = csqrt(ddx*ddx + ddy*ddy);
    if (pe._length > 0.0) {
      pe._direction = normalize_angle(rad_2_deg(catan2(-ddy,ddx)));
    } else {
      pe._direction = _events.back()._direction;
    }
    pe._rotation = delta_angle(_events.back()._direction, pe._direction);
  } else {
    pe._dx = 0;
    pe._dy = 0;
    pe._length = 0.0;
    pe._direction = 0.0;
    pe._rotation = 0.0;
  }
  _events.push_back(pe);
}

/**
 * Adds a new event to the end of the list based on the given mouse movement.
 */
void PointerEventList::
add_event(bool in_win, int xpos, int ypos, double xdelta, double ydelta, int seq, double time) {
  PointerEvent pe;
  pe._in_window = in_win;
  pe._xpos = xpos;
  pe._ypos = ypos;
  pe._dx = xdelta;
  pe._dy = ydelta;
  pe._sequence = seq;
  pe._time = time;
  pe._length = csqrt(xdelta*xdelta + ydelta*ydelta);
  if (pe._length > 0.0) {
    pe._direction = normalize_angle(rad_2_deg(catan2(-ydelta,xdelta)));
  } else if (!_events.empty()) {
    pe._direction = _events.back()._direction;
  } else {
    pe._direction = 0.0;
  }
  if (!_events.empty()) {
    pe._rotation = delta_angle(_events.back()._direction, pe._direction);
  } else {
    pe._rotation = 0.0;
  }
  _events.push_back(pe);
}

/**
 * Returns true if the trail loops around the specified point.
 */
bool PointerEventList::
encircles(int x, int y) const {
  int tot_events = _events.size();
  if (tot_events < 3) {
    return false;
  }
  int last = tot_events-1;
  double dx = _events[last]._xpos - x;
  double dy = _events[last]._ypos - y;
  double lastang = rad_2_deg(catan2(dy, dx));
  double total = 0.0;
  for (int i=last; (i>=0) && (total < 360.0) && (total > -360.0); i--) {
    dx = _events[i]._xpos - x;
    dy = _events[i]._ypos - y;
    if ((dx==0.0)&&(dy==0.0)) {
      continue;
    }
    double angle = rad_2_deg(catan2(dy,dx));
    double deltang = delta_angle(lastang, angle);
    if (deltang * total < 0.0) {
      total = 0.0;
    }
    total += deltang;
    lastang = angle;
  }
  return (total > 360.0) || (total < -360.0);
}

/**
 * returns the total angular deviation that the trail has made in the
 * specified time period.  A small number means that the trail is moving in a
 * relatively straight line, a large number means that the trail is zig-
 * zagging or spinning.  The result is in degrees.
 */
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

/**
 * This function is not implemented yet.  It is a work in progress.  The
 * intent is as follows:
 *
 * Returns a nonzero value if the mouse movements match the specified pattern.
 * The higher the value, the better the match.  The pattern is a sequence of
 * compass directions (ie, "E", "NE", etc) separated by spaces.  If rot is
 * nonzero, then the pattern is rotated counterclockwise by the specified
 * amount before testing.  Seglen is the minimum length a mouse movement needs
 * to be in order to be considered significant.
 */
double PointerEventList::
match_pattern(const std::string &ascpat, double rot, double seglen) {
  // Convert the pattern from ascii to a more usable form.
  vector_double pattern;
  parse_pattern(ascpat, pattern);

  // Apply the rotation to the pattern.
  for (size_t i=0; i<pattern.size(); i++) {
    pattern[i] = normalize_angle(pattern[i] + rot);
  }

  return 0.0;
}

/**
 * Parses a pattern as used by match_pattern.
 */
void PointerEventList::
parse_pattern(const std::string &ascpat, vector_double &pattern) {
  int chars = 0;
  double dir = 180.0;
  for (size_t i=0; i<ascpat.size(); i++) {
    char c = ascpat[i];
    double ang = -1.0;
    if      ((c=='E')||(c=='e')) ang=0.0;
    else if ((c=='N')||(c=='n')) ang=90.0;
    else if ((c=='W')||(c=='w')) ang=180.0;
    else if ((c=='S')||(c=='s')) ang=270.0;
    if (ang >= 0.0) {
      double offset = delta_angle(dir, ang);
      double newang = dir + offset;
      dir = normalize_angle((dir * chars + newang) / (chars + 1));
      chars += 1;
    } else {
      if ((c != ' ')&&(c != '\t')) {
        event_cat.warning() <<
          "Invalid pattern in PointerEventList::match_pattern\n";
        pattern.clear();
        return;
      }
      if (chars > 0) {
        pattern.push_back(dir);
      }
      chars = 0;
      dir = 180.0;
    }
  }
  if (chars > 0) {
    pattern.push_back(dir);
  }

  std::cerr << "Pattern: ";
  for (int i=0; i<(int)pattern.size(); i++) {
    std::cerr << pattern[i] << " ";
  }
  std::cerr << "\n";
}
