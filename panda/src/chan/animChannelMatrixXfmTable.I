// Filename: animChannelMatrixXfmTable.I
// Created by:  drose (21Feb99)
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
//     Function: AnimChannelMatrixXfmTable::is_valid_id
//       Access: Public, Static
//  Description: Returns true if the given letter is one of the nine
//               valid table id's.
////////////////////////////////////////////////////////////////////
INLINE bool AnimChannelMatrixXfmTable::
is_valid_id(char table_id) {
  return get_table_index(table_id) >= 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::has_table
//       Access: Public
//  Description: Returns true if the indicated subtable has been
//               assigned.
////////////////////////////////////////////////////////////////////
INLINE bool AnimChannelMatrixXfmTable::
has_table(char table_id) const {
  int table_index = get_table_index(table_id);
  if (table_index < 0) {
    return false;
  }
  return !(_tables[table_index] == (const float *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_table
//       Access: Public
//  Description: Returns a pointer to the indicated subtable's data,
//               if it exists, or NULL if it does not.
////////////////////////////////////////////////////////////////////
INLINE CPTA_float AnimChannelMatrixXfmTable::
get_table(char table_id) const {
  int table_index = get_table_index(table_id);
  if (table_index < 0) {
    return CPTA_float();
  }
  return _tables[table_index];
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::clear_table
//       Access: Public
//  Description: Removes the indicated table from the definition.
////////////////////////////////////////////////////////////////////
INLINE void AnimChannelMatrixXfmTable::
clear_table(char table_id) {
  int table_index = get_table_index(table_id);
  if (table_index >= 0) {
    _tables[table_index] = NULL;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_table_id
//       Access: Protected, Static
//  Description: Returns the table ID associated with the indicated
//               table index number.  This is the letter 'i', 'j',
//               'k', 'a', 'b', 'c', 'h', 'p', 'r', 'x', 'y', or 'z'.
////////////////////////////////////////////////////////////////////
INLINE char AnimChannelMatrixXfmTable::
get_table_id(int table_index) {
  nassertr(table_index >= 0 && table_index < num_matrix_components, '\0');
  return matrix_component_letters[table_index];
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_default_value
//       Access: Protected, Static
//  Description: Returns the default value the indicated table is
//               expected to have in the absence of any data.
////////////////////////////////////////////////////////////////////
INLINE float AnimChannelMatrixXfmTable::
get_default_value(int table_index) {
  nassertr(table_index >= 0 && table_index < num_matrix_components, 0.0);
  return matrix_component_defaults[table_index];
}

