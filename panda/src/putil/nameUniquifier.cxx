// Filename: nameUniquifier.cxx
// Created by:  drose (16Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "nameUniquifier.h"

#include <notify.h>

#include <stdio.h>


////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::add_name_body
//       Access: Private
//  Description: The actual implementation of the two flavors of
//               add_name().
//
//               If name is nonempty and so far unique, returns it
//               unchanged.
//
//               Otherwise, generates and returns a new name according
//               to the following rules:
//
//               If the prefix is empty, the new name is the
//               NameUniquifier's "empty" string followed by a number,
//               or the "separator" string if the "empty" string is
//               empty.
//
//               If the prefix is nonempty, the new name is the
//               prefix, followed by the NameUniquifier's "separator"
//               string, followed by a number.
////////////////////////////////////////////////////////////////////
string NameUniquifier::
add_name_body(const string &name, const string &prefix) {
  if (!name.empty()) {
    if (_names.insert(name).second) {
      // The name was successfully inserted into the set; therefore,
      // it's unique.  Return it.
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

