// Filename: texGenProperty.h
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

#ifndef TEXGENPROPERTY_H
#define TEXGENPROPERTY_H

#include <pandabase.h>

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : TexGenProperty
// Description : This defines the kinds of texture coordinates that
//               may be automatically generated at render time.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenProperty {
public:
  enum Mode {
    M_none,
    M_eye_linear,
    M_texture_projector,
    M_sphere_map,
  };

public:
  INLINE TexGenProperty();
  INLINE static TexGenProperty texture_projector();
  INLINE static TexGenProperty sphere_map();

  INLINE Mode get_mode() const;
  INLINE const LMatrix4f &get_plane() const;

  INLINE int compare_to(const TexGenProperty &other) const;

  void output(ostream &out) const;

private:
  Mode _mode;
  LMatrix4f _plane;
};

ostream &operator << (ostream &out, TexGenProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const TexGenProperty &prop) {
  prop.output(out);
  return out;
}

#include "texGenProperty.I"

#endif
