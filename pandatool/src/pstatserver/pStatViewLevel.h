// Filename: pStatViewLevel.h
// Created by:  drose (11Jul00)
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

#ifndef PSTATVIEWLEVEL_H
#define PSTATVIEWLEVEL_H

#include "pandatoolbase.h"

#include "pvector.h"

class PStatClientData;

////////////////////////////////////////////////////////////////////
//       Class : PStatViewLevel
// Description : This is a single level value, or band of color,
//               within a View.
//
//               It generally indicates either the elapsed time, or
//               the "level" value, for a particular Collector within
//               a given frame for a particular thread.
////////////////////////////////////////////////////////////////////
class PStatViewLevel {
public:
  INLINE int get_collector() const;
  INLINE float get_value_alone() const;
  float get_net_value() const;

  void sort_children(const PStatClientData *client_data);

  int get_num_children() const;
  const PStatViewLevel *get_child(int n) const;

private:
  int _collector;
  float _value_alone;
  PStatViewLevel *_parent;

  typedef pvector<PStatViewLevel *> Children;
  Children _children;

  friend class PStatView;
};

#include "pStatViewLevel.I"

#endif
