// Filename: textureApplyProperty.cxx
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

#include "textureApplyProperty.h"
#include <datagram.h>
#include <datagramIterator.h>

ostream &
operator << (ostream &out, TextureApplyProperty::Mode mode) {
  switch (mode) {
  case TextureApplyProperty::M_modulate:
    return out << "modulate";

  case TextureApplyProperty::M_decal:
    return out << "decal";

  case TextureApplyProperty::M_blend:
    return out << "blend";

  case TextureApplyProperty::M_replace:
    return out << "replace";

  case TextureApplyProperty::M_add:
    return out << "add";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TextureApplyProperty::
output(ostream &out) const {
  out << _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               this object to a Datagram
////////////////////////////////////////////////////////////////////
void TextureApplyProperty::
write_datagram(Datagram &destination)
{
  destination.add_uint8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::read_datagram
//       Access: Public
//  Description: Function to write the important information into
//               this object out of a Datagram
////////////////////////////////////////////////////////////////////
void TextureApplyProperty::
read_datagram(DatagramIterator &source)
{
  _mode = (enum Mode) source.get_uint8();
}



