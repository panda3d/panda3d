// Filename: objectDeletor.h
// Created by:  drose (10Apr06)
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

#include "objectDeletor.h"
#include "configVariableString.h"
#include "config_express.h"

void *ObjectDeletor::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ObjectDeletor::
~ObjectDeletor() {
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::delete_object
//       Access: Public, Virtual
//  Description: Adds the pointer to the object to be deleted, along
//               with a pointer to a function that can delete it.
////////////////////////////////////////////////////////////////////
void ObjectDeletor::
delete_object(DeleteFunc *func, void *ptr) {
  // The base class functionality simply deletes the pointer
  // immediately.
  (*func)(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::flush
//       Access: Public, Virtual
//  Description: Ensures that any objects queued up for deletion have
//               been fully deleted by the time flush() returns.
////////////////////////////////////////////////////////////////////
void ObjectDeletor::
flush() {
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::register_subclass
//       Access: Public, Static
//  Description: Called at static init time as each subclass is
//               defined.  The purpose here is to consult the config
//               variable object-deletor and install the requested
//               deletor.  If the indicated deletor is the one that is
//               named by the config variable, it is installed.
////////////////////////////////////////////////////////////////////
void ObjectDeletor::
register_subclass(ObjectDeletor *deletor, const string &name) {
  ConfigVariableString object_deletor
    ("object-deletor", "",
     PRC_DESC("Specify the type of ObjectDeletor to install.  This string "
              "must match one of the existing ObjectDeletor types, and if "
              "it does not match, no error message is generated.  To "
              "determine if the ObjectDeletor was properly installed, "
              "set notify-level-express debug."));

  if (object_deletor == name) {
    if (express_cat.is_debug()) {
      express_cat.debug()
        << "Installing ObjectDeletor type " << name << "\n";
    }
    set_global_ptr(deletor);
  } else {
    delete deletor;
  }
}
