// Filename: sampleClass.I
// Created by:  drose (10Jun00)
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
//     Function: SampleClass::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SampleClass::
SampleClass(const SampleClass &copy) :
  _public_data_member(copy._public_data_member),
  _private_data_member(copy._private_data_member),
  _flag(copy._flag)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SampleClass::
~SampleClass() {
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::set_flag
//       Access: Public
//  Description: A few sentences describing the purpose of flag, the
//               legal values it may be set to, and the implications
//               of setting it to certain values.
////////////////////////////////////////////////////////////////////
INLINE void SampleClass::
set_flag(int flag) {
  _flag = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::get_flag
//       Access: Public
//  Description: A few sentences describing the purpose of flag, the
//               legal values it may be set to, and the implications
//               of setting it to certain values.
////////////////////////////////////////////////////////////////////
INLINE int SampleClass::
get_flag() const {
  return _flag;
}

