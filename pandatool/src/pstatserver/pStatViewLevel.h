/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatViewLevel.h
 * @author drose
 * @date 2000-07-11
 */

#ifndef PSTATVIEWLEVEL_H
#define PSTATVIEWLEVEL_H

#include "pandatoolbase.h"

#include "pvector.h"

class PStatClientData;

/**
 * This is a single level value, or band of color, within a View.
 *
 * It generally indicates either the elapsed time, or the "level" value, for a
 * particular Collector within a given frame for a particular thread.
 */
class PStatViewLevel {
public:
  INLINE int get_collector() const;
  INLINE double get_value_alone() const;
  double get_net_value() const;

  void sort_children(const PStatClientData *client_data);

  int get_num_children() const;
  const PStatViewLevel *get_child(int n) const;

private:
  int _collector;
  double _value_alone;
  PStatViewLevel *_parent;

  typedef pvector<PStatViewLevel *> Children;
  Children _children;

  friend class PStatView;
};

#include "pStatViewLevel.I"

#endif
