// Filename: eggMesherEdge.cxx
// Created by:  drose (13Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "eggMesherEdge.h"

////////////////////////////////////////////////////////////////////
//     Function: EggMesherEdge::remove
//       Access: Public
//  Description: Removes an edge from a particular strip.
////////////////////////////////////////////////////////////////////
void EggMesherEdge::
remove(EggMesherStrip *strip) {
  strip->_edges.remove(this);
  strip->_edges.remove(_opposite);

  _strips.remove(strip);
  _opposite->_strips.remove(strip);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherEdge::change_strip
//       Access: Public
//  Description: Reparents the edge from strip "from" to strip "to".
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggMesherEdge::output
//       Access: Public
//  Description: Formats the edge for output in some sensible way.
////////////////////////////////////////////////////////////////////
void EggMesherEdge::
output(ostream &out) const {
  out << "Edge [" << _vi_a << " to " << _vi_b << "], "
      << _strips.size() << " strips:";

  Strips::const_iterator si;
  for (si = _strips.begin(); si != _strips.end(); ++si) {
    out << " " << (*si)->_index;
  }

  if (_opposite!=NULL) {
    out << " opposite "
        << _opposite->_strips.size() << " strips:";

    for (si = _opposite->_strips.begin();
         si != _opposite->_strips.end();
         ++si) {
      out << " " << (*si)->_index;
    }
  }
}
