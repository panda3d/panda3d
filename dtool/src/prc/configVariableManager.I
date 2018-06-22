/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableManager.I
 * @author drose
 * @date 2004-10-15
 */

/**
 * Returns the current number of active ConfigVariableCores in the world.
 */
INLINE size_t ConfigVariableManager::
get_num_variables() const {
  return _variables.size();
}

/**
 * Returns the nth active ConfigVariableCore in the world.
 */
INLINE ConfigVariableCore *ConfigVariableManager::
get_variable(size_t n) const {
  nassertr(n < _variables.size(), nullptr);
  return _variables[n];
}

INLINE std::ostream &
operator << (std::ostream &out, const ConfigVariableManager &variableMgr) {
  variableMgr.output(out);
  return out;
}
