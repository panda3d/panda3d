// Filename: fogAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: FogAttrib::Constructor
//       Access: Private
//  Description: Use FogAttrib::make() to construct a new FogAttrib
//               object.
////////////////////////////////////////////////////////////////////
INLINE FogAttrib::
FogAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::is_off
//       Access: Published
//  Description: Returns true if the FogAttrib is an 'off' FogAttrib,
//               indicating that it should disable fog.
////////////////////////////////////////////////////////////////////
INLINE bool FogAttrib::
is_off() const {
  return _fog == (const Fog *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::get_fog
//       Access: Published
//  Description: If the FogAttrib is not an 'off' FogAttrib,
//               returns the fog that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE Fog *FogAttrib::
get_fog() const {
  return _fog;
}
