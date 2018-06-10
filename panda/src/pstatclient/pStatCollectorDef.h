/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollectorDef.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATCOLLECTORDEF_H
#define PSTATCOLLECTORDEF_H

#include "pandabase.h"
#include "numeric_types.h"

class Datagram;
class DatagramIterator;
class PStatClient;
class PStatClientVersion;

/**
 * Defines the details about the Collectors: the name, the suggested color,
 * etc.
 */
class EXPCL_PANDA_PSTATCLIENT PStatCollectorDef {
public:
  PStatCollectorDef();
  PStatCollectorDef(int index, const std::string &name);
  void set_parent(const PStatCollectorDef &parent);

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source, PStatClientVersion *version);

  struct ColorDef {
    float r, g, b;
  };

  int _index;
  std::string _name;
  int _parent_index;
  ColorDef _suggested_color;
  int _sort;
  std::string _level_units;
  double _suggested_scale;
  double _factor;
  bool _is_active;
  bool _active_explicitly_set;
};

#endif
