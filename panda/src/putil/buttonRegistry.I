// Filename: buttonRegistry.I
// Created by:  drose (01Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "notify.h"

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::RegistryNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonRegistry::RegistryNode::
RegistryNode(ButtonHandle handle, const string &name) :
  _handle(handle), _name(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::ptr
//       Access: Public, Static
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
