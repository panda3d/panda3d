// Filename: textureApplyProperty.h
// Created by:  drose (23Mar00)
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

#ifndef TEXTUREAPPLYPROPERTY_H
#define TEXTUREAPPLYPROPERTY_H

#include <pandabase.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : TextureApplyProperty
// Description : Defines the way texture colors modify existing
//               geometry colors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureApplyProperty {
public:
  enum Mode {
    M_modulate,
    M_decal,
    M_blend,
    M_replace,
    M_add
  };

public:
  INLINE TextureApplyProperty(Mode mode);
  INLINE TextureApplyProperty(void);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const TextureApplyProperty &other) const;

  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Mode _mode;
};

ostream &operator << (ostream &out, TextureApplyProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const TextureApplyProperty &prop) {
  prop.output(out);
  return out;
}

#include "textureApplyProperty.I"

#endif
