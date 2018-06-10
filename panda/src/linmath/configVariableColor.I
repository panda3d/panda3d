/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableColor.I
 * @author rdb
 * @date 2014-02-02
 */

/**
 *
 */
INLINE ConfigVariableColor::
ConfigVariableColor(const std::string &name) :
  ConfigVariable(name, VT_color),
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableColor::
ConfigVariableColor(const std::string &name, const LColor &default_value,
                    const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_color, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_color, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  set_default_value(default_value);
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableColor::
ConfigVariableColor(const std::string &name, const std::string &default_value,
                    const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_color, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_color, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableColor::
operator = (const LColor &value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE ConfigVariableColor::
operator const LColor & () const {
  return get_value();
}

/**
 * Returns the value of the color's nth component (which is not necessarily
 * the same thing as the variable's nth word).
 */
INLINE PN_stdfloat ConfigVariableColor::
operator [] (int n) const {
  return get_value()[n];
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableColor::
set_value(const LColor &color) {
  set_string_value("");
  set_double_word(0, color[0]);
  set_double_word(1, color[1]);
  set_double_word(2, color[2]);
  set_double_word(3, color[3]);
}

/**
 * Returns the variable's value.
 */
INLINE const LColor &ConfigVariableColor::
get_value() const {
  TAU_PROFILE("const LColor &ConfigVariableColor::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(_local_modified);

    switch (get_num_words()) {
    case 1:
      _cache.set((PN_stdfloat)get_double_word(0), (PN_stdfloat)get_double_word(0),
                 (PN_stdfloat)get_double_word(0), 1);
      break;

    case 2:
      _cache.set((PN_stdfloat)get_double_word(0), (PN_stdfloat)get_double_word(0),
                 (PN_stdfloat)get_double_word(0), (PN_stdfloat)get_double_word(1));
      break;

    case 3:
      _cache.set((PN_stdfloat)get_double_word(0), (PN_stdfloat)get_double_word(1),
                 (PN_stdfloat)get_double_word(2), 1);
      break;

    case 4:
      _cache.set((PN_stdfloat)get_double_word(0), (PN_stdfloat)get_double_word(1),
                 (PN_stdfloat)get_double_word(2), (PN_stdfloat)get_double_word(3));
      break;

    default:
      prc_cat->warning()
        << "Invalid color value for ConfigVariable "
        << get_name() << ": " << get_string_value() << "\n";
    }
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
INLINE LColor ConfigVariableColor::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    switch (decl->get_num_words()) {
    case 1:
      return LColor((PN_stdfloat)decl->get_double_word(0), (PN_stdfloat)decl->get_double_word(0),
                    (PN_stdfloat)decl->get_double_word(0), 1);
    case 2:
      return LColor((PN_stdfloat)decl->get_double_word(0), (PN_stdfloat)decl->get_double_word(0),
                    (PN_stdfloat)decl->get_double_word(0), (PN_stdfloat)decl->get_double_word(1));
    case 3:
      return LColor((PN_stdfloat)decl->get_double_word(0), (PN_stdfloat)decl->get_double_word(1),
                    (PN_stdfloat)decl->get_double_word(2), 1);
    case 4:
      return LColor((PN_stdfloat)decl->get_double_word(0), (PN_stdfloat)decl->get_double_word(1),
                    (PN_stdfloat)decl->get_double_word(2), (PN_stdfloat)decl->get_double_word(3));
    default:
      prc_cat->warning()
        << "Invalid default color value for ConfigVariable "
        << get_name() << ": " << decl->get_string_value() << "\n";
    }
  }
  return LColor(0, 0, 0, 1);
}
