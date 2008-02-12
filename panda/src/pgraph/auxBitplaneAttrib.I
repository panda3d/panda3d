// Filename: auxBitplaneAttrib.I
// Created by:  drose (04Mar02)
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

