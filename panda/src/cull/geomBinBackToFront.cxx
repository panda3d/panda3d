// Filename: geomBinBackToFront.cxx
// Created by:  drose (13Apr00)
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

#include "cull_headers.h"
#pragma hdrstop

TypeHandle GeomBinBackToFront::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::clear_current_states
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to reset the
//               list of CullStates that were added last frame, in
//               preparation for defining a new set of CullStates
//               visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinBackToFront::
clear_current_states() {
  _node_entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::record_current_state
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to indicate
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinBackToFront::
record_current_state(GraphicsStateGuardian *gsg, CullState *cs, int,
                     CullTraverser *trav) {
  PStatTimer timer(CullTraverser::_cull_bins_btf_pcollector);

  // Get the transform matrix from the state.
  TransformAttribute *trans_attrib = NULL;
  get_attribute_into(trans_attrib, cs->get_attributes(),
                     TransformTransition::get_class_type());

  CullState::geom_const_iterator gi;
  for (gi = cs->geom_begin(); gi != cs->geom_end(); ++gi) {
    const ArcChain &arc_chain = (*gi);
    nassertv(!arc_chain.empty());
    GeomNode *node;
    DCAST_INTO_V(node, arc_chain.back()->get_child());
    nassertv(node != (GeomNode *)NULL);
    const BoundingVolume &volume = node->get_bound();

    if (!volume.is_empty() &&
        volume.is_of_type(GeometricBoundingVolume::get_class_type())) {
      const GeometricBoundingVolume *gbv;
      DCAST_INTO_V(gbv, &volume);

      LPoint3f center = gbv->get_approx_center();
      if (trans_attrib != (TransformAttribute *)NULL) {
        center = center * trans_attrib->get_matrix();
      }

      float distance = gsg->compute_distance_to(center);
      _node_entries.insert(NodeEntry(distance, cs, arc_chain, false));
    }
  }

  CullState::direct_const_iterator di;
  for (di = cs->direct_begin(); di != cs->direct_end(); ++di) {
    const ArcChain &arc_chain = (*di);
    nassertv(!arc_chain.empty());
    Node *node = arc_chain.back()->get_child();
    nassertv(node != (Node *)NULL);

    const BoundingVolume &volume = node->get_bound();
    float distance = 0.0;
    bool got_distance = false;

    if (!volume.is_empty() &&
        volume.is_of_type(GeometricBoundingVolume::get_class_type())) {
      const GeometricBoundingVolume *gbv;
      DCAST_INTO_V(gbv, &volume);

      LPoint3f center = gbv->get_approx_center();
      if (trans_attrib != (TransformAttribute *)NULL) {
        center = center * trans_attrib->get_matrix();
      }

      distance = gsg->compute_distance_to(center);
      got_distance = true;
    }

    if (!got_distance) {
      // Choose the average center of all of our children.
      LPoint3f avg(0.0, 0.0, 0.0);
      int num_points = 0;

      TypeHandle graph_type = trav->get_graph_type();
      int num_children = node->get_num_children(graph_type);
      for (int i = 0; i < num_children; i++) {
        NodeRelation *arc = node->get_child(graph_type, i);

        const BoundingVolume &volume = arc->get_bound();
        if (!volume.is_empty() &&
            volume.is_of_type(GeometricBoundingVolume::get_class_type())) {
          const GeometricBoundingVolume *gbv;
          DCAST_INTO_V(gbv, &volume);

          LPoint3f center = gbv->get_approx_center();
          avg += center;
          num_points++;
        }
      }

      if (num_points > 0) {
        avg /= (float)num_points;
        if (trans_attrib != (TransformAttribute *)NULL) {
          avg = avg * trans_attrib->get_matrix();
        }
        distance = gsg->compute_distance_to(avg);
        got_distance = true;
      }
    }

    _node_entries.insert(NodeEntry(distance, cs, arc_chain, true));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::draw
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinBackToFront::
draw(CullTraverser *trav) {
  PStatTimer timer(CullTraverser::_draw_pcollector);

  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "GeomBinBackToFront drawing " << _node_entries.size() << " entries.\n";
  }
  NodeEntries::iterator nei;
  for (nei = _node_entries.begin(); nei != _node_entries.end(); ++nei) {
    (*nei).draw(trav);
  }
}
