// Filename: buttonRegistry.h
// Created by:  drose (01Mar00)
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

#ifndef BUTTONREGISTRY_H
#define BUTTONREGISTRY_H

#include "pandabase.h"

#include "buttonHandle.h"

#include "pvector.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonRegistry
// Description : The ButtonRegistry class maintains all the assigned
//               ButtonHandles in a given system.  There should be only
//               one ButtonRegistry class during the lifetime of the
//               application.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonRegistry {
protected:
  class EXPCL_PANDA RegistryNode {
  public:
    INLINE RegistryNode(ButtonHandle handle, ButtonHandle alias,
                        const string &name);

    ButtonHandle _handle;
    ButtonHandle _alias;
    string _name;
  };

public:
  bool register_button(ButtonHandle &button_handle, const string &name,
                       ButtonHandle alias = ButtonHandle::none(),
                       char ascii_equivalent = '\0');

PUBLISHED:
  ButtonHandle get_button(const string &name);
  ButtonHandle find_ascii_button(char ascii_equivalent) const;

  void write(ostream &out) const;

  // ptr() returns the pointer to the global ButtonRegistry object.
  INLINE static ButtonRegistry *ptr();

public:
  INLINE string get_name(ButtonHandle button) const;
  INLINE ButtonHandle get_alias(ButtonHandle button) const;

private:
  // The ButtonRegistry class should never be constructed by user code.
  // There is only one in the universe, and it constructs itself!
  ButtonRegistry();

  static void init_global_pointer();

  RegistryNode *look_up(ButtonHandle button) const;

  typedef pvector<RegistryNode *> HandleRegistry;
  HandleRegistry _handle_registry;

  typedef phash_map<string, RegistryNode *, string_hash> NameRegistry;
  NameRegistry _name_registry;

  static ButtonRegistry *_global_pointer;
};

#include "buttonRegistry.I"

#endif
