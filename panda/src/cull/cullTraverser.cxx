// Filename: cullTraverser.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullTraverser.h"
#include "geomBinTransition.h"
#include "geomBinAttribute.h"
#include "cullStateSubtree.h"
#include "geomBinNormal.h"
#include "directRenderTransition.h"
#include "config_cull.h"

#include <wrt.h>
#include <frustumCullTraverser.h>
#include <graphicsStateGuardian.h>
#include <decalTransition.h>
#include <pruneTransition.h>
#include <transformTransition.h>
#include <nodeTransitionWrapper.h>
#include <indent.h>
#include <config_sgraphutil.h>  // for implicit_app_traversal
#include <pStatTimer.h>

TypeHandle CullTraverser::_type_handle;

PStatCollector CullTraverser::_cull_pcollector =
  PStatCollector("Cull", RGBColorf(0,1,0), 10);
PStatCollector CullTraverser::_draw_pcollector =
  PStatCollector("Draw", RGBColorf(1,0,0), 20);



////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
CullTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
	      const ArcChain &arc_chain) :
  RenderTraverser(gsg, graph_type, arc_chain)
{
  _default_bin = new GeomBinNormal("default", this);
  _nested_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
~CullTraverser() {
  // We should detach each of our associated bins when we destruct.
  // We can't just run a for loop, because this is a self-modifying
  // operation.
  while (!_bins.empty()) {
    GeomBin *bin = (*_bins.begin());
    nassertv(bin != (GeomBin *)NULL);
    nassertv(bin->get_traverser() == this);
    bin->detach();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::traverse
//       Access: Public, Virtual
//  Description: This performs a normal, complete cull-and-render
//               traversal using this CullTraverser object.  State is
//               saved within the CullTraverser object so that the
//               next frame will be processed much more quickly than
//               the first frame.
////////////////////////////////////////////////////////////////////
void CullTraverser::
traverse(Node *root, 
	 const AllAttributesWrapper &initial_state,
	 const AllTransitionsWrapper &net_trans) {
  // Statistics
  PStatTimer timer(_cull_pcollector);

  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Beginning level " << _nested_count << " cull traversal at "
      << *root << "\n";
  }

  nassertv(!_bins.empty());

  bool is_initial = (_nested_count == 0);
  if (is_initial) {
    if (cull_force_update) {
      _now = UpdateSeq::fresh();
    } else {
      _now = UpdateSeq::initial();
    }
  }
  _nested_count++;

  if (is_initial) {
    _initial_state.apply_from(initial_state, net_trans);

    States::iterator si;
    for (si = _states.begin(); si != _states.end(); ++si) {
      (*si)->clear_current_nodes();
    }
  }

  CullLevelState level_state;
  level_state._lookup = &_lookup;
  level_state._now = _now;

  // Determine the relative transform matrix from the camera to our
  // starting node.  This is important for proper view-frustum
  // culling.
  LMatrix4f rel_from_camera;
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(_gsg->get_current_projection_node(), root, begin(), end(),
      ntw, get_graph_type());
  const TransformTransition *tt;
  if (get_transition_into(tt, ntw)) {
    rel_from_camera = tt->get_matrix();
  } else {
    // No relative transform.
    rel_from_camera = LMatrix4f::ident_mat();
  }

  fc_traverse(root, rel_from_camera, *this, NullAttributeWrapper(), 
	      level_state, _gsg, _graph_type);

  if (is_initial) {
    draw();
    clean_out_old_states();
  }

  _nested_count--;

  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Ending level " << _nested_count << " cull traversal at "
      << *root << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullTraverser::
output(ostream &out) const {
  int node_count = 0;
  int used_states = 0;

  States::const_iterator si;
  for (si = _states.begin(); si != _states.end(); ++si) {
    const CullState *cs = (*si);
    int c = cs->count_current_nodes();
    if (c != 0) {
      node_count += c;
      used_states++;
    }
  }

  out << node_count << " nodes with " << used_states << " states; "
      << _states.size() - used_states << " unused states.";
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullTraverser::
write(ostream &out, int indent_level) const {
  /*
  States::const_iterator si;
  for (si = _states.begin(); si != _states.end(); ++si) {
    const CullState *cs = (*si);
    cs->write(out, indent_level);
    out << "\n";
  }
  */

  Bins::const_iterator bi;
  for (bi = _bins.begin(); bi != _bins.end(); ++bi) {
    (*bi)->write(out, indent_level);
  }
  _lookup.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw
//       Access: Private
//  Description: Puts all of the renderable geometry into the
//               appropriate bins, and renders the bins.
////////////////////////////////////////////////////////////////////
void CullTraverser::
draw() {
  if (cull_cat.is_debug()) {
    // Count up the nonempty states for debug output.
    int num_states = 0;
    States::iterator si;
    for (si = _states.begin(); si != _states.end(); ++si) {
      CullState *cs = (*si);
      if (!cs->is_empty()) {
	num_states++;
      }
    }

    cull_cat.debug()
      << "Initiating draw with " << num_states
      << " nonempty states of " << _states.size() << " total.\n";
  }

  Bins::iterator bi;
  for (bi = _bins.begin(); bi != _bins.end(); ++bi) {
    (*bi)->clear_current_states();
  }

  States::iterator si;
  for (si = _states.begin(); si != _states.end(); ++si) {
    CullState *cs = (*si);
    if (!cs->is_empty()) {
      cs->apply_to(_initial_state);

      // Check the requested bin for the Geoms in this state.
      GeomBin *requested_bin = _default_bin;
      int draw_order = 0;

      const GeomBinAttribute *bin_attrib;
      if (get_attribute_into(bin_attrib, cs->get_attributes(),
			     GeomBinTransition::get_class_type())) {
	requested_bin = bin_attrib->get_bin();
	draw_order = bin_attrib->get_draw_order();
      }

      requested_bin->record_current_state(_gsg, cs, draw_order, this);
    }
  }

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    if (cull_cat.is_debug()) {
      cull_cat.debug()
	<< "Drawing " << _bins.size() << " bins.\n";
    }
    for (bi = _bins.begin(); bi != _bins.end(); ++bi) {
      (*bi)->draw(this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::clean_out_old_states
//       Access: Private
//  Description: Walks through the list of CullStates after a frame
//               has been completely rendered and drawn, and removes
//               any from the list that don't seem to be in use any
//               more.
////////////////////////////////////////////////////////////////////
void CullTraverser::
clean_out_old_states() {
  _lookup.clean_out_old_nodes();

  States::iterator si, snext;
  si = _states.begin();
  snext = si;
  while (si != _states.end()) {
    ++snext;

    CullState *cs = (*si);
    if (cs->get_ref_count() == 1 && cs->is_empty() && !cs->has_bin()) {
      // This CullState doesn't seem to be used anywhere else.
      _states.erase(si);
    }
    si = snext;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::add_geom_node
//       Access: Private
//  Description: Records the indicated GeomNode as one that is visible
//               this frame with the indicated state, setting up an
//               appropriate CullState node and adding it to the
//               appropriate bin.
////////////////////////////////////////////////////////////////////
void CullTraverser::
add_geom_node(GeomNode *node, const AllTransitionsWrapper &trans,
	      const CullLevelState &level_state) {
  nassertv(node != (GeomNode *)NULL);
  const ArcChain &arc_chain = get_arc_chain();
  nassertv(!arc_chain.empty());
  nassertv(arc_chain.back()->get_child() == node);

  AllTransitionsWrapper complete_trans;
  level_state._lookup->compose_trans(trans, complete_trans);

  // Actually, there should be no need to remove this transition
  // explicitly, as we should never encounter it--we'll never traverse
  // below a DirectRenderTransition in the CullTraverser.  We try to
  // remove it here just to be paranoid.
  complete_trans.clear_transition(DirectRenderTransition::get_class_type());

  CullState *cs = level_state._lookup->find_node
    (node, complete_trans, level_state._now);
  if (cs == (CullState *)NULL) {
    if (cull_cat.is_spam()) {
      cull_cat.spam()
	<< "Finding a new bin state\n";
    }

    // The node didn't have a previously-associated CullState that we
    // could use, so determine a new one for it.
    cs = find_bin_state(complete_trans);
    nassertv(cs != (CullState *)NULL);

    level_state._lookup->record_node(node, cs, level_state._now);
  }

  cs->record_current_geom_node(arc_chain);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::add_direct_node
//       Access: Private
//  Description: Records the indicated Node as one that is immediately
//               under a DirectRenderTransition, and thus it and its
//               subtree should be rendered directly, in a depth-first
//               traversal.  This is usually done when the render order
//               is very important within a small subtree of nodes.
////////////////////////////////////////////////////////////////////
void CullTraverser::
add_direct_node(Node *node, const AllTransitionsWrapper &trans,
		const CullLevelState &level_state) {
  nassertv(node != (Node *)NULL);
  const ArcChain &arc_chain = get_arc_chain();
  nassertv(!arc_chain.empty());
  nassertv(arc_chain.back()->get_child() == node);

  AllTransitionsWrapper complete_trans;
  level_state._lookup->compose_trans(trans, complete_trans);

  // Remove this, just so we won't clutter up the state unnecessarily
  // and interfere with state-sorting.
  complete_trans.clear_transition(DirectRenderTransition::get_class_type());

  CullState *cs = level_state._lookup->find_node
    (node, complete_trans, level_state._now);
  if (cs == (CullState *)NULL) {
    // The node didn't have a previously-associated CullState that we
    // could use, so determine a new one for it.
    cs = find_bin_state(complete_trans);
    nassertv(cs != (CullState *)NULL);

    level_state._lookup->record_node(node, cs, level_state._now);
  }

  cs->record_current_direct_node(arc_chain);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::forward_arc
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool CullTraverser::
forward_arc(NodeRelation *arc, NullTransitionWrapper &,
	    NullAttributeWrapper &, NullAttributeWrapper &,
	    CullLevelState &level_state) {
  nassertr(level_state._lookup != (CullStateLookup *)NULL, false);

  if (arc->has_transition(PruneTransition::get_class_type())) {
    return false;
  }

  Node *node = arc->get_child();

  if (implicit_app_traversal) {
    node->app_traverse();
  }
  node->draw_traverse();

  AllTransitionsWrapper trans;

  UpdateSeq last_update = arc->get_last_update();
  if (level_state._now < last_update) {
    level_state._now = last_update;
  }

  bool is_instanced = (node->get_num_parents(_graph_type) > 1);
  bool is_geom = node->is_of_type(GeomNode::get_class_type());
  bool node_has_sub_render = node->has_sub_render();
  bool arc_has_sub_render = arc->has_sub_render_trans();
  bool has_direct_render =
    arc->has_transition(DirectRenderTransition::get_class_type()) ||
    arc->has_transition(DecalTransition::get_class_type());

  if (arc_has_sub_render) {
    level_state._now = UpdateSeq::fresh();
  }
  _now = level_state._now;

  mark_forward_arc(arc);

  if (cull_cat.is_spam()) {
    cull_cat.spam() 
      << "Reached " << *node << ":\n"
      << " now = " << level_state._now
      << " is_instanced = " << is_instanced
      << " is_geom = " << is_geom
      << " node_has_sub_render = " << node_has_sub_render
      << " arc_has_sub_render = " << arc_has_sub_render
      << " has_direct_render = " << has_direct_render
      << "\n";
  }

  if (is_instanced || is_geom || node_has_sub_render || 
      arc_has_sub_render || has_direct_render) {
    // In any of these cases, we'll need to determine the net
    // transition to this node.
    wrt_subtree(arc, level_state._lookup->get_top_subtree(), 
		trans, _graph_type);
  }

  if (arc_has_sub_render || node_has_sub_render) {
    if (_gsg != (GraphicsStateGuardian *)NULL) {
      AllTransitionsWrapper complete_trans;
      level_state._lookup->compose_trans(trans, complete_trans);
      AllAttributesWrapper attrib;
      attrib.apply_from(_initial_state, complete_trans);

      AllTransitionsWrapper new_trans;

      if (!arc->sub_render_trans(attrib, new_trans, this) ||
	  !node->sub_render(attrib, new_trans, this)) {
	mark_backward_arc(arc);
	return false;
      }

      trans.compose_in_place(new_trans);
    }
  }

  if (has_direct_render) {
    add_direct_node(node, trans, level_state);
    // Since we're adding the node to be rendered directly, we won't
    // traverse any further beyond it--the rest of the subgraph
    // beginning at this node will be traversed when the node is
    // rendered.
    mark_backward_arc(arc);
    return false;
  }

  if (is_instanced || arc_has_sub_render) {
    // This node is multiply instanced; thus, it begins a subtree.
    level_state._lookup = add_instance(arc, trans, node, level_state);
    if (cull_cat.is_spam()) {
      cull_cat.spam()
	<< "Added " << *node << " as instance.\n";
    }

    // It might also be an instanced GeomNode.
    if (is_geom) {
      if (cull_cat.is_spam()) {
	cull_cat.spam()
	  << "Added " << *node << "\n";
      }
      add_geom_node(DCAST(GeomNode, node), AllTransitionsWrapper(), level_state);
    }

  } else if (is_geom) {
    if (cull_cat.is_spam()) {
      cull_cat.spam()
	<< "Added " << *node << "\n";
    }
    add_geom_node(DCAST(GeomNode, node), trans, level_state);
  }

  return true;
}
