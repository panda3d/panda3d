/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nameUniquifier.I
 * @author drose
 * @date 2000-02-16
 */

/**
 * If name is nonempty and so far unique, returns it unchanged.
 *
 * Otherwise, generates and returns a new name according to the following
 * rules:
 *
 * If the name is empty, the new name is the NameUniquifier's "empty" string
 * followed by a number, or the "separator" string if the "empty" string is
 * empty.
 *
 * If the name is nonempty, the new name is the original name, followed by the
 * NameUniquifier's "separator" string, followed by a number.
 */
INLINE std::string NameUniquifier::
add_name(const std::string &name) {
  return add_name_body(name, name);
}

/**
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
INLINE std::string NameUniquifier::
add_name(const std::string &name, const std::string &prefix) {
  return add_name_body(name, prefix);
}
