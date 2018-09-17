/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonHandle.cxx
 * @author drose
 * @date 2000-03-01
 */

#include "buttonHandle.h"
#include "buttonRegistry.h"

TypeHandle ButtonHandle::_type_handle;

/**
 * Constructs a ButtonHandle with the corresponding name, which is looked up
 * in the ButtonRegistry.  This exists for the purpose of being able to
 * automatically coerce a string into a ButtonHandle; for most purposes, you
 * should use either the static KeyboardButton/MouseButton getters or
 * ButtonRegistry::register_button().
 */
ButtonHandle::
ButtonHandle(const std::string &name) {
  _index = ButtonRegistry::ptr()->get_button(name)._index;
}

/**
 * Returns the name of the button.
 */
std::string ButtonHandle::
get_name() const {
  if ((*this) == ButtonHandle::none()) {
    return "none";
  } else {
    return ButtonRegistry::ptr()->get_name(*this);
  }
}

/**
 * Returns the alias (alternate name) associated with the button, if any, or
 * ButtonHandle::none() if the button has no alias.
 *
 * Each button is allowed to have one alias, and multiple different buttons
 * can refer to the same alias.  The alias should be the more general name for
 * the button, for instance, shift is an alias for lshift, but not vice-versa.
 */
ButtonHandle ButtonHandle::
get_alias() const {
  if ((*this) == ButtonHandle::none()) {
    return ButtonHandle::none();
  } else {
    return ButtonRegistry::ptr()->get_alias(*this);
  }
}
