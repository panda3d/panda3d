// Filename: cullTraverser.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullStateSubtree.h"
#include "config_cull.h"

#include <directRenderTraverser.h>
#include <graphicsStateGuardian.h>

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_default_bin
//       Access: Public
//  Description: Specifies the default GeomBin that any geometry will
//               be assigned to if the scene graph doesn't specify
//               otherwise.  There must always be some default GeomBin
//               in effect.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_default_bin(GeomBin *bin) {
  nassertv(bin != (GeomBin *)NULL);
  bin->attach_to(this, bin->get_sort());
  _default_bin = bin;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_default_bin
//       Access: Public
//  Description: Returns the default GeomBin that any geometry will
//               be assigned to if the scene graph doesn't specify
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE GeomBin *CullTraverser::
get_default_bin() const {
  return _default_bin;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw_geom
//       Access: Public
//  Description: Sets up the GSG to draw the indicated GeomNode in the
//               indicated state.  If the state so demands it and the
//               GeomNode has children, draws the rest of the
//               hierarchy below it immediately.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
draw_geom(GeomNode *geom_node, const AllAttributesWrapper &initial_state) {
  if (cull_cat.is_spam()) {
    cull_cat.spam() 
      << "Drawing " << *geom_node << " with state: " << initial_state << "\n";
  }
  nassertv(geom_node != (GeomNode *)NULL);
  _gsg->set_state(initial_state.get_attributes(), true);
  _gsg->prepare_display_region();
  geom_node->draw(_gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw_direct
//       Access: Public
//  Description: Draws the indicated subtree beginning at the
//               indicated node, and including all nested nodes below.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
draw_direct(Node *node, const AllAttributesWrapper &initial_state) {
  if (cull_cat.is_spam()) {
    cull_cat.spam() 
      << "Drawing " << *node << " in direct mode.\n";
  }
  nassertv(node != (Node *)NULL);
  DirectRenderTraverser drt(_gsg, _graph_type);
  drt.traverse(node, initial_state, AllTransitionsWrapper());
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::add_instance
//       Access: Private
//  Description: This function should be called for each arc that
//               leads to a multiply-instanced node in the graph; that
//               is, each arc that points to a node with multiple
//               parents.  It will return a suitable CullStateLookup
//               pointer that should be passed to subsequent
//               add_geom_node() and add_instance() calls for nodes
//               that descend from this arc.
////////////////////////////////////////////////////////////////////
INLINE CullStateLookup *CullTraverser::
add_instance(NodeRelation *arc, const AllTransitionsWrapper &trans,
	     Node *top_subtree, const CullLevelState &level_state) {
  return level_state._lookup->get_subtree(arc, trans, top_subtree, _now);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::find_bin_state
//       Access: Private
//  Description: Looks for a CullState in the set that corresponds to
//               the indicates set of transitions; if one is found,
//               returns its pointer; otherwise, creates a new one and
//               returns it.
////////////////////////////////////////////////////////////////////
INLINE CullState *CullTraverser::
find_bin_state(const AllTransitionsWrapper &trans) {
  PT(CullState) cs = new CullState(trans);

  // The insert operation will either succeed or fail, depending on
  // whether a matching CullState was already in the set.  It doesn't
  // matter; in either case, the return value represents the
  // corresponding CullState that is (now) stored in the set.
  pair<States::iterator, bool> result = _states.insert(cs);

  /*
  if (result.second) {
    // The insert succeeded, so the CullState was not there
    // previously.
    cerr << "Created CullState for:\n";
    trans.write(cerr, 2);
  }
  */
		       
  return *result.first;
}
