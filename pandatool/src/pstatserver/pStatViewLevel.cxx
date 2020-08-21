/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatViewLevel.cxx
 * @author drose
 * @date 2000-07-11
 */

#include "pStatViewLevel.h"
#include "pStatClientData.h"

#include "pStatCollectorDef.h"
#include "pnotify.h"

#include <algorithm>

/**
 * Returns the total level value (or elapsed time) represented by this
 * Collector, including all values in its child Collectors.
 */
double PStatViewLevel::
get_net_value() const {
  double net = _value_alone;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    net += (*ci)->get_net_value();
  }

  return net;
}


// STL function object for sorting children in order by the collector's sort
// index, used in sort_children(), below.
class SortCollectorLevels {
public:
  SortCollectorLevels(const PStatClientData *client_data) :
    _client_data(client_data) {
  }
  bool operator () (const PStatViewLevel *a, const PStatViewLevel *b) const {
    return
      _client_data->get_collector_def(a->get_collector())._sort >
      _client_data->get_collector_def(b->get_collector())._sort;
  }
  const PStatClientData *_client_data;
};

/**
 * Sorts the children of this view level into order as specified by the
 * client's sort index.
 */
void PStatViewLevel::
sort_children(const PStatClientData *client_data) {
  SortCollectorLevels sort_levels(client_data);

  sort(_children.begin(), _children.end(), sort_levels);
}

/**
 * Returns the number of children of this Level/Collector.  These are the
 * Collectors whose value is considered to be part of the total value of this
 * level's Collector.
 */
int PStatViewLevel::
get_num_children() const {
  return _children.size();
}

/**
 * Returns the nth child of this Level/Collector.
 */
const PStatViewLevel *PStatViewLevel::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), nullptr);
  return _children[n];
}
