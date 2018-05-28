/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComponentData.I
 * @author drose
 * @date 2001-02-26
 */

/**
 * Returns the maximum number of back pointers this component may have.  The
 * component may store a back pointer for models indexed 0 .. num_models - 1.
 * You must call has_model() on each model index to confirm whether a
 * particular model in that range has a back pointer.
 */
INLINE int EggComponentData::
get_num_models() const {
  return _back_pointers.size();
}

/**
 * Returns true if the component has a back pointer to an egg file somewhere
 * for the indicated model, false otherwise.
 */
INLINE bool EggComponentData::
has_model(int model_index) const {
  if (model_index >= 0 && model_index < (int)_back_pointers.size()) {
    return _back_pointers[model_index] != nullptr;
  }
  return false;
}

/**
 * Returns the back pointer to an egg file for the indicated model if it
 * exists, or NULL if it does not.
 */
INLINE EggBackPointer *EggComponentData::
get_model(int model_index) const {
  if (model_index >= 0 && model_index < (int)_back_pointers.size()) {
    return _back_pointers[model_index];
  }
  return nullptr;
}
