/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNamedObject.I
 * @author drose
 * @date 1999-02-10
 */

/**
 *
 */
INLINE EggNamedObject::
EggNamedObject(const std::string &name) : Namable(name) {
}


/**
 *
 */
INLINE EggNamedObject::
EggNamedObject(const EggNamedObject &copy) : EggObject(copy), Namable(copy) {
}


/**
 *
 */
INLINE EggNamedObject &EggNamedObject::
operator = (const EggNamedObject &copy) {
  EggObject::operator = (copy);
  Namable::operator = (copy);
  return *this;
}

INLINE std::ostream &operator << (std::ostream &out, const EggNamedObject &n) {
  n.output(out);
  return out;
}
