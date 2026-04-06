/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonRegistry.h
 * @author drose
 * @date 2000-03-01
 */

#ifndef BUTTONREGISTRY_H
#define BUTTONREGISTRY_H

#include "pandabase.h"

#include "buttonHandle.h"

#include "pvector.h"
#include "pmap.h"

/**
 * The ButtonRegistry class maintains all the assigned ButtonHandles in a
 * given system.  There should be only one ButtonRegistry class during the
 * lifetime of the application.
 */
class EXPCL_PANDA_PUTIL ButtonRegistry {
protected:
  class EXPCL_PANDA_PUTIL RegistryNode {
  public:
    INLINE RegistryNode(ButtonHandle handle, ButtonHandle alias,
                        const std::string &name);

    ButtonHandle _handle;
    ButtonHandle _alias;
    std::string _name;
  };

public:
  bool register_button(ButtonHandle &button_handle, const std::string &name,
                       ButtonHandle alias = ButtonHandle::none(),
                       char ascii_equivalent = '\0');

PUBLISHED:
  ButtonHandle get_button(const std::string &name);
  ButtonHandle find_button(const std::string &name);
  ButtonHandle find_ascii_button(char ascii_equivalent) const;

  void write(std::ostream &out) const;

  // ptr() returns the pointer to the global ButtonRegistry object.
  INLINE static ButtonRegistry *ptr();

public:
  INLINE std::string get_name(ButtonHandle button) const;
  INLINE ButtonHandle get_alias(ButtonHandle button) const;

private:
  // The ButtonRegistry class should never be constructed by user code.  There
  // is only one in the universe, and it constructs itself!
  ButtonRegistry();

  static void init_global_pointer();

  RegistryNode *look_up(ButtonHandle button) const;

  typedef pvector<RegistryNode *> HandleRegistry;
  HandleRegistry _handle_registry;

  typedef pmap<std::string, RegistryNode *> NameRegistry;
  NameRegistry _name_registry;

  static ButtonRegistry *_global_pointer;
};

#include "buttonRegistry.I"

#endif
