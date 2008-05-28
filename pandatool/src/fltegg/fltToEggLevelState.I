// Filename: fltToEggLevelState.I
// Created by:  drose (18Apr01)
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
//     Function: FltToEggLevelState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FltToEggLevelState::
FltToEggLevelState(FltToEggConverter *converter) :
  _converter(converter)
{
  _flt_object = (FltObject *)NULL;
  _egg_parent = (EggGroupNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FltToEggLevelState::
FltToEggLevelState(const FltToEggLevelState &copy) :
  _flt_object(copy._flt_object),
  _egg_parent(copy._egg_parent),
  _converter(copy._converter)
{
  // We don't bother to copy the _parents map.
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FltToEggLevelState::
operator = (const FltToEggLevelState &copy) {
  _flt_object = copy._flt_object;
  _egg_parent = copy._egg_parent;
  _converter = copy._converter;
  // We don't bother to copy the _parents map.
}
