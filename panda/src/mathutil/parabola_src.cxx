// Filename: parabola_src.cxx
// Created by:  drose (10Oct07)
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


////////////////////////////////////////////////////////////////////
//     Function: Parabola::xform
//       Access: Published
//  Description: Transforms the parabola by the indicated matrix.
////////////////////////////////////////////////////////////////////
void FLOATNAME(Parabola)::
xform(const FLOATNAME(LMatrix4) &mat) {
  // I'm not really sure if this is the right thing to do here.
  _a = mat.xform_vec_general(_a);
  _b = mat.xform_vec_general(_b);
  _c = mat.xform_point(_c);
}

////////////////////////////////////////////////////////////////////
//     Function: Parabola::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(Parabola)::
output(ostream &out) const {
  out << "Parabola(" << _a << ", " << _b << ", " << _c << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Parabola::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(Parabola)::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Parabola::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(Parabola)::
write_datagram(Datagram &destination) const {
  _a.write_datagram(destination);
  _b.write_datagram(destination);
  _c.write_datagram(destination);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
void FLOATNAME(Parabola)::
read_datagram(DatagramIterator &source) {
  _a.read_datagram(source);
  _b.read_datagram(source);
  _c.read_datagram(source);
}
