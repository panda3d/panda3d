// Filename: writableParam.I
// Created by:  jason (13Jun00)
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
//     Function: WritableParam::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WritableParam::
WritableParam(const Datagram &datagram) :
  _packet(datagram)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WritableParam::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WritableParam::
WritableParam(const WritableParam &copy) :
  FactoryParam(copy),
  _packet(copy._packet)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WritableParam::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WritableParam::
~WritableParam(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: WritableParam::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WritableParam::
operator = (const WritableParam &) {
  // The assignment operator cannot be used for this class.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: WritableParam::get_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Datagram& WritableParam::
get_datagram(void) {
  return _packet;
}

