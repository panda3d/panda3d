// Filename: cullTraverser.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullTraverser.h"
#include "geomBinTransition.h"
#include "geomBinAttribute.h"
#include "cullStateSubtree.h"
#include "geomBinNormal.h"
#include "geomBinFixed.h"
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
#include <config_sgattrib.h>    // for support_decals
#include <string_utils.h>

TypeHandle CullTraverser::_type_handle;

#ifdef DO_PSTATS
#include <pStatTimer.h>

PStatCollector CullTraverser::_cull_pcollector =
  PStatCollector("Cull", RGBColorf(0,1,0), 10);
PStatCollector CullTraverser::_draw_pcollector =
  PStatCollector("Draw", RGBColorf(1,0,0), 20);
#endif

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
  _nested_count = 0;

  setup_initial_bins();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
~CullTraverser() {
  // We should detach each of our associated bins when we destruct.
  clear_bins();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::has_bin
//       Access: Public
//  Description: Returns true if a bin by the given name has been
//               attached to the CullTraverser, false otherwise.
////////////////////////////////////////////////////////////////////
bool CullTraverser::
has_bin(const string &name) const {
  return (_toplevel_bins.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_bin
//       Access: Public
//  Description: Returns the GeomBin that was previously attached to
//               the CullTraverser that shares the indicated name, or
//               NULL if no GeomBin with a matching name was added.
////////////////////////////////////////////////////////////////////
GeomBin *CullTraverser::
get_bin(const string &name) const {
  ToplevelBins::const_iterator tbi;
  tbi = _toplevel_bins.find(name);
  if (tbi == _toplevel_bins.end()) {
    return NULL;
  }
  return (*tbi).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::clear_bins
//       Access: Public
//  Description: Disassociates all the GeomBins previously associated
//               with this traverser (and deletes them, if they have
//               no other references).  You must add new GeomBins
//               before rendering by calling set_traverser() on the
//               appropriate bins.
////////////////////////////////////////////////////////////////////
void CullTraverser::
clear_bins() {
  // We can't just run a for loop, because this is a self-modifying
  // operation.
  while (!_toplevel_bins.empty()) {
    GeomBin *bin = (*_toplevel_bins.begin()).second;
    nassertv(bin != (GeomBin *)NULL);
    nassertv(bin->get_traverser() == this);
    bin->clear_traverser();
  }

  nassertv(_sub_bins.empty());
  nassertv(_default_bin == (GeomBin *)NULL);
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

  ToplevelBins::const_iterator tbi;
  for (tbi = _toplevel_bins.begin(); tbi != _toplevel_bins.end(); ++tbi) {
    (*tbi).second->write(out, indent_level);
  }
  _lookup.write(out, indent_level);
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
#ifdef DO_PSTATS
  // Statistics
  PStatTimer timer(_cull_pcollector);
#endif

  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Beginning level " << _nested_count << " cull traversal at "
      << *root << "\n";
  }

  bool is_initial = (_nested_count == 0);
  if (is_initial) {
    if (cull_force_update) {
      _as_of = UpdateSeq::fresh();
    } else {
      _as_of = UpdateSeq::initial();
    }
  }
  _now = last_graph_update(_graph_type);
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
  level_state._as_of = _as_of;

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

  fc_traverse(_arc_chain, root, rel_from_camera, *this,
	      NullAttributeWrapper(), level_state, _gsg, _graph_type);

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
//     Function: CullTraverser::setup_initial_bins
//       Access: Private
//  Description: Creates all the appropriate rendering bins as
//               requested from the Configrc file.
////////////////////////////////////////////////////////////////////
void CullTraverser::
setup_initial_bins() {
  // We always have "default", "background", and "fixed" hardcoded in,
  // although these may be overridden by specifing a new bin with the
  // same name in the Configrc file.

  GeomBinNormal *default_bin = new GeomBinNormal("default");
  GeomBinFixed *background = new GeomBinFixed("background");
  GeomBinFixed *fixed = new GeomBinFixed("fixed");
  background->set_sort(10);
  fixed->set_sort(40);

  default_bin->set_traverser(this);
  background->set_traverser(this);
  fixed->set_traverser(this);


  // Now get the config options.
  Config::ConfigTable::Symbol cull_bins;
  config_cull.GetAll("cull-bin", cull_bins);

  Config::ConfigTable::Symbol::iterator bi;
  for (bi = cull_bins.begin(); bi != cull_bins.end(); ++bi) {
    ConfigString def = (*bi).Val();

    // This is a string in three tokens, separated by whitespace:
    //    bin_name sort type

    vector_string words;
    extract_words(def, words);

    if (words.size() != 3) {
      cull_cat.error()
	<< "Invalid cull-bin definition: " << def << "\n"
	<< "Definition should be three words: bin_name sort type\n";
    } else {
      int sort;
      if (!string_to_int(words[1], sort)) {
	cull_cat.error()
	  << "Invalid cull-bin definition: " << def << "\n"
	  << "Sort token " << words[1] << " is not an integer.\n";

      } else {
	TypeHandle type = GeomBin::parse_bin_type(words[2]);
	if (type == TypeHandle::none()) {
	  cull_cat.error()
	    << "Invalid cull-bin definition: " << def << "\n"
	    << "Bin type " << words[2] << " is not known.\n";
	} else {
	  PT(GeomBin) bin = GeomBin::make_bin(type, words[0]);
	  nassertv(bin != (GeomBin *)NULL);
	  bin->set_sort(sort);
	  bin->set_traverser(this);
	}
      }
    }
  }
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

  SubBins::const_iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi).second->clear_current_states();
  }

  States::iterator si;
  for (si = _states.begin(); si != _states.end(); ++si) {
    CullState *cs = (*si);
    if (!cs->is_empty()) {
      cs->apply_to(_initial_state);

      static string default_bin_name = "default";
      string bin_name = default_bin_name;
      GeomBin *requested_bin = _default_bin;
      int draw_order = 0;

      // Check the requested bin for the Geoms in this state.
      const GeomBinAttribute *bin_attrib;
      if (get_attribute_into(bin_attrib, cs->get_attributes(),
			     GeomBinTransition::get_class_type())) {
	draw_order = bin_attrib->get_draw_order();
	bin_name = bin_attrib->get_bin();
	requested_bin = get_bin(bin_name);
      }

      if (requested_bin == (GeomBin *)NULL) {
	// If we don't have a bin by this name, create one.
	cull_cat.warning()
	  << "Bin " << bin_name << " is unknown; creating a default bin.\n";

	requested_bin = new GeomBinNormal(bin_name);
	requested_bin->set_traverser(this);
      }

      requested_bin->record_current_state(_gsg, cs, draw_order, this);
    }
  }

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    if (cull_cat.is_debug()) {
      cull_cat.debug()
	<< "Drawing " << _sub_bins.size() << " bins.\n";
    }
    for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
      GeomBin *bin = (*sbi).second;
      if (bin->is_active()) {
	bin->draw(this);
      }
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
    (node, complete_trans, level_state._as_of);
  if (cs == (CullState *)NULL) {
    if (cull_cat.is_spam()) {
      cull_cat.spam()
	<< "Finding a new bin state\n";
    }

    // The node didn't have a previously-associated CullState that we
    // could use, so determine a new one for it.
    cs = find_bin_state(complete_trans);
    nassertv(cs != (CullState *)NULL);

    level_state._lookup->record_node(node, cs, level_state._as_of);
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
    (node, complete_trans, level_state._as_of);
  if (cs == (CullState *)NULL) {
    // The node didn't have a previously-associated CullState that we
    // could use, so determine a new one for it.
    cs = find_bin_state(complete_trans);
    nassertv(cs != (CullState *)NULL);

    level_state._lookup->record_node(node, cs, level_state._as_of);
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

  // We have to get a new _now timestamp, just in case either of the
  // above traversals changed it.
  _now = last_graph_update(_graph_type);

  AllTransitionsWrapper trans;

  UpdateSeq last_update = arc->get_last_update();
  if (level_state._as_of < last_update) {
    level_state._as_of = last_update;
  }

  bool is_instanced = (node->get_num_parents(_graph_type) > 1);
  bool is_geom = node->is_of_type(GeomNode::get_class_type());
  bool node_has_sub_render = node->has_sub_render();
  int arc_num_sub_render = arc->get_num_sub_render_trans();
  bool has_decal = arc->has_transition(DecalTransition::get_class_type());

  if (has_decal) {
    // For the purposes of cull, we don't consider a DecalTransition
    // to be a sub_render transition.
    arc_num_sub_render--;
  }

  bool has_direct_render = 
    arc->has_transition(DirectRenderTransition::get_class_type());

#ifndef NDEBUG
  if (support_decals != SD_on) {
    has_direct_render = has_direct_render && !has_decal;
  } else 
#endif
    {
      has_direct_render = has_direct_render || has_decal;
    }

#ifndef NDEBUG
  if (support_subrender == SD_off) {
    node_has_sub_render = false;
    arc_num_sub_render = 0;

  } else if (support_subrender == SD_hide) {
    if ((node_has_sub_render || arc_num_sub_render != 0) &&
	!arc->has_transition(DecalTransition::get_class_type())) {
      return false;
    }
    node_has_sub_render = false;
    arc_num_sub_render = 0;
  }
#endif

#ifndef NDEBUG
  if (support_direct == SD_off) {
    has_direct_render = false;

  } else if (support_direct == SD_hide) {
    if (has_direct_render) {
      return false;
    }
  }
#endif

  if (arc_num_sub_render != 0) {
    level_state._as_of = UpdateSeq::fresh();
  }
  _as_of = level_state._as_of;

  mark_forward_arc(arc);

#ifndef NDEBUG
  if (cull_cat.is_spam()) {
    cull_cat.spam() 
      << "Reached " << *node << ":\n"
      << " as_of = " << level_state._as_of
      << " now = " << _now
      << " is_instanced = " << is_instanced
      << " is_geom = " << is_geom
      << " node_has_sub_render = " << node_has_sub_render
      << " arc_num_sub_render = " << arc_num_sub_render
      << " has_direct_render = " << has_direct_render
      << "\n";
  }
#endif

  if (is_instanced || is_geom || node_has_sub_render || 
      arc_num_sub_render != 0 || has_direct_render) {
    // In any of these cases, we'll need to determine the net
    // transition to this node.
    wrt_subtree(arc, level_state._lookup->get_top_subtree(), 
		level_state._as_of, _now,
		trans, _graph_type);
  }

  if (arc_num_sub_render != 0 || node_has_sub_render) {
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

  if (is_instanced || arc_num_sub_render != 0) {
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

#ifndef NDEBUG
  if (support_decals == SD_hide &&
      arc->has_transition(DecalTransition::get_class_type())) {
    mark_backward_arc(arc);
    return false;
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::attach_toplevel_bin
//       Access: Private
//  Description: This is intended to be called only by
//               GeomBin::attach().  It stores the bin in the
//               appropriate structures within the traverser, as a
//               toplevel bin, e.g. a bin that may be referenced by
//               name from outside the traverser, or one that geometry
//               may be explicitly assigned to by name.
////////////////////////////////////////////////////////////////////
void CullTraverser::
attach_toplevel_bin(GeomBin *bin) {
  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Attaching toplevel bin " << *bin << "\n";
  }

  const string &bin_name = bin->get_name();

  // Insert the new bin by name.
  pair<ToplevelBins::iterator, bool> result = 
    _toplevel_bins.insert(ToplevelBins::value_type(bin_name, bin));

  if (!result.second) {
    // There was already a bin by the same name name in this
    // traverser.  We should therefore detach this bin.
    GeomBin *other_bin = (*result.first).second;
    if (other_bin != bin) {
      other_bin->clear_traverser();
    }

    result =
      _toplevel_bins.insert(ToplevelBins::value_type(bin_name, bin));
    nassertv(result.second);
  }

  if (bin_name == "default") {
    _default_bin = bin;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::attach_sub_bin
//       Access: Private
//  Description: This is intended to be called only by
//               GeomBin::attach().  It stores the bin in the
//               appropriate structures within the traverser, as a
//               sub bin, e.g. a bin that may have geometry assigned
//               to it, and may therefore be rendered.
//
//               In most cases, a toplevel bin is the same as a sub
//               bin, except in the case of a GeomBinGroup, which is a
//               toplevel bin that contains a number of sub bins.
////////////////////////////////////////////////////////////////////
void CullTraverser::
attach_sub_bin(GeomBin *bin) {
  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Attaching sub bin " << *bin << "\n";
  }

  _sub_bins.insert(SubBins::value_type(bin->get_sort(), bin));
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::detach_toplevel_bin
//       Access: Private
//  Description: This is intended to be called only by
//               GeomBin::detach().  It removes the bin from the
//               appropriate structures within the traverser, as a
//               toplevel bin.  See attach_toplevel_bin().
////////////////////////////////////////////////////////////////////
void CullTraverser::
detach_toplevel_bin(GeomBin *bin) {
  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Detaching toplevel bin " << *bin << "\n";
  }

  const string &bin_name = bin->get_name();

  ToplevelBins::iterator tbi = _toplevel_bins.find(bin_name);
  nassertv(tbi != _toplevel_bins.end());
  _toplevel_bins.erase(tbi);

  if (bin_name == "default") {
    _default_bin = (GeomBin *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::detach_sub_bin
//       Access: Private
//  Description: This is intended to be called only by
//               GeomBin::detach().  It removes the bin from the
//               appropriate structures within the traverser, as a
//               sub bin.  See attach_sub_bin().
////////////////////////////////////////////////////////////////////
void CullTraverser::
detach_sub_bin(GeomBin *bin) {
  if (cull_cat.is_debug()) {
    cull_cat.debug()
      << "Detaching sub bin " << *bin << "\n";
  }

  // Remove the bin from its place in the sort order list.  This will
  // be one of the (possibly several) entries in the multimap with the
  // same sort value.
  pair<SubBins::iterator, SubBins::iterator> range;
  range = _sub_bins.equal_range(bin->get_sort());

  SubBins::iterator sbi;
  for (sbi = range.first; sbi != range.second; ++sbi) {
    GeomBin *consider_bin = (*sbi).second;
    nassertv(consider_bin->get_sort() == bin->get_sort());

    if (consider_bin == bin) {
      // Here's the position!
      _sub_bins.erase(sbi);
      return;
    }
  }
  nassertv(false);
}
