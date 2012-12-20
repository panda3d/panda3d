// Filename: eggSaver.I
// Created by:  drose (19Dec12)
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
//     Function: EggSaver::get_egg_data
//       Access: Published
//  Description: Returns the EggData populated within this class.
////////////////////////////////////////////////////////////////////
INLINE EggData *EggSaver::
get_egg_data() const {
  return _data;
}

