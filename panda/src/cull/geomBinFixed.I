// Filename: geomBinFixed.I
// Created by:  drose (14Apr00)
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

#include "cullTraverser.h"

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::NodeEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomBinFixed::NodeEntry::
NodeEntry(int draw_order, const PT(CullState) &state,
          const ArcChain &arc_chain, bool is_direct) :
  _draw_order(draw_order),
  _state(state),
  _arc_chain(arc_chain),
  _is_direct(is_direct)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::NodeEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomBinFixed::NodeEntry::
NodeEntry(const GeomBinFixed::NodeEntry &copy) :
  _draw_order(copy._draw_order),
  _state(copy._state),
  _arc_chain(copy._arc_chain),
  _is_direct(copy._is_direct)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::NodeEntry::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GeomBinFixed::NodeEntry::
operator = (const NodeEntry &copy) {
  _draw_order = copy._draw_order;
  _state = copy._state;
  _arc_chain = copy._arc_chain;
  _is_direct = copy._is_direct;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::NodeEntry::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomBinFixed::NodeEntry::
operator < (const NodeEntry &other) const {
  return _draw_order < other._draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::NodeEntry::draw
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GeomBinFixed::NodeEntry::
draw(CullTraverser *trav) const {
  nassertv(!_arc_chain.empty());
  if (_is_direct) {
    trav->draw_direct(_arc_chain, AllAttributesWrapper(_state->get_attributes()));
  } else {
    trav->draw_geom(_arc_chain, AllAttributesWrapper(_state->get_attributes()));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomBinFixed::
GeomBinFixed(const string &name) :
  GeomBin(name)
{
}
