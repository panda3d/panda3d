/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherEdge.cxx
 * @author drose
 * @date 2005-03-13
 */

#include "eggMesherEdge.h"
#include "eggMesherStrip.h"

/**
 * Removes an edge from a particular strip.
 */
void EggMesherEdge::
remove(EggMesherStrip *strip) {
  strip->_edges.remove(this);
  strip->_edges.remove(_opposite);

  _strips.remove(strip);
  _opposite->_strips.remove(strip);
}

/**
 * Reparents the edge from strip "from" to strip "to".
 */
void EggMesherEdge::
change_strip(EggMesherStrip *from, EggMesherStrip *to) {
  Strips::iterator si;

  for (si = _strips.begin(); si != _strips.end(); ++si) {
    if (*si == from) {
      *si = to;
    }
  }

  for (si = _opposite->_strips.begin();
       si != _opposite->_strips.end();
       ++si) {
    if (*si == from) {
      *si = to;
    }
  }
}

/**
 * Formats the edge for output in some sensible way.
 */
void EggMesherEdge::
output(std::ostream &out) const {
  out << "Edge [" << _vi_a << " to " << _vi_b << "], "
      << _strips.size() << " strips:";

  Strips::const_iterator si;
  for (si = _strips.begin(); si != _strips.end(); ++si) {
    out << " " << (*si)->_index;
  }

  if (_opposite!=nullptr) {
    out << " opposite "
        << _opposite->_strips.size() << " strips:";

    for (si = _opposite->_strips.begin();
         si != _opposite->_strips.end();
         ++si) {
      out << " " << (*si)->_index;
    }
  }
}
