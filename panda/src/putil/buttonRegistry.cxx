// Filename: buttonRegistry.cxx
// Created by:  drose (01Mar00)
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
//     Function: ButtonRegistry::register_type
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool ButtonRegistry::
register_button(ButtonHandle &button_handle, const string &name,
		char ascii_equivalent) { 
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
    
    if (util_cat->is_spam()) {
      util_cat->spam()
	<< "Registering button " << name << "\n";
    }

    if (index == -1) {
      // It's not an ASCII equivalent; make up a new number.
      index = _handle_registry.size();
      _handle_registry.push_back(NULL);
    }
    
    ButtonHandle new_handle;
    new_handle._index = index;

    RegistryNode *rnode = new RegistryNode(new_handle, name);
    _handle_registry[index] = rnode;
    _name_registry[name] = rnode;

    button_handle = new_handle;
    return true;
  }

  RegistryNode *rnode = (*ri).second;
  nassertr(rnode->_name == (*ri).first, false);
  nassertr(rnode->_handle._index >= 0 && 
	   rnode->_handle._index < _handle_registry.size(), false);
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
//     Function: ButtonRegistry::find_button
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonRegistry::
find_button(const string &name) const {
  NameRegistry::const_iterator ri;
  ri = _name_registry.find(name);
  if (ri == _name_registry.end()) {
    return ButtonHandle::none();
  } else {
    return (*ri).second->_handle;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::find_button
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonRegistry::
find_button(char ascii_equivalent) const {
  if (_handle_registry[ascii_equivalent] == (RegistryNode *)NULL) {
    return ButtonHandle::none();
  }
  return _handle_registry[ascii_equivalent]->_handle;
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonRegistry::write
//       Access: Public
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
      out << "  " << (*ri).second->_name << "\n";
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
      handle._index >= _handle_registry.size()) {
    util_cat->fatal()
      << "Invalid ButtonHandle index " << handle._index 
      << "!  Is memory corrupt?\n";
    return (RegistryNode *)NULL;
  }

  return _handle_registry[handle._index];
}
