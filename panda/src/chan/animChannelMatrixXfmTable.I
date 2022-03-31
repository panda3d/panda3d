/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixXfmTable.I
 * @author drose
 * @date 1999-02-21
 */

/**
 * Returns true if the given letter is one of the nine valid table id's.
 */
INLINE bool AnimChannelMatrixXfmTable::
is_valid_id(char table_id) {
  return get_table_index(table_id) >= 0;
}

/**
 * Returns a pointer to the indicated subtable's data, if it exists, or NULL
 * if it does not.
 */
INLINE CPTA_stdfloat AnimChannelMatrixXfmTable::
get_table(char table_id) const {
  int table_index = get_table_index(table_id);
  if (table_index < 0) {
    return CPTA_stdfloat(get_class_type());
  }
  return _tables[table_index];
}

/**
 * Returns true if the indicated subtable has been assigned.
 */
INLINE bool AnimChannelMatrixXfmTable::
has_table(char table_id) const {
  int table_index = get_table_index(table_id);
  if (table_index < 0) {
    return false;
  }
  return !(_tables[table_index] == nullptr);
}

/**
 * Removes the indicated table from the definition.
 */
INLINE void AnimChannelMatrixXfmTable::
clear_table(char table_id) {
  int table_index = get_table_index(table_id);
  if (table_index >= 0) {
    _tables[table_index] = nullptr;
  }
}


/**
 * Returns the table ID associated with the indicated table index number.
 * This is the letter 'i', 'j', 'k', 'a', 'b', 'c', 'h', 'p', 'r', 'x', 'y',
 * or 'z'.
 */
INLINE char AnimChannelMatrixXfmTable::
get_table_id(int table_index) {
  nassertr(table_index >= 0 && table_index < num_matrix_components, '\0');
  return matrix_component_letters[table_index];
}


/**
 * Returns the default value the indicated table is expected to have in the
 * absence of any data.
 */
INLINE PN_stdfloat AnimChannelMatrixXfmTable::
get_default_value(int table_index) {
  nassertr(table_index >= 0 && table_index < num_matrix_components, 0.0);
  return matrix_component_defaults[table_index];
}
