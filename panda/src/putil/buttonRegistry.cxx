// Filename: buttonRegistry.cxx
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

#include "buttonRegistry.h"
#include "config_util.h"

#include <stdio.h>

// In general, we use the util_cat->info() syntax in this file
// (instead of util_cat.info()), because much of this work is done at
// static init time, and we must use the arrow syntax to force
// initialization of the util_cat category.

ButtonRegistry *ButtonRegistry::_global_pointer = NULL;


////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::register_button
//       Access: Public
//  Description: Registers a new ButtonHandle with the indicated name,
//               and if specified, the indicated ASCII equivalent.
//               Returns true if the button was registered, or false
//               it was already registered; in either case, the new
//               ButtonHandle is loaded into the first parameter.
//
//               If the alias is not ButtonHandle::none(), it
//               indicates an alias (alternate name) for the same
//               button.  Each button is allowed to have one alias,
//               and multiple different buttons can refer to the same
//               alias.  The alias should be the more general name for
//               the button, for instance, shift is an alias for
//               lshift, but not vice-versa.
//
//               This defines a new kind of button matching the
//               indicated name.  The ButtonHandle can then be passed
//               around to devices as a button in its own right.
////////////////////////////////////////////////////////////////////
bool ButtonRegistry::
register_button(ButtonHandle &button_handle, const string &name,
                ButtonHandle alias, char ascii_equivalent) {
  NameRegistry::iterator ri;
  ri = _name_registry.find(name);

  if (ri == _name_registry.end()) {
    // The name was not already used; this is the first time this
    // button has been defined.

    int index = -1;
    if (ascii_equivalent != '\0') {
      if (_handle_registry[ascii_equivalent] == (RegistryNode *)NULL) {
        index = ascii_equivalent;
      } else {
        util_cat->error()
          << "Attempt to register multiple buttons under ASCII equivalent "
          << ascii_equivalent << "\n";
      }
    }

#ifdef NOTIFY_DEBUG
    // This code runs at static init time, so cannot use the
    // util_cat.is_spam() syntax.
    if (util_cat->is_spam()) {
      util_cat->spam()
        << "Registering button " << name << "\n";
    }
#endif

    if (index == -1) {
      // It's not an ASCII equivalent; make up a new number.
      index = _handle_registry.size();
      _handle_registry.push_back(NULL);
    }

    ButtonHandle new_handle;
    new_handle._index = index;

    RegistryNode *rnode = new RegistryNode(new_handle, alias, name);
    _handle_registry[index] = rnode;
    _name_registry[name] = rnode;

    button_handle = new_handle;
    return true;
  }

  RegistryNode *rnode = (*ri).second;
  nassertr(rnode->_name == (*ri).first, false);
  nassertr(rnode->_handle._index >= 0 &&
           rnode->_handle._index < (int)_handle_registry.size(), false);
  nassertr(_handle_registry[rnode->_handle._index] == rnode, false);
  nassertr(rnode->_handle._index != 0, false);

  if (button_handle != rnode->_handle) {
    // Hmm, we seem to have a contradictory button registration!
    util_cat->warning()
      << "Attempt to register button " << name << " more than once!\n";

    button_handle = rnode->_handle;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::get_button
//       Access: Published
//  Description: Finds a ButtonHandle in the registry matching the
//               indicated name.  If there is no such ButtonHandle,
//               registers a new one and returns it.
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonRegistry::
get_button(const string &name) {
  NameRegistry::const_iterator ri;
  ri = _name_registry.find(name);

  if (ri != _name_registry.end()) {
    return (*ri).second->_handle;
  }

  ButtonHandle button;
  register_button(button, name);
  return button;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::find_button
//       Access: Published
//  Description: Finds a ButtonHandle in the registry matching the
//               indicated name.  If there is no such ButtonHandle,
//               returns ButtonHandle::none().
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonRegistry::
find_button(const string &name) {
  NameRegistry::const_iterator ri;
  ri = _name_registry.find(name);

  if (ri != _name_registry.end()) {
    return (*ri).second->_handle;
  }

  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::find_ascii_button
//       Access: Published
//  Description: Finds a ButtonHandle in the registry matching the
//               indicated ASCII equivalent character.  If there is no
//               such ButtonHandle, returns ButtonHandle::none().
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonRegistry::
find_ascii_button(char ascii_equivalent) const {
  if (_handle_registry[ascii_equivalent] == (RegistryNode *)NULL) {
    return ButtonHandle::none();
  }
  return _handle_registry[ascii_equivalent]->_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonRegistry::
write(ostream &out) const {
  out << "ASCII equivalents:\n";
  for (int i = 1; i < 128; i++) {
    if (_handle_registry[i] != (RegistryNode *)NULL) {
      char hex[12];
      sprintf(hex, "%02x", (unsigned int)i);
      nassertv(strlen(hex) < 12);

      out << "  " << hex << " " << _handle_registry[i]->_name << "\n";
    }
  }

  out << "\nOther buttons:\n";
  NameRegistry::const_iterator ri;
  for (ri = _name_registry.begin(); ri != _name_registry.end(); ++ri) {
    if (!(*ri).second->_handle.has_ascii_equivalent()) {
      out << "  " << (*ri).second->_name;
      if ((*ri).second->_alias != ButtonHandle::none()) {
        out << " (alias " << (*ri).second->_alias << ")";
      }
      out << "\n";
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
ButtonRegistry::
ButtonRegistry() {
  // We'll start by filling up the handle_registry with 128 entries
  // for ButtonHandle::none(), as well as for all the ASCII
  // equivalents.

  _handle_registry.reserve(128);
  int i;
  for (i = 0; i < 128; i++) {
    _handle_registry.push_back(NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::init_global_pointer
//       Access: Private, Static
//  Description: Constructs the ButtonRegistry object for the first
//               time.
////////////////////////////////////////////////////////////////////
void ButtonRegistry::
init_global_pointer() {
  _global_pointer = new ButtonRegistry;
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::look_up
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
ButtonRegistry::RegistryNode *ButtonRegistry::
look_up(ButtonHandle handle) const {
  nassertr(handle._index != 0, NULL);

  if (handle._index < 0 ||
      handle._index >= (int)_handle_registry.size()) {
    util_cat->fatal()
      << "Invalid ButtonHandle index " << handle._index
      << "!  Is memory corrupt?\n";
    return (RegistryNode *)NULL;
  }

  return _handle_registry[handle._index];
}
