// Filename: renderModeProperty.h
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

#ifndef RENDERMODEPROPERTY_H
#define RENDERMODEPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : RenderModeProperty
// Description : This defines the types of renderMode testing we can
//               enable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderModeProperty {
public:
  enum Mode {
    M_filled,
    M_wireframe,

    // Perhaps others to be added later.
  };

public:
  INLINE RenderModeProperty(Mode mode, double line_width);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE void set_line_width(double line_width);
  INLINE double get_line_width() const;

  INLINE int compare_to(const RenderModeProperty &other) const;
  void output(ostream &out) const;

private:
  Mode _mode;
  double _line_width;
};

ostream &operator << (ostream &out, RenderModeProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const RenderModeProperty &prop) {
  prop.output(out);
  return out;
}

#include "renderModeProperty.I"

#endif
