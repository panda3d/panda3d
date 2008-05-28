// Filename: configVariableList.I
// Created by:  drose (20Oct04)
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
//     Function: ConfigVariableList::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableList::
~ConfigVariableList() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableList::
ConfigVariableList(const string &name, 
                   const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariableBase(name, VT_list, description, flags)
#else
  ConfigVariableBase(name, VT_list, string(), flags)
#endif
{
  // A list variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from
  // complaining should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_num_values
//       Access: Published
//  Description: Returns the number of values in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
get_num_values() const {
  nassertr(_core != (ConfigVariableCore *)NULL, 0);
  return _core->get_num_trusted_references();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_string_value
//       Access: Published
//  Description: Returns the nth value of the variable.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
get_string_value(int n) const {
  nassertr(_core != (ConfigVariableCore *)NULL, string());
  const ConfigDeclaration *decl = _core->get_trusted_reference(n);
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_value();
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_num_unique_values
//       Access: Published
//  Description: Returns the number of unique values in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
get_num_unique_values() const {
  nassertr(_core != (ConfigVariableCore *)NULL, 0);
  return _core->get_num_unique_references();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_unique_value
//       Access: Published
//  Description: Returns the nth unique value of the variable.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
get_unique_value(int n) const {
  nassertr(_core != (ConfigVariableCore *)NULL, string());
  const ConfigDeclaration *decl = _core->get_unique_reference(n);
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_value();
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::size()
//       Access: Published
//  Description: Returns the number of unique values of the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
size() const {
  return get_num_unique_values();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::operator []
//       Access: Published
//  Description: Returns the nth unique value of the variable.  Note
//               that the indexing operator returns the list of unique
//               values, and so the maximum range is
//               get_num_unique_values().
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
operator [] (int n) const {
  return get_unique_value(n);
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableList &variable) {
  variable.output(out);
  return out;
}
