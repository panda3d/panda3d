// Filename: auxBitplaneAttrib.I
// Created by:  drose (04Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::Constructor
//       Access: Private
//  Description: Use AuxBitplaneAttrib::make() to construct a new
//               AuxBitplaneAttrib object.
////////////////////////////////////////////////////////////////////
INLINE AuxBitplaneAttrib::
AuxBitplaneAttrib(int outputs) :
  _outputs(outputs)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::get_outputs
//       Access: Published
//  Description: Returns the AuxBitplaneAttrib output bits.
////////////////////////////////////////////////////////////////////
INLINE int AuxBitplaneAttrib::
get_outputs() const {
  return _outputs;
}

