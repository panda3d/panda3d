// Filename: windowsGuid.I
// Created by:  drose (03Oct04)
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
//     Function: WindowsGuid::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WindowsGuid::
WindowsGuid() {
  memset(this, 0, sizeof(WindowsGuid));
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WindowsGuid::
WindowsGuid(unsigned long data1, 
            unsigned short data2, unsigned short data3,
            unsigned char b1, unsigned char b2, unsigned char b3, 
            unsigned char b4, unsigned char b5, unsigned char b6, 
            unsigned char b7, unsigned char b8) :
  _data1(data1),
  _data2(data2),
  _data3(data3),
  _b1(b1),
  _b2(b2),
  _b3(b3),
  _b4(b4),
  _b5(b5),
  _b6(b6),
  _b7(b7),
  _b8(b8)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WindowsGuid::
WindowsGuid(const WindowsGuid &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WindowsGuid::
operator = (const WindowsGuid &copy) {
  memcpy(this, &copy, sizeof(WindowsGuid));
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::operator ==
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool WindowsGuid::
operator == (const WindowsGuid &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool WindowsGuid::
operator != (const WindowsGuid &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::operator <
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool WindowsGuid::
operator < (const WindowsGuid &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsGuid::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this WindowsGuid
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
////////////////////////////////////////////////////////////////////
INLINE int WindowsGuid::
compare_to(const WindowsGuid &other) const {
  return memcmp(this, &other, sizeof(WindowsGuid));
}

INLINE ostream &
operator << (ostream &out, const WindowsGuid &guid) {
  guid.output(out);
  return out;
}

