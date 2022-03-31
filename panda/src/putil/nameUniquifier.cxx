/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nameUniquifier.cxx
 * @author drose
 * @date 2000-02-16
 */

#include "nameUniquifier.h"

#include "pnotify.h"

#include <stdio.h>

using std::string;


/**
 * Creates a new NameUniquifier.
 *
 * The separator string is used to separate the original name (or supplied
 * prefix) and the generated number when a name must be generated.
 *
 * If the original name is empty, the empty string is used, followed by the
 * generated number.
 */
NameUniquifier::
NameUniquifier(const string &separator,
               const string &empty) :
  _separator(separator),
  _empty(empty)
{
  _counter = 0;

  if (_empty.empty()) {
    _empty = _separator;
  }
}

/**
 *
 */
NameUniquifier::
~NameUniquifier() {
}

/**
 * The actual implementation of the two flavors of add_name().
 *
 * If name is nonempty and so far unique, returns it unchanged.
 *
 * Otherwise, generates and returns a new name according to the following
 * rules:
 *
 * If the prefix is empty, the new name is the NameUniquifier's "empty" string
 * followed by a number, or the "separator" string if the "empty" string is
 * empty.
 *
 * If the prefix is nonempty, the new name is the prefix, followed by the
 * NameUniquifier's "separator" string, followed by a number.
 */
string NameUniquifier::
add_name_body(const string &name, const string &prefix) {
  if (!name.empty()) {
    if (_names.insert(name).second) {
      // The name was successfully inserted into the set; therefore, it's
      // unique.  Return it.
      return name;
    }
  }

  // The name was not successfully inserted; there must be another one
  // already.  Make up a new one.

  // Keep trying to make up names until we make one that's unique.
  string temp_name;
  do {
    static const int max_len = 16;
    char num_str[max_len];
    sprintf(num_str, "%d", ++_counter);
    nassertr((int)strlen(num_str) <= max_len, "");

    if (prefix.empty()) {
      temp_name = _empty + num_str;
    } else {
      temp_name = prefix + _separator + num_str;
    }
  } while (!_names.insert(temp_name).second);

  return temp_name;
}
