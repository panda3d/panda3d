/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonRegistry.I
 * @author drose
 * @date 2000-03-01
 */

#include "pnotify.h"

/**
 *
 */
INLINE ButtonRegistry::RegistryNode::
RegistryNode(ButtonHandle handle, ButtonHandle alias, const std::string &name) :
  _handle(handle), _alias(alias), _name(name)
{
}

/**
 * Returns the pointer to the global ButtonRegistry object.
 */
INLINE ButtonRegistry *ButtonRegistry::
ptr() {
  if (_global_pointer == nullptr) {
    init_global_pointer();
  }
  return _global_pointer;
}

/**
 * Returns the name of the indicated button.
 */
INLINE std::string ButtonRegistry::
get_name(ButtonHandle button) const {
  RegistryNode *rnode = look_up(button);
  nassertr(rnode != nullptr, "");
  return rnode->_name;
}

/**
 * Returns the alias for the indicated button, or ButtonHandle::none() if the
 * button has no specified alias.
 */
INLINE ButtonHandle ButtonRegistry::
get_alias(ButtonHandle button) const {
  RegistryNode *rnode = look_up(button);
  nassertr(rnode != nullptr, ButtonHandle::none());
  return rnode->_alias;
}
