/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableCore.I
 * @author drose
 * @date 2004-10-15
 */

/**
 * Returns the name of the variable.
 */
INLINE const std::string &ConfigVariableCore::
get_name() const {
  return _name;
}

/**
 * Returns true if the variable has been referenced by a ConfigVariable
 * somewhere in code, false otherwise.
 */
INLINE bool ConfigVariableCore::
is_used() const {
  return _is_used;
}

/**
 * Returns the stated type of this variable.  If the variable has not yet been
 * defined, this will be VT_undefined.
 */
INLINE ConfigVariableCore::ValueType ConfigVariableCore::
get_value_type() const {
  return _value_type;
}

/**
 * Returns the brief description of this variable, if it has been defined.
 */
INLINE const std::string &ConfigVariableCore::
get_description() const {
  return _description;
}

/**
 * Returns the flags value as set by set_flags().  This includes the trust
 * level and some other settings.  See the individual methods is_closed(),
 * get_trust_level(), etc.  to pull out the semantic meaning of these flags
 * individually.
 */
INLINE int ConfigVariableCore::
get_flags() const {
  return _flags;
}

/**
 * Returns true if the variable is not trusted by any prc file (and hence
 * cannot be modified from its compiled-in default value), or false for the
 * normal case, in which the variable can be modified by any prc file at or
 * above its trust level (see get_trust_level()).
 *
 * This value only has effect in a release build (specifically, when
 * PRC_RESPECT_TRUST_LEVEL is defined true in Config.pp).
 */
INLINE bool ConfigVariableCore::
is_closed() const {
  return (_flags & F_closed) != 0;
}

/**
 * Returns the minimum trust_level a prc file must demonstrate in order to
 * redefine the value for this variable.  Arguably, this should be called the
 * "mistrust level", since the larger the value, the more suspicious we are of
 * prc files.  This value is not used if is_closed() returns true, which
 * indicates no file may be trusted.
 *
 * This value only has effect in a release build (specifically, when
 * PRC_RESPECT_TRUST_LEVEL is defined true in Config.pp).
 */
INLINE int ConfigVariableCore::
get_trust_level() const {
  return (_flags & F_trust_level_mask);
}

/**
 * Returns true if the variable was indicated as "dynamic" by its constructor,
 * indicating that its name was dynamically generated, possibly from a large
 * pool, and it should not be listed along with the other variables.
 */
INLINE bool ConfigVariableCore::
is_dynamic() const {
  return (_flags & F_dynamic) != 0;
}

/**
 * Returns the default variable specified for this variable.  If the variable
 * has not yet been defined, this will return NULL.
 */
INLINE const ConfigDeclaration *ConfigVariableCore::
get_default_value() const {
  return _default_value;
}

/**
 * Marks that the variable has been "declared" by a ConfigVariable.
 */
INLINE void ConfigVariableCore::
set_used() {
  _is_used = true;
}

/**
 * Returns true if this variable's value has been shadowed by a local
 * assignment (as created via make_local_value()), or false otherwise.
 */
INLINE bool ConfigVariableCore::
has_local_value() const {
  return _local_value != nullptr;
}

/**
 * Returns the number of prc files that reference this variable.  This is not
 * exactly the same as the number of declarations; see get_reference().
 */
INLINE size_t ConfigVariableCore::
get_num_references() const {
  check_sort_declarations();
  return _declarations.size();
}

/**
 * Returns the nth declaration in a prc file that references this variable.
 * This is similar, but not identical to, get_declaration().  The difference
 * is that this will list *only* true references in a prc file, and will not
 * list default values or locally-assigned values; it also will list even the
 * untrusted files.
 */
INLINE const ConfigDeclaration *ConfigVariableCore::
get_reference(size_t n) const {
  check_sort_declarations();
  nassertr(n < _declarations.size(), nullptr);
  return _declarations[n];
}

/**
 * Returns the number of trusted prc files that reference this variable.  See
 * also get_num_references().
 */
INLINE size_t ConfigVariableCore::
get_num_trusted_references() const {
  check_sort_declarations();
  return _trusted_declarations.size();
}

/**
 * Returns the nth declaration in a trusted prc file that references this
 * variable.  This is similar, but not identical to, get_declaration().  The
 * difference is that this will list *only* true references in a prc file, and
 * will not list default values or locally-assigned values.
 *
 * This is also similar to get_reference(), except that it only lists the
 * trusted declarations, omitting the untrusted ones.
 */
INLINE const ConfigDeclaration *ConfigVariableCore::
get_trusted_reference(size_t n) const {
  check_sort_declarations();
  nassertr(n < _trusted_declarations.size(), nullptr);
  return _trusted_declarations[n];
}

/**
 * Returns the number of trusted, unique (by string value) values there exist
 * for this variable.
 */
INLINE size_t ConfigVariableCore::
get_num_unique_references() const {
  check_sort_declarations();
  return _unique_declarations.size();
}

/**
 * Returns the nth trusted, unique value for this variable.  This is similar
 * to get_trusted_reference(), except that duplicate values are removed.
 */
INLINE const ConfigDeclaration *ConfigVariableCore::
get_unique_reference(size_t n) const {
  check_sort_declarations();
  nassertr(n < _unique_declarations.size(), nullptr);
  return _unique_declarations[n];
}

/**
 * Called internally to ensure that the list of declarations is properly
 * sorted.
 */
INLINE void ConfigVariableCore::
check_sort_declarations() const {
  // First, make sure that all of the implicit .prc files have been loaded.
  // This may unsort the list by adding a bunch more declarations.
  ConfigPageManager::get_global_ptr()->load_implicit_pages();

  // Then sort the list if it needs it.
  if (!_declarations_sorted) {
    ((ConfigVariableCore *)this)->sort_declarations();
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const ConfigVariableCore &variable) {
  variable.output(out);
  return out;
}
