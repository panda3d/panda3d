/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientAnalogDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "clientAnalogDevice.h"

#include "indent.h"

TypeHandle ClientAnalogDevice::_type_handle;



/**
 * Guarantees that there is a slot in the array for the indicated index
 * number, by filling the array up to that index if necessary.
 */
void ClientAnalogDevice::
ensure_control_index(int index) {
  nassertv(index >= 0);

  _controls.reserve(index + 1);
  while ((int)_controls.size() <= index) {
    _controls.push_back(AnalogState());
  }
}

/**
 *
 */
void ClientAnalogDevice::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_device_name() << ":\n";
  write_controls(out, indent_level + 2);
}

/**
 * Writes a multi-line description of the current analog control states.
 */
void ClientAnalogDevice::
write_controls(std::ostream &out, int indent_level) const {
  bool any_controls = false;
  Controls::const_iterator ai;
  for (ai = _controls.begin(); ai != _controls.end(); ++ai) {
    const AnalogState &state = (*ai);
    if (state._known) {
      any_controls = true;

      indent(out, indent_level)
        << (int)(ai - _controls.begin()) << ". " << state._state << "\n";
    }
  }

  if (!any_controls) {
    indent(out, indent_level)
      << "(no known analog controls)\n";
  }
}
