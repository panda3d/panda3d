// Filename: eggCharacterFilter.cxx
// Created by:  drose (23Feb01)
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

#include "eggCharacterFilter.h"
#include "eggCharacterCollection.h"
#include "eggCharacterData.h"


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
EggCharacterFilter() : EggMultiFilter(false) {
  _collection = (EggCharacterCollection *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
~EggCharacterFilter() {
  if (_collection != (EggCharacterCollection *)NULL) {
    delete _collection;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::post_command_line
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggCharacterFilter::
post_command_line() {
  if (_collection == (EggCharacterCollection *)NULL) {
    _collection = make_collection();
  }

  if (!EggMultiFilter::post_command_line()) {
    return false;
  }

  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    EggData *data = (*ei);

    if (_collection->add_egg(data) < 0) {
      nout << data->get_egg_filename().get_basename()
           << " does not contain a character model or animation channel.\n";
      return false;
    }
  }

  _collection->check_errors(nout);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::write_eggs
//       Access: Protected, Virtual
//  Description: Writes out all of the egg files in the _eggs vector,
//               to the output directory if one is specified, or over
//               the input files if -inplace was specified.
////////////////////////////////////////////////////////////////////
void EggCharacterFilter::
write_eggs() {
  // Optimize (that is, collapse redudant nodes) in all of the
  // characters' joint tables before writing them out.
  int num_characters = _collection->get_num_characters();
  for (int i = 0; i < num_characters; i++) {
    EggCharacterData *char_data = _collection->get_character(i);
    char_data->get_root_joint()->optimize();
  }

  EggMultiFilter::write_eggs();
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::make_collection
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggCharacterCollection
//               structure.  This is primarily intended as a hook so
//               derived classes can customize the type of
//               EggCharacterCollection object used to represent the
//               character information.
////////////////////////////////////////////////////////////////////
EggCharacterCollection *EggCharacterFilter::
make_collection() {
  return new EggCharacterCollection;
}
