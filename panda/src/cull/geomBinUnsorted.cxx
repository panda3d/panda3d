// Filename: geomBinUnsorted.cxx
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

#include "geomBinUnsorted.h"
#include "cullTraverser.h"

#include <indent.h>
#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>
#include <pStatTimer.h>

TypeHandle GeomBinUnsorted::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GeomBinUnsorted::
~GeomBinUnsorted() {
  clear_current_states();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::clear_current_states
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to reset the
//               list of CullStates that were added last frame, in
//               preparation for defining a new set of CullStates
//               visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinUnsorted::
clear_current_states() {
  CullStates::iterator csi;
  for (csi = _cull_states.begin(); csi != _cull_states.end(); ++csi) {
    disclaim_cull_state(*csi);
  }

  _cull_states.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::record_current_state
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to indicate
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinUnsorted::
record_current_state(GraphicsStateGuardian *, CullState *cs, int,
                     CullTraverser *) {
  PStatTimer timer(CullTraverser::_cull_bins_unsorted_pcollector);

  // This shouldn't be called twice for a particular CullState on this
  // bin, since we don't preserve any CullStates between frames.
  nassertv(cs->get_bin() != this);

  claim_cull_state(cs);
  _cull_states.push_back(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::draw
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinUnsorted::
draw(CullTraverser *trav) {
  PStatTimer timer(CullTraverser::_draw_pcollector);

  GraphicsStateGuardian *gsg = trav->get_gsg();

  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "GeomBinUnsorted drawing " << _cull_states.size() << " states.\n";
  }

  CullStates::const_iterator csi;
  for (csi = _cull_states.begin(); csi != _cull_states.end(); ++csi) {
    const CullState *cs = (*csi);
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "GeomBinUnsorted state with " << cs->geom_size() << " geoms and "
        << cs->direct_size() << " direct nodes.\n"
        << "setting state " << cs->get_attributes() << "\n";
    }

    if (cs->geom_size() != 0) {
      gsg->set_state(cs->get_attributes(), true);
      gsg->prepare_display_region();

      CullState::geom_iterator gi;
      for (gi = cs->geom_begin(); gi != cs->geom_end(); ++gi) {
        const ArcChain &arc_chain = (*gi);
        nassertv(!arc_chain.empty());
        GeomNode *geom_node;
        DCAST_INTO_V(geom_node, arc_chain.back()->get_child());
        if (cull_cat.is_spam()) {
          cull_cat.spam()
            << "Drawing " << *geom_node << "\n";
        }
        geom_node->draw(gsg);
      }
    }

    CullState::direct_iterator di;
    for (di = cs->direct_begin(); di != cs->direct_end(); ++di) {
      const ArcChain &arc_chain = (*di);
      nassertv(!arc_chain.empty());

      trav->draw_direct(arc_chain, cs->get_attributes());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinUnsorted::
output(ostream &out) const {
  out << get_type() << " " << get_name()
      << ", " << _cull_states.size() << " states.";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinUnsorted::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << " {\n";

  CullStates::const_iterator csi;
  for (csi = _cull_states.begin(); csi != _cull_states.end(); ++csi) {
    const CullState *cs = (*csi);
    cs->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}
