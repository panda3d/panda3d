/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableBase.cxx
 * @author drose
 * @date 2004-10-21
 */

#include "configVariableBase.h"
#include "config_prc.h"

ConfigVariableBase::Unconstructed *ConfigVariableBase::_unconstructed;

/**
 * This constructor is only intended to be called from a specialized
 * ConfigVariableFoo derived class.
 */
ConfigVariableBase::
ConfigVariableBase(const std::string &name,
                   ConfigVariableBase::ValueType value_type,
                   const std::string &description, int flags) :
  _core(ConfigVariableManager::get_global_ptr()->make_variable(name))
{
#ifndef NDEBUG
  if (was_unconstructed()) {
    prc_cat->error()
      << "Late constructing " << this << ": " << name << "\n";
  }
#endif  // NDEBUG

  if (value_type != VT_undefined) {
    _core->set_value_type(value_type);
  }
#ifdef PRC_SAVE_DESCRIPTIONS
  if (!description.empty()) {
    _core->set_description(description);
  }
#endif  // PRC_SAVE_DESCRIPTIONS
  if (flags != 0) {
    _core->set_flags(flags);
  }
}

/**
 * Records that this config variable was referenced before it was constructed
 * (presumably a static-init ordering issue).  This is used to print a useful
 * error message later, when the constructor is actually called (and we then
 * know what the name of the variable is).
 */
void ConfigVariableBase::
record_unconstructed() const {
#ifndef NDEBUG
  if (_unconstructed == nullptr) {
    _unconstructed = new Unconstructed;
  }
  _unconstructed->insert(this);
#endif
}

/**
 * Returns true if record_unconstructed() was ever called on this pointer,
 * false otherwise.
 */
bool ConfigVariableBase::
was_unconstructed() const {
#ifndef NDEBUG
  if (_unconstructed != nullptr) {
    Unconstructed::const_iterator ui = _unconstructed->find(this);
    if (ui != _unconstructed->end()) {
      return true;
    }
  }
#endif
  return false;
}
