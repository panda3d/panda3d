// Filename: textureApplyProperty.cxx
// Created by:  drose (23Mar00)
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



