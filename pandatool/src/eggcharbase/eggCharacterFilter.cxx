/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterFilter.cxx
 * @author drose
 * @date 2001-02-23
 */

#include "eggCharacterFilter.h"
#include "eggCharacterCollection.h"
#include "eggCharacterData.h"


/**
 *
 */
EggCharacterFilter::
EggCharacterFilter() : EggMultiFilter(false) {
  _collection = nullptr;

  _force_initial_rest_frame = false;
}

/**
 *
 */
EggCharacterFilter::
~EggCharacterFilter() {
  if (_collection != nullptr) {
    delete _collection;
  }
}

/**
 *
 */
void EggCharacterFilter::
add_fixrest_option() {
  add_option
    ("fixrest", "", 30,
     "Specify this to force all the initial rest frames of the various "
     "model files to the same value as the first model specified.  This "
     "is a fairly drastic way to repair models whose initial rest frame "
     "values are completely bogus, but should not be performed when the "
     "input models are correct.",
     &EggCharacterFilter::dispatch_none, &_force_initial_rest_frame);
}


/**
 *
 */
bool EggCharacterFilter::
post_command_line() {
  if (_collection == nullptr) {
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

  _collection->check_errors(nout, _force_initial_rest_frame);

  return true;
}

/**
 * Writes out all of the egg files in the _eggs vector, to the output
 * directory if one is specified, or over the input files if -inplace was
 * specified.
 */
void EggCharacterFilter::
write_eggs() {
  // Optimize (that is, collapse redudant nodes) in all of the characters'
  // joint tables before writing them out.
  int num_characters = _collection->get_num_characters();
  for (int i = 0; i < num_characters; i++) {
    EggCharacterData *char_data = _collection->get_character(i);
    char_data->get_root_joint()->optimize();
  }

  EggMultiFilter::write_eggs();
}

/**
 * Allocates and returns a new EggCharacterCollection structure.  This is
 * primarily intended as a hook so derived classes can customize the type of
 * EggCharacterCollection object used to represent the character information.
 */
EggCharacterCollection *EggCharacterFilter::
make_collection() {
  return new EggCharacterCollection;
}
