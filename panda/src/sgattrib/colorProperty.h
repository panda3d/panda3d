// Filename: colorProperty.h
// Created by:  drose (22Mar00)
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

#ifndef COLORPROPERTY_H
#define COLORPROPERTY_H

#include <pandabase.h>

#include <luse.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : ColorProperty
// Description : This class defines the scene graph color property
//               that we may set on a transition.  It makes a
//               distinction between a real color and the uncolor.
//               When a "real" color is set, it specifies the RGBA
//               that will be sent in lieu of vertex color.  When a
//               "non-real" color (i.e. the uncolor) is set, it
//               disables the sending of any color information at all.
//               When no color is set (e.g. the transition is "off" or
//               absent), the vertex color is sent.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorProperty {
public:
  INLINE ColorProperty();
  INLINE ColorProperty(const Colorf &color);

  INLINE bool is_real() const;
  INLINE Colorf get_color() const;

  INLINE int compare_to(const ColorProperty &other) const;
  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Colorf _color;
  bool _real;
};

INLINE ostream &operator << (ostream &out, const ColorProperty &prop) {
  prop.output(out);
  return out;
}

#include "colorProperty.I"

#endif
