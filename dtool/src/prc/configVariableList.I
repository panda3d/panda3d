/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableList.I
 * @author drose
 * @date 2004-10-20
 */

/**
 *
 */
INLINE ConfigVariableList::
~ConfigVariableList() {
}

/**
 *
 */
INLINE ConfigVariableList::
ConfigVariableList(const std::string &name,
                   const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariableBase(name, VT_list, description, flags)
#else
  ConfigVariableBase(name, VT_list, std::string(), flags)
#endif
{
  // A list variable implicitly defines a default value of the empty string.
  // This is just to prevent the core variable from complaining should anyone
  // ask for its solitary value.
  if (_core->get_default_value() == nullptr) {
    _core->set_default_value("");
  }
  _core->set_used();
}

/**
 * Returns the number of values in the variable.
 */
INLINE size_t ConfigVariableList::
get_num_values() const {
  nassertr(_core != nullptr, 0);
  return _core->get_num_trusted_references();
}

/**
 * Returns the nth value of the variable.
 */
INLINE std::string ConfigVariableList::
get_string_value(size_t n) const {
  nassertr(_core != nullptr, std::string());
  const ConfigDeclaration *decl = _core->get_trusted_reference(n);
  if (decl != nullptr) {
    return decl->get_string_value();
  }
  return std::string();
}

/**
 * Returns the number of unique values in the variable.
 */
INLINE size_t ConfigVariableList::
get_num_unique_values() const {
  nassertr(_core != nullptr, 0);
  return _core->get_num_unique_references();
}

/**
 * Returns the nth unique value of the variable.
 */
INLINE std::string ConfigVariableList::
get_unique_value(size_t n) const {
  nassertr(_core != nullptr, std::string());
  const ConfigDeclaration *decl = _core->get_unique_reference(n);
  if (decl != nullptr) {
    return decl->get_string_value();
  }
  return std::string();
}

/**
 * Returns the number of unique values of the variable.
 */
INLINE size_t ConfigVariableList::
size() const {
  return get_num_unique_values();
}

/**
 * Returns the nth unique value of the variable.  Note that the indexing
 * operator returns the list of unique values, and so the maximum range is
 * get_num_unique_values().
 */
INLINE std::string ConfigVariableList::
operator [] (size_t n) const {
  return get_unique_value(n);
}

INLINE std::ostream &
operator << (std::ostream &out, const ConfigVariableList &variable) {
  variable.output(out);
  return out;
}
