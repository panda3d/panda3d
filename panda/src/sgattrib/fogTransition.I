// Filename: fogTransition.I
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
//     Function: FogTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FogTransition::
FogTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FogTransition::
FogTransition(Fog *fog) :
  OnOffTransition(TD_on),
  _value(fog)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::off
//       Access: Public
//  Description: This named constructor returns a FogTransition that
//               turns off fog.
////////////////////////////////////////////////////////////////////
INLINE FogTransition FogTransition::
off() {
  FogTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::set_on
//       Access: Public
//  Description: Changes the FogTransition to render the particular
//               fog.
////////////////////////////////////////////////////////////////////
INLINE void FogTransition::
set_on(Fog *fog) {
  nassertv(fog != (Fog *)NULL);
  _value = fog;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::get_fog
//       Access: Public
//  Description: Returns the fog that the FogTransition represents.
//               It is only valid to call this if is_on() has returned
//               true.
////////////////////////////////////////////////////////////////////
INLINE Fog *FogTransition::
get_fog() const {
  nassertr(is_on(), NULL);
  return _value;
}
