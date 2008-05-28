// Filename: configVariableManager.I
// Created by:  drose (15Oct04)
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
//     Function: ConfigVariableManager::get_num_variables
//       Access: Published
//  Description: Returns the current number of active ConfigVariableCores in
//               the world.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableManager::
get_num_variables() const {
  return _variables.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::get_variable
//       Access: Published
//  Description: Returns the nth active ConfigVariableCore in the world.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableCore *ConfigVariableManager::
get_variable(int n) const {
  nassertr(n >= 0 && n < (int)_variables.size(), (ConfigVariableCore *)NULL);
  return _variables[n];
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableManager &variableMgr) {
  variableMgr.output(out);
  return out;
}
