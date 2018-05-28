/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterCollection.I
 * @author drose
 * @date 2001-02-26
 */

/**
 * Returns the number of egg files that have successfully been added to the
 * Character table.
 */
INLINE int EggCharacterCollection::
get_num_eggs() const {
  return _eggs.size();
}

/**
 * Returns the ith egg file.
 */
INLINE EggData *EggCharacterCollection::
get_egg(int i) const {
  nassertr(i >= 0 && i < (int)_eggs.size(), nullptr);
  return _eggs[i]._egg;
}

/**
 * Returns the first model index associated with the indicated egg file.  An
 * egg file may contain multiple models, which will be consecutive integers
 * beginning at get_first_model_index() and continuing for get_num_models().
 *
 * Each "model" corresponds to a single character model, or one LOD of a
 * multiple-LOD model, or a single animation bundle.
 */
INLINE int EggCharacterCollection::
get_first_model_index(int egg_index) const {
  nassertr(egg_index >= 0 && egg_index < (int)_eggs.size(), 0);
  return _eggs[egg_index]._first_model_index;
}

/**
 * Returns the number of different models found in the indicated egg file.  An
 * egg file may contain multiple models, which will be consecutive integers
 * beginning at get_first_model_index() and continuing for get_num_models().
 *
 * Each "model" corresponds to a single character model, or one LOD of a
 * multiple-LOD model, or a single animation bundle.
 */
INLINE int EggCharacterCollection::
get_num_models(int egg_index) const {
  nassertr(egg_index >= 0 && egg_index < (int)_eggs.size(), 0);
  return _eggs[egg_index]._models.size();
}

/**
 * Returns the number of separate Characters that have been discovered in the
 * various egg files added to the collection.
 */
INLINE int EggCharacterCollection::
get_num_characters() const {
  return _characters.size();
}

/**
 * Returns the ith character in the collection.
 */
INLINE EggCharacterData *EggCharacterCollection::
get_character(int i) const {
  nassertr(i >= 0 && i < (int)_characters.size(), nullptr);
  return _characters[i];
}

/**
 * Returns the character associated with the indicated model index.
 */
INLINE EggCharacterData *EggCharacterCollection::
get_character_by_model_index(int model_index) const {
  nassertr(model_index >= 0 && model_index < (int)_characters_by_model_index.size(),
           nullptr);
  return _characters_by_model_index[model_index];
}

/**
 *
 */
INLINE EggCharacterCollection::ModelDescription::
ModelDescription() {
  _root_node = nullptr;
}
