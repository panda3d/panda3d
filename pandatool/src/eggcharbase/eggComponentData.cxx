// Filename: eggComponentData.cxx
// Created by:  drose (26Feb01)
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

#include "eggComponentData.h"
#include "eggBackPointer.h"

#include "indent.h"


////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggComponentData::
EggComponentData(EggCharacterCollection *collection,
                 EggCharacterData *char_data) :
  _collection(collection),
  _char_data(char_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggComponentData::
~EggComponentData() {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      delete back;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::add_name
//       Access: Public
//  Description: Adds the indicated name to the set of names that this
//               component can be identified with.  If this is the
//               first name added, it becomes the primary name of the
//               component; later names added do not replace the
//               primary name, but do get added to the list of names
//               that will be accepted by matched_name().
////////////////////////////////////////////////////////////////////
void EggComponentData::
add_name(const string &name) {
  if (!has_name()) {
    set_name(name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::matches_name
//       Access: Public
//  Description: Returns true if the indicated name matches any name
//               that was ever matched with this particular joint,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggComponentData::
matches_name(const string &name) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggComponentData::set_model
//       Access: Public
//  Description: Sets the back_pointer associated with the given
//               model_index.
////////////////////////////////////////////////////////////////////
void EggComponentData::
set_model(int model_index, EggBackPointer *back) {
  while ((int)_back_pointers.size() <= model_index) {
    _back_pointers.push_back((EggBackPointer *)NULL);
  }

  if (_back_pointers[model_index] != (EggBackPointer *)NULL) {
    nout << "Warning: deleting old back pointer.\n";
    delete _back_pointers[model_index];
  }
  _back_pointers[model_index] = back;
}
