// Filename: transparencyProperty.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "transparencyProperty.h"
#include <datagram.h>
#include <datagramIterator.h>

ostream &
operator << (ostream &out, TransparencyProperty::Mode mode) {
  switch (mode) {
  case TransparencyProperty::M_none:
    return out << "none";

  case TransparencyProperty::M_alpha:
    return out << "alpha";

  case TransparencyProperty::M_alpha_sorted:
    return out << "alpha sorted";

  case TransparencyProperty::M_multisample:
    return out << "multisample";

  case TransparencyProperty::M_multisample_mask:
    return out << "multisample mask";

  case TransparencyProperty::M_binary:
    return out << "binary";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void TransparencyProperty::
output(ostream &out) const {
  out << _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               this object to a Datagram
////////////////////////////////////////////////////////////////////
void TransparencyProperty::
write_datagram(Datagram &destination) 
{
  destination.add_uint8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::read_datagram
//       Access: Public
//  Description: Function to write the important information into
//               this object out of a Datagram
////////////////////////////////////////////////////////////////////
void TransparencyProperty::
read_datagram(DatagramIterator &source) 
{
  _mode = (enum Mode) source.get_uint8();
}

