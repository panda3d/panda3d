// Filename: geomBinFixed.cxx
// Created by:  drose (14Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "geomBinFixed.h"
#include "cullTraverser.h"

#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>
#include <transformTransition.h>
#include <transformAttribute.h>
#include <geometricBoundingVolume.h>
#include <directRenderTraverser.h>
#include <pStatTimer.h>

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
//  Description: Called each frame by the CullTraverser to indicated
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinFixed::
record_current_state(GraphicsStateGuardian *, CullState *cs,
		     int draw_order, CullTraverser *) {
  // Get the transform matrix from the state.
  LMatrix4f mat;

  TransformAttribute *trans_attrib = NULL;
  get_attribute_into(trans_attrib, cs->get_attributes(),
		     TransformTransition::get_class_type());

  CullState::geom_const_iterator gi;
  for (gi = cs->geom_begin(); gi != cs->geom_end(); ++gi) {
    GeomNode *node = (*gi);
    nassertv(node != (GeomNode *)NULL);
    _node_entries.insert(NodeEntry(draw_order, cs, node, false));
  }

  CullState::direct_const_iterator di;
  for (di = cs->direct_begin(); di != cs->direct_end(); ++di) {
    Node *node = (*di);
    nassertv(node != (Node *)NULL);
    _node_entries.insert(NodeEntry(draw_order, cs, node, true));
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
