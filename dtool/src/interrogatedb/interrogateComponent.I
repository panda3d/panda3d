/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateComponent.I
 * @author drose
 * @date 2000-08-08
 */

/**
 *
 */
INLINE InterrogateComponent::
InterrogateComponent(InterrogateModuleDef *def) :
  _def(def)
{
}

/**
 *
 */
INLINE InterrogateComponent::
InterrogateComponent(const InterrogateComponent &copy) :
  _def(copy._def),
  _name(copy._name)
{
}

/**
 *
 */
INLINE void InterrogateComponent::
operator = (const InterrogateComponent &copy) {
  _def = copy._def;
  _name = copy._name;
}

/**
 * Returns true if we have a known library name, false if we do not.  See
 * get_library_name().
 */
INLINE bool InterrogateComponent::
has_library_name() const {
  const char *name = get_library_name();
  return (name != nullptr && name[0] != '\0');
}

/**
 * Returns the library name, if it is known, or NULL if it is not.  This is
 * the name of the library that this particular component was built into.
 * Typically this will be a one-to-one correspondance with an invocation of
 * the interrogate command.  Typical examples are "libutil" and "liblinmath".
 */
INLINE const char *InterrogateComponent::
get_library_name() const {
  if (_def != nullptr) {
    return _def->library_name;
  }
  return nullptr;
}

/**
 * Returns true if we have a known module name, false if we do not.  See
 * get_module_name().
 */
INLINE bool InterrogateComponent::
has_module_name() const {
  const char *name = get_module_name();
  return (name != nullptr && name[0] != '\0');
}

/**
 * Returns the module name, if it is known, or NULL if it is not.  This is the
 * name of the module that this particular component is associated with.  This
 * is a higher grouping than library.  Typical examples are "panda" and
 * "pandaegg".
 */
INLINE const char *InterrogateComponent::
get_module_name() const {
  if (_def != nullptr) {
    return _def->module_name;
  }
  return nullptr;
}

/**
 *
 */
INLINE bool InterrogateComponent::
has_name() const {
  return !_name.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateComponent::
get_name() const {
  return _name;
}

/**
 *
 */
INLINE int InterrogateComponent::
get_num_alt_names() const {
  return _alt_names.size();
}

/**
 *
 */
INLINE const std::string &InterrogateComponent::
get_alt_name(int n) const {
  if (n >= 0 && n < (int)_alt_names.size()) {
    return _alt_names[n];
  }
  return _empty_string;
}
