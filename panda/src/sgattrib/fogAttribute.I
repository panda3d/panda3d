// Filename: fogAttribute.I
// Created by:  drose (24Mar00)
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

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FogAttribute::
FogAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FogAttribute::
set_on(Fog *fog) {
  nassertv(fog != (Fog *)NULL);
  _value = fog;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::get_fog
//       Access: Public
//  Description: Returns the fog that the FogAttribute represents.  It
//               is only valid to call this if is_on() has returned
//               true.
////////////////////////////////////////////////////////////////////
INLINE Fog *FogAttribute::
get_fog() const {
  nassertr(is_on(), NULL);
  return _value;
}
