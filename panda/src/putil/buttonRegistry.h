// Filename: buttonRegistry.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BUTTONREGISTRY_H
#define BUTTONREGISTRY_H

#include <pandabase.h>

#include "buttonHandle.h"

#include <vector>
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////
// 	 Class : ButtonRegistry
// Description : The ButtonRegistry class maintains all the assigned
//               ButtonHandles in a given system.  There should be only
//               one ButtonRegistry class during the lifetime of the
//               application.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonRegistry {
protected:
  class EXPCL_PANDA RegistryNode {
  public:
    INLINE RegistryNode(ButtonHandle handle, const string &name);

    ButtonHandle _handle;
    string _name;
  };

public:
  bool register_button(ButtonHandle &button_handle, const string &name,
		       char ascii_equivalent = '\0');

  ButtonHandle find_button(const string &name) const;
  ButtonHandle find_button(char ascii_equivalent) const;

  INLINE string get_name(ButtonHandle button) const;

  void write(ostream &out) const;

  // ptr() returns the pointer to the global ButtonRegistry object.
  INLINE static ButtonRegistry *ptr();

private:
  // The ButtonRegistry class should never be constructed by user code.
  // There is only one in the universe, and it constructs itself!
  ButtonRegistry();

  static void init_global_pointer();

  RegistryNode *look_up(ButtonHandle button) const;

  typedef vector<RegistryNode *> HandleRegistry;
  HandleRegistry _handle_registry;

  typedef map<string, RegistryNode *> NameRegistry;
  NameRegistry _name_registry;

  static ButtonRegistry *_global_pointer;
};

#include "buttonRegistry.I"

#endif
