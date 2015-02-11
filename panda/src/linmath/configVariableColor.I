// Filename: configVariableColor.I
// Created by:  rdb (02Feb14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableColor::
ConfigVariableColor(const string &name) :
  ConfigVariable(name, VT_color),
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableColor::
ConfigVariableColor(const string &name, const LColor &default_value,
                    const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_color, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_color, string(), flags),
#endif
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableColor::
ConfigVariableColor(const string &name, const string &default_value,
                    const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_color, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_color, string(), flags),
#endif
  _local_modified(initial_invalid_cache()),
  _cache(0, 0, 0, 1)
{
  _core->set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableColor::
operator = (const LColor &value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableColor::
operator const LColor & () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::operator []
//       Access: Published
//  Description: Returns the value of the color's nth component
//               (which is not necessarily the same thing as the
//               variable's nth word).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ConfigVariableColor::
operator [] (int n) const {
  return get_value()[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableColor::
set_value(const LColor &color) {
  set_string_value("");
  set_double_word(0, color[0]);
  set_double_word(1, color[1]);
  set_double_word(2, color[2]);
  set_double_word(3, color[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE const LColor &ConfigVariableColor::
get_value() const {
  TAU_PROFILE("const LColor &ConfigVariableColor::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(_local_modified);

    switch (get_num_words()) {
    case 1:
      _cache.set(get_double_word(0), get_double_word(0), get_double_word(0), 1);
      break;

    case 2:
      _cache.set(get_double_word(0), get_double_word(0), get_double_word(0), get_double_word(1));
      break;

    case 3:
      _cache.set(get_double_word(0), get_double_word(1), get_double_word(2), 1);
      break;

    case 4:
      _cache.set(get_double_word(0), get_double_word(1), get_double_word(2), get_double_word(3));
      break;

    default:
      prc_cat->warning()
        << "Invalid color value for ConfigVariable "
        << get_name() << ": " << get_string_value() << "\n";
    }
  }
  return _cache;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE LColor ConfigVariableColor::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    switch (decl->get_num_words()) {
    case 1:
      return LColor(decl->get_double_word(0), decl->get_double_word(0),
                    decl->get_double_word(0), 1);
    case 2:
      return LColor(decl->get_double_word(0), decl->get_double_word(0),
                    decl->get_double_word(0), decl->get_double_word(1));
    case 3:
      return LColor(decl->get_double_word(0), decl->get_double_word(1),
                    decl->get_double_word(2), 1);
    case 4:
      return LColor(decl->get_double_word(0), decl->get_double_word(1),
                    decl->get_double_word(2), decl->get_double_word(3));
    default:
      prc_cat->warning()
        << "Invalid default color value for ConfigVariable "
        << get_name() << ": " << decl->get_string_value() << "\n";
    }
  }
  return LColor(0, 0, 0, 1);
}
