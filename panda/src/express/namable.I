/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file namable.I
 * @author drose
 * @date 2000-02-16
 */

/**
 *
 */
INLINE Namable::
Namable(const std::string &initial_name) :
  _name(initial_name)
{
}

/**
 *
 */
INLINE void Namable::
set_name(const std::string &name) {
  _name = name;
}

/**
 * Resets the Namable's name to empty.
 */
INLINE void Namable::
clear_name() {
  _name = "";
}

/**
 * Returns true if the Namable has a nonempty name set, false if the name is
 * empty.
 */
INLINE bool Namable::
has_name() const {
  return !_name.empty();
}

/**
 *
 */
INLINE const std::string &Namable::
get_name() const {
  return _name;
}

/**
 * Outputs the Namable.  This function simply writes the name to the output
 * stream; most Namable derivatives will probably redefine this.
 */
INLINE void Namable::
output(std::ostream &out) const {
  out << get_name();
}


INLINE std::ostream &operator << (std::ostream &out, const Namable &n) {
  n.output(out);
  return out;
}

/**
 *
 */
INLINE bool NamableOrderByName::
operator ()(const Namable *n1, const Namable *n2) const {
  return (n1->get_name() < n2->get_name());
}
