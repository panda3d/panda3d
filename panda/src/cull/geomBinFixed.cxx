// Filename: geomBinFixed.cxx
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


#include "geomBinFixed.h"
#include "cullTraverser.h"

#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>
#include <transformTransition.h>
#include <transformAttribute.h>
#include <geometricBoundingVolume.h>
#include <pStatTimer.h>

#include <directRenderTraverser.h>

TypeHandle GeomBinFixed::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::clear_current_states
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to reset the
//               list of CullStates that were added last frame, in
//               preparation for defining a new set of CullStates
//               visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinFixed::
clear_current_states() {
  _node_entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::record_current_state
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to indicate
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinFixed::
record_current_state(GraphicsStateGuardian *, CullState *cs,
                     int draw_order, CullTraverser *) {
  PStatTimer timer(CullTraverser::_cull_bins_fixed_pcollector);

  // Get the transform matrix from the state.
  TransformAttribute *trans_attrib = NULL;
  get_attribute_into(trans_attrib, cs->get_attributes(),
                     TransformTransition::get_class_type());

  CullState::geom_const_iterator gi;
  for (gi = cs->geom_begin(); gi != cs->geom_end(); ++gi) {
    const ArcChain &arc_chain = (*gi);
    _node_entries.insert(NodeEntry(draw_order, cs, arc_chain, false));
  }

  CullState::direct_const_iterator di;
  for (di = cs->direct_begin(); di != cs->direct_end(); ++di) {
    const ArcChain &arc_chain = (*di);
    _node_entries.insert(NodeEntry(draw_order, cs, arc_chain, true));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinFixed::draw
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinFixed::
draw(CullTraverser *trav) {
  PStatTimer timer(CullTraverser::_draw_pcollector);

  NodeEntries::iterator nei;
  for (nei = _node_entries.begin(); nei != _node_entries.end(); ++nei) {
    (*nei).draw(trav);
  }
}
