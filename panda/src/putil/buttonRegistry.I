// Filename: buttonRegistry.I
// Created by:  drose (01Mar00)
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

#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::RegistryNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonRegistry::RegistryNode::
RegistryNode(ButtonHandle handle, ButtonHandle alias, const string &name) :
  _handle(handle), _alias(alias), _name(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::ptr
//       Access: Published, Static
//  Description: Returns the pointer to the global ButtonRegistry
//               object.
////////////////////////////////////////////////////////////////////
INLINE ButtonRegistry *ButtonRegistry::
ptr() {
  if (_global_pointer == (ButtonRegistry *)NULL) {
    init_global_pointer();
  }
  return _global_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::get_name
//       Access: Public
//  Description: Returns the name of the indicated button.
////////////////////////////////////////////////////////////////////
INLINE string ButtonRegistry::
get_name(ButtonHandle button) const {
  RegistryNode *rnode = look_up(button);
  nassertr(rnode != (RegistryNode *)NULL, "");
  return rnode->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::get_alias
//       Access: Public
//  Description: Returns the alias for the indicated button, or
//               ButtonHandle::none() if the button has no specified
//               alias.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonRegistry::
get_alias(ButtonHandle button) const {
  RegistryNode *rnode = look_up(button);
  nassertr(rnode != (RegistryNode *)NULL, ButtonHandle::none());
  return rnode->_alias;
}
