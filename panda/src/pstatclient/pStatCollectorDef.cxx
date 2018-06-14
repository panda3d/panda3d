/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollectorDef.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "pStatCollectorDef.h"

#include "datagram.h"
#include "datagramIterator.h"


/**
 *
 */
PStatCollectorDef::
PStatCollectorDef() {
  _index = 0;
  _parent_index = 0;
  _suggested_color.r = 0.0;
  _suggested_color.g = 0.0;
  _suggested_color.b = 0.0;
  _sort = -1;
  _suggested_scale = 0.0;
  _factor = 1.0;
  _is_active = true;
  _active_explicitly_set = false;
}

/**
 *
 */
PStatCollectorDef::
PStatCollectorDef(int index, const std::string &name) :
  _index(index),
  _name(name)
{
  _parent_index = 0;
  _suggested_color.r = 0.0;
  _suggested_color.g = 0.0;
  _suggested_color.b = 0.0;
  _sort = -1;
  _suggested_scale = 0.0;
  _factor = 1.0;
  _is_active = true;
  _active_explicitly_set = false;
}

/**
 * This is normally called only by the PStatClient when the new
 * PStatCollectorDef is created; it sets the parent of the CollectorDef and
 * inherits whatever properties are appropriate.
 */
void PStatCollectorDef::
set_parent(const PStatCollectorDef &parent) {
  _parent_index = parent._index;
  _level_units = parent._level_units;
  _suggested_scale = parent._suggested_scale;
  _factor = parent._factor;
  _is_active = parent._is_active;
  _active_explicitly_set = parent._active_explicitly_set;
}

/**
 * Writes the definition of the collectorDef to the datagram.
 */
void PStatCollectorDef::
write_datagram(Datagram &destination) const {
  destination.add_int16(_index);
  destination.add_string(_name);
  destination.add_int16(_parent_index);
  destination.add_float32(_suggested_color.r);
  destination.add_float32(_suggested_color.g);
  destination.add_float32(_suggested_color.b);
  destination.add_int16(_sort);
  destination.add_string(_level_units);
  destination.add_float32(_suggested_scale);
  destination.add_float32(_factor);
}

/**
 * Extracts the collectorDef definition from the datagram.
 */
void PStatCollectorDef::
read_datagram(DatagramIterator &source, PStatClientVersion *) {
  _index = source.get_int16();
  _name = source.get_string();
  _parent_index = source.get_int16();
  _suggested_color.r = source.get_float32();
  _suggested_color.g = source.get_float32();
  _suggested_color.b = source.get_float32();
  _sort = source.get_int16();
  _level_units = source.get_string();
  _suggested_scale = source.get_float32();
  _factor = source.get_float32();
}
