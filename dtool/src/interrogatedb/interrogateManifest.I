/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateManifest.I
 * @author drose
 * @date 2000-08-11
 */

/**
 *
 */
INLINE InterrogateManifest::
InterrogateManifest(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _int_value = 0;
  _type = 0;
  _getter = 0;
}

/**
 *
 */
INLINE InterrogateManifest::
InterrogateManifest(const InterrogateManifest &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE void InterrogateManifest::
operator = (const InterrogateManifest &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _definition = copy._definition;
  _int_value = copy._int_value;
  _type = copy._type;
  _getter = copy._getter;
}


/**
 *
 */
INLINE const std::string &InterrogateManifest::
get_definition() const {
  return _definition;
}

/**
 *
 */
INLINE bool InterrogateManifest::
has_type() const {
  return (_flags & F_has_type) != 0;
}

/**
 *
 */
INLINE TypeIndex InterrogateManifest::
get_type() const {
  return _type;
}

/**
 *
 */
INLINE bool InterrogateManifest::
has_getter() const {
  return (_flags & F_has_getter) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateManifest::
get_getter() const {
  return _getter;
}

/**
 *
 */
INLINE bool InterrogateManifest::
has_int_value() const {
  return (_flags & F_has_int_value) != 0;
}

/**
 *
 */
INLINE int InterrogateManifest::
get_int_value() const {
  return _int_value;
}


INLINE std::ostream &
operator << (std::ostream &out, const InterrogateManifest &manifest) {
  manifest.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateManifest &manifest) {
  manifest.input(in);
  return in;
}
