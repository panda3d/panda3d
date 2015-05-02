// Filename: eggToSomethingConverter.cxx
// Created by:  drose (26Apr01)
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

#include "eggToSomethingConverter.h"

#include "eggData.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToSomethingConverter::
EggToSomethingConverter() {
  _egg_data = (EggData *)NULL;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToSomethingConverter::
EggToSomethingConverter(const EggToSomethingConverter &copy) {
  _egg_data = (EggData *)NULL;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggToSomethingConverter::
~EggToSomethingConverter() {
  clear_egg_data();
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::set_egg_data
//       Access: Public
//  Description: Sets the egg data that will be filled in when
//               convert_file() is called.  This must be called before
//               convert_file().
////////////////////////////////////////////////////////////////////
void EggToSomethingConverter::
set_egg_data(EggData *egg_data) {
  _egg_data = egg_data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::get_additional_extensions
//       Access: Public, Virtual
//  Description: Returns a space-separated list of extension, in
//               addition to the one returned by get_extension(), that
//               are recognized by this converter.
////////////////////////////////////////////////////////////////////
string EggToSomethingConverter::
get_additional_extensions() const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomethingConverter::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently save
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool EggToSomethingConverter::
supports_compressed() const {
  return false;
}
