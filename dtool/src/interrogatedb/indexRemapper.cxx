/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexRemapper.cxx
 * @author drose
 * @date 2000-08-05
 */

#include "indexRemapper.h"


/**
 *
 */
IndexRemapper::
IndexRemapper() {
}

/**
 *
 */
IndexRemapper::
~IndexRemapper() {
}

/**
 * Removes all mappings from the object.
 */
void IndexRemapper::
clear() {
  _map_int.clear();
}

/**
 * Adds a mapping from the integer 'from' to 'to'.
 */
void IndexRemapper::
add_mapping(int from, int to) {
  _map_int[from] = to;
}

/**
 * Returns true if the given 'from' integer has been assigned a mapping, false
 * if it has not.
 */
bool IndexRemapper::
in_map(int from) const {
  return _map_int.count(from) != 0;
}

/**
 * Returns the integer that the given 'from' integer had been set to map to,
 * or the same integer if nothing had been set for it.
 */
int IndexRemapper::
map_from(int from) const {
  std::map<int, int>::const_iterator mi;
  mi = _map_int.find(from);
  if (mi == _map_int.end()) {
    return from;
  }
  return (*mi).second;
}
