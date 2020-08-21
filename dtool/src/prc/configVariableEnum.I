/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableEnum.I
 * @author drose
 * @date 2004-10-21
 */

/**
 *
 */
template<class EnumType>
INLINE ConfigVariableEnum<EnumType>::
ConfigVariableEnum(const std::string &name, EnumType default_value,
                   const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_enum, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_enum, std::string(), flags),
#endif
  _got_default_value(true),
  _default_value(default_value),
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(format_enum(default_value));
  _core->set_used();
}

/**
 *
 */
template<class EnumType>
INLINE ConfigVariableEnum<EnumType>::
ConfigVariableEnum(const std::string &name, const std::string &default_value,
                   const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_enum, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_enum, std::string(), flags),
#endif
  _got_default_value(true),
  _default_value(parse_string(default_value)),
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 *
 */
template<class EnumType>
INLINE ConfigVariableEnum<EnumType>::
~ConfigVariableEnum() {
}

/**
 * Reassigns the variable's local value.
 */
template<class EnumType>
INLINE void ConfigVariableEnum<EnumType>::
operator = (EnumType value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
template<class EnumType>
INLINE ConfigVariableEnum<EnumType>::
operator EnumType () const {
  return get_value();
}

/**
 * Returns the number of unique words in the variable.
 */
template<class EnumType>
INLINE size_t ConfigVariableEnum<EnumType>::
size() const {
  return get_num_words();
}

/**
 * Returns the value of the variable's nth word.
 */
template<class EnumType>
INLINE EnumType ConfigVariableEnum<EnumType>::
operator [] (size_t n) const {
  return get_word(n);
}

/**
 * Reassigns the variable's local value.
 */
template<class EnumType>
INLINE void ConfigVariableEnum<EnumType>::
set_value(EnumType value) {
  set_string_value(format_enum(value));
}

/**
 * Returns the variable's value.
 */
template<class EnumType>
INLINE EnumType ConfigVariableEnum<EnumType>::
get_value() const {
  TAU_PROFILE("EnumType ConfigVariableEnum<EnumType>::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableEnum<EnumType> *)this)->_local_modified);
    ((ConfigVariableEnum<EnumType> *)this)->_cache = (EnumType)parse_string(get_string_value());
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
template<class EnumType>
INLINE EnumType ConfigVariableEnum<EnumType>::
get_default_value() const {
  if (!_got_default_value) {
    const ConfigDeclaration *decl = ConfigVariable::get_default_value();
    if (decl != nullptr) {
      ((ConfigVariableEnum<EnumType> *)this)->_default_value = (EnumType)parse_string(decl->get_string_value());
      ((ConfigVariableEnum<EnumType> *)this)->_got_default_value = true;
    }
  }
  return _default_value;
}

/**
 * Returns the variable's nth value.
 */
template<class EnumType>
INLINE EnumType ConfigVariableEnum<EnumType>::
get_word(size_t n) const {
  return (EnumType)parse_string(get_string_word(n));
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
template<class EnumType>
INLINE void ConfigVariableEnum<EnumType>::
set_word(size_t n, EnumType value) {
  set_string_word(n, format_enum(value));
}

/**
 * Turns the string value into a value of the enumerated type by invoking its
 * predefined operator >> (istream) operator.
 */
template<class EnumType>
INLINE EnumType ConfigVariableEnum<EnumType>::
parse_string(const std::string &value) const {
  std::istringstream strm(value);
  EnumType result;
  strm >> result;
  return result;
}

/**
 * The format_enum() method assumes the enumerated type has a valid operator
 * << (ostream) defined, which balances against the operator >> (istream)
 * operator.
 */
template<class EnumType>
INLINE std::string ConfigVariableEnum<EnumType>::
format_enum(EnumType value) const {
  std::ostringstream strm;
  strm << value;
  return strm.str();
}
