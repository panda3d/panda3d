// Filename: pStatCollectorDef.h
// Created by:  drose (09Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PSTATCOLLECTORDEF_H
#define PSTATCOLLECTORDEF_H

#include "pandabase.h"

#include "luse.h"

class Datagram;
class DatagramIterator;
class PStatClient;
class PStatClientVersion;

////////////////////////////////////////////////////////////////////
//       Class : PStatCollectorDef
// Description : Defines the details about the Collectors: the name,
//               the suggested color, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatCollectorDef {
public:
  PStatCollectorDef();
  PStatCollectorDef(int index, const string &name);
  void set_parent(const PStatCollectorDef &parent);

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source, PStatClientVersion *version);

  int _index;
  string _name;
  int _parent_index;
  RGBColorf _suggested_color;
  int _sort;
  string _level_units;
  float _suggested_scale;
  float _factor;
  bool _is_active;
  bool _active_explicitly_set;
};

#endif

