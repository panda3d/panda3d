// Filename: configVariableManager.I
// Created by:  drose (15Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  nassertr(n >= 0 && n < (int)_variables.size(), NULL);
  return _variables[n];
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableManager &variableMgr) {
  variableMgr.output(out);
  return out;
}
