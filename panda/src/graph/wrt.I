// Filename: wrt.I
// Created by:  drose (26Oct98)
// 
////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: describe_ambiguous_wrt
//  Description: Writes an error message to wrt_cat describing an
//               attempt to compute an ambiguous wrt() without
//               specifying enough arcs to resolve the ambiguity.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
void
describe_ambiguous_wrt(const Node *node,
		       InputIterator arc_list_begin, 
		       InputIterator arc_list_end,
		       TypeHandle graph_type) {
  if (wrt_cat.is_warning()) {
    const UpRelationPointers &urp = node->find_connection(graph_type).get_up();
    int num_parents = urp.size();
    wrt_cat.warning()
      << *node << " has " << num_parents << " parents; wrt() ambiguous.\n"
      << "  parents are: ";
    nassertv(num_parents > 1);

    NodeRelation *parent_arc = urp[0];
    wrt_cat.warning(false) << *parent_arc->get_parent();
    for (int i = 1; i < num_parents; i++) {
      parent_arc = urp[i];
      wrt_cat.warning(false) 
	<< ", " << *parent_arc->get_parent();
    }
    wrt_cat.warning(false) << "\n";
    
    // Show the chain of arcs specified.
    if (arc_list_begin == arc_list_end) {
      wrt_cat.warning(false)
	<< "  no arcs specified.\n";
    } else {
      wrt_cat.warning(false)
	<< "  arcs specified:\n";
      InputIterator ri = arc_list_begin;
      wrt_cat.warning(false)
	<< "    " << *(*ri)->get_parent() << "/"
	<< *(*ri)->get_child();
      
      InputIterator ri_last = ri;
      ++ri;
      while (ri != arc_list_end) {
	if ((*ri)->get_parent() == (*ri_last)->get_child()) {
	  wrt_cat.warning(false)
	    << "/" << *(*ri)->get_child();
	} else {
	  wrt_cat.warning(false)
	    << "\n    " << *(*ri)->get_parent() << "/"
	    << *(*ri)->get_child();
	}
	ri_last = ri;
	++ri;
      } 
      wrt_cat.warning(false) << "\n";
    }
  }
  if (ambiguous_wrt_abort) {
    abort();
  }
}
#endif


////////////////////////////////////////////////////////////////////
//     Function: get_cached_net_transition
//  Description: Returns the net transition from the root of the
//               graph, or the nearest ancestor with multiple parents,
//               to and including the indicated arc.  This is a
//               support function for wrt() and should not be called
//               directly.
//
//               This flavor of get_cached_net_transition() gets the
//               transition up to an arc, instead of a node, and does
//               not necessarily start at the root of the scene graph.
//               By starting at the nearest ancestor with multiple
//               parents, we allow ambiguous paths in the scene graph
//               without interfering with state caching.  Since there
//               are no nodes with multiple parents in the chain of
//               transitions returned by this function, the result is
//               always unambiguous; any wrt() ambiguity is resolved
//               later.
//
//               as_of is the most recent update so far on this branch
//               of the scene graph; now is the stamp to use to mark
//               any computed cache values.
////////////////////////////////////////////////////////////////////
template<class TransitionWrapper>
void
get_cached_net_transition(NodeRelation *arc, Node *&root, 
			  UpdateSeq as_of, UpdateSeq now,
			  TransitionWrapper &net, TypeHandle graph_type) {
  TransitionWrapper cur_cache = TransitionWrapper::init_from(net);
  Node *top_subtree = cur_cache.extract_from_cache(arc);

  if (cur_cache.is_cache_verified(as_of)) {
    // This arc's cached value has recently been verified, so we don't
    // even need to go anywhere--we trust it's still good.
    root = top_subtree;
    net = cur_cache;

#ifndef NDEBUG
    if (wrt_cat.is_debug()) {
      wrt_cat.debug()
	<< "get_cached_net_transition(" << *arc->get_child() << ", ";
      if (root == (Node *)NULL) {
	wrt_cat.debug(false) << "(top)";
      } else {
	wrt_cat.debug(false) << *root;
      }
      wrt_cat.debug(false)
	<< ") is current as of " << as_of << " (now is " 
	<< now << ")\n";
      if (wrt_cat.is_spam()) {
	wrt_cat.spam() << "result of " << *arc << " is " << net << "\n";
      }	
    }
#endif

  } else {
    // This arc's cache hasn't recently been verified, and we need to
    // verify it now.  This will entail at least walking up the scene
    // graph to the root, and possibly recomputing the cache as we go.

    // Get the node above the arc.
    Node *node = arc->get_parent();

    const UpRelationPointers &urp = node->find_connection(graph_type).get_up();
    int num_parents = urp.size();

    if (num_parents != 1) {
      // The node has zero or multiple parents.  It's thus either the
      // top of the scene graph, or it's the first ancestor with
      // multiple parents; in either case, it's as far as we can take
      // the caching.  Stop here.

      if (num_parents == 0) {
	// The node is the very top of the scene graph.  We'll treat
	// this as a special case by setting the node to NULL.  This
	// will tell our calling function that there's no need to look
	// further.
	node = NULL;
      }

      root = node;
      net.extract_from(arc);
      net.set_computed_verified(now);
      net.store_to_cache(arc, node);

    } else {
      // The node has exactly one parent.  Carry on.

      NodeRelation *parent_arc = urp[0];
      get_cached_net_transition(parent_arc, root, as_of, now, net, graph_type);

      // Now recompute our own cache.
      TransitionWrapper cur_value = TransitionWrapper::init_from(net);
      cur_value.extract_from(arc);

      net.cached_compose(cur_cache, cur_value, now);
      net.store_to_cache(arc, root);
    }

#ifndef NDEBUG
    if (wrt_cat.is_debug()) {
      wrt_cat.debug()
	<< "get_cached_net_transition(" << *arc->get_child() << ", ";
      if (root == (Node *)NULL) {
	wrt_cat.debug(false) << "(top)";
      } else {
	wrt_cat.debug(false) << *root;
      }
      wrt_cat.debug(false)
	<< ") is recomputed as of " << as_of << " (now is " 
	<< now << ")\n";
      if (wrt_cat.is_spam()) {
	wrt_cat.spam() << "result of " << *arc << " is " << net << "\n";
      }	
    }
#endif
  }
}    


////////////////////////////////////////////////////////////////////
//     Function: get_cached_net_transition
//  Description: Returns the net transition from the root of the graph
//               to the indicated node.  This is a support function
//               for wrt() and should not be called directly.
//
//               If [arc_list_begin..arc_list_end) is nonempty, it
//               contains a list of NodeRelation pointers to resolve
//               ambiguities when a node with multiple parents is
//               encountered.
//
//               as_of is the most recent update so far on this branch
//               of the scene graph; now is the stamp to use to mark
//               any computed cache values.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class TransitionWrapper>
void
get_cached_net_transition(const Node *node, 
			  InputIterator arc_list_begin, 
			  InputIterator arc_list_end,
			  UpdateSeq as_of, UpdateSeq now,
			  TransitionWrapper &result,
			  TypeHandle graph_type) {
  if (node == NULL) {
    // the NULL node is by convention equivalent to the root.
    result.make_identity();

#ifndef NDEBUG
    if (wrt_cat.is_debug()) {
      wrt_cat.debug()
	<< "get_cached_net_transition((top))\n";
    }
#endif
    return;
  }

  const UpRelationPointers &urp = node->find_connection(graph_type).get_up();
  int num_parents = urp.size();

  const NodeRelation *parent_arc = (const NodeRelation *)NULL;

  if (num_parents == 1) {
    // There is only one parent, so there's no doubt as to which arc
    // to choose.
    parent_arc = urp[0];

  } else if (num_parents == 0) {
    // This node has no parents.  Stop here.
    result.make_identity();
    
#ifndef NDEBUG
    if (wrt_cat.is_debug()) {
      wrt_cat.debug()
	<< "get_cached_net_transition(" << *node << ") has no parents.\n";
    }
#endif
    return;

  } else {
    // The node has multiple parents, and we have a list of arcs in
    // arc_list_begin .. arc_list_end.  If any of the parents is
    // mentioned in the list, we must choose that parent.
    for (InputIterator ri = arc_list_begin; 
	 parent_arc == (NodeRelation *)NULL && ri != arc_list_end; 
	 ++ri) {
      if ((*ri)->get_child() == node) {
	parent_arc = (*ri);
      }
    }

    if (parent_arc == (NodeRelation *)NULL) {
#ifndef NDEBUG
      // No, it wasn't mentioned.  Issue a warning and use the first
      // one.
      describe_ambiguous_wrt(node, arc_list_begin, arc_list_end, graph_type);
#endif
      parent_arc = urp[0];
    }
  }

  // Get the net transition leading into this node.
  Node *root;
  get_cached_net_transition((NodeRelation *)parent_arc, root, 
			    as_of, now, result, graph_type);

  if (root != NULL) {
    // There's more to get.  The above function call was forced to
    // stop at a node with multiple parents.
    TransitionWrapper more = TransitionWrapper::init_from(result);
    get_cached_net_transition(root, arc_list_begin, arc_list_end, 
			      as_of, now, more, graph_type);
    more.compose_in_place(result);
    result = more;
  }
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: get_uncached_net_transition
//  Description: Returns the net transition from the root of the graph
//               to the indicated node.  This is a support function
//               for wrt() and should not be called directly.
//
//               If [arc_list_begin..arc_list_end) is nonempty, it
//               contains a list of NodeRelation pointers to resolve
//               ambiguities when a node with multiple parents is
//               encountered.
//
//               This variant of get_cached_net_transition() does not
//               respect or update the cache values stored in the
//               arcs.  It's intended only as a debugging doublecheck
//               against the normal get_cached_net_transition()
//               function that does do this.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class TransitionWrapper>
void
get_uncached_net_transition(const Node *node, 
			    InputIterator arc_list_begin, 
			    InputIterator arc_list_end,
			    TransitionWrapper &result,
			    TypeHandle graph_type) {
  if (node == NULL) {
    // the NULL node is by convention equivalent to the root.
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "get_uncached_net_transition(top)" << *node;
    }
    result.make_identity();
    return;
  }

  const UpRelationPointers &urp = node->find_connection(graph_type).get_up();
  int num_parents = urp.size();

  const NodeRelation *parent_arc = (const NodeRelation *)NULL;

  if (num_parents == 1) {
    // There is only one parent, so there's no doubt as to which arc
    // to choose.
    parent_arc = urp[0];

  } else if (num_parents == 0) {
    // This node has no parents.  Stop here.
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "get_uncached_net_transition(" << *node << ") has no parents.\n";
    }
    result.make_identity();
    return;

  } else {
    // The node has multiple parents, and we have a list of arcs in
    // arc_list_begin .. arc_list_end.  If any of the parents is
    // mentioned in the list, we must choose that parent.
    for (InputIterator ri = arc_list_begin; 
	 parent_arc == (NodeRelation *)NULL && ri != arc_list_end; 
	 ++ri) {
      if ((*ri)->get_child() == node) {
	parent_arc = (*ri);
      }
    }

    if (parent_arc == (NodeRelation *)NULL) {
#ifndef NDEBUG
      // No, it wasn't mentioned.  Issue a warning and use the first
      // one. 
      describe_ambiguous_wrt(node, arc_list_begin, arc_list_end, graph_type);
#endif
      parent_arc = urp[0];
    }
  }

  get_uncached_net_transition(parent_arc->get_parent(),
			      arc_list_begin, arc_list_end,
			      result,
			      graph_type);

  TransitionWrapper next = TransitionWrapper::init_from(result);
  next.extract_from(parent_arc);

  result.compose_in_place(next);

  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "get_uncached_net_transition(" << *node;
    InputIterator ri;
    for (ri = arc_list_begin; ri != arc_list_end; ++ri) {
      wrt_cat.spam(false) << ", " << *(*ri);
    }
    
    wrt_cat.spam(false)
      << "), " << num_parents << " parents, is:\n";
    result.write(wrt_cat.spam(false), 2);
  }
}
#endif


////////////////////////////////////////////////////////////////////
//     Function: cached_wrt_base
//  Description: The implementation of wrt_base, below, but always
//               uses the cached value.  This is the normal behavior.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class TransitionWrapper>
void
cached_wrt_base(const Node *from,
		InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
		const Node *to, 
		InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
		TransitionWrapper &result,
		TypeHandle graph_type) {

  UpdateSeq now = last_graph_update(graph_type);

  TransitionWrapper net_from_trans = TransitionWrapper::init_from(result);

#ifndef NDEBUG
  if (wrt_cat.is_debug()) {
    wrt_cat.debug()
      << "Computing wrt(";
    if (from == (Node *)NULL) {
      wrt_cat.debug(false) << "(top), ";
    } else {
      wrt_cat.debug(false) << *from << ", ";
    }
    if (to == (Node *)NULL) {
      wrt_cat.debug(false) << "(top)";
    } else {
      wrt_cat.debug(false) << *to;
    }
    wrt_cat.debug(false)
      << "), as of " << now << " (now is " 
      << now << ")\n";
  }
#endif

  get_cached_net_transition(from, from_arcs_begin, from_arcs_end, 
			    now, now, net_from_trans, graph_type);
  get_cached_net_transition(to, to_arcs_begin, to_arcs_end, 
			    now, now, result, graph_type);
  
#ifndef NDEBUG
  if (paranoid_wrt) {
    // We don't entirely trust the caching to work flawlessly.  Go
    // ahead and compute the uncached transition just to doublecheck.
    TransitionWrapper check_from_trans =
      TransitionWrapper::init_from(result);
    TransitionWrapper check_to_trans = 
      TransitionWrapper::init_from(result);
    get_uncached_net_transition(from, from_arcs_begin, from_arcs_end,
				check_from_trans, graph_type);
    if (check_from_trans.compare_to(net_from_trans) != 0) {
      wrt_cat.warning()
	<< "WRT cache from " << *from << " is invalid!\n"
	<< "  cached value is:\n";
      net_from_trans.write(wrt_cat.warning(false), 4);
      wrt_cat.warning(false)
	<< "  should be:\n";
      check_from_trans.write(wrt_cat.warning(false), 4);
      net_from_trans = check_from_trans;
    }
    
    get_uncached_net_transition(to, to_arcs_begin, to_arcs_end,
				check_to_trans, graph_type);
    if (check_to_trans.compare_to(result) != 0) {
      wrt_cat.warning()
	<< "WRT cache to " << *to << " is invalid!\n"
	<< "  cached value is:\n";
      result.write(wrt_cat.warning(false), 4);
      wrt_cat.warning(false)
	<< "  should be:\n";
      check_to_trans.write(wrt_cat.warning(false), 4);
      result = check_to_trans;
    }
  }
#endif

  result.invert_compose_in_place(net_from_trans);
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: uncached_wrt_base
//  Description: The implementation of wrt_base, below, but never
//               cached.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class TransitionWrapper>
void
uncached_wrt_base(const Node *from,
		  InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
		  const Node *to, 
		  InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
		  TransitionWrapper &result,
		  TypeHandle graph_type) {
  TransitionWrapper net_from_trans = TransitionWrapper::init_from(result);
  get_uncached_net_transition(from, from_arcs_begin, from_arcs_end,
			      net_from_trans, graph_type);
  get_uncached_net_transition(to, to_arcs_begin, to_arcs_end,
			      result, graph_type);
  result.invert_compose_in_place(net_from_trans);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: wrt_base
//  Description: Returns the net transition from the node "to"
//               to the node "from".
//
//               If [from_arcs_begin..from_arcs_end) is nonempty, it
//               contains a list of NodeRelation pointers to resolve
//               ambiguities when a node with multiple parents is
//               encountered as an ancestor of the from node.  If any
//               of the parents is listed, that parent will be chosen.
//
//               Similarly with [to_arcs_begin..to_arcs_end).
//
//               wrt_base() is a low-level implementation; use the
//               various wrt() wrappers instead.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class TransitionWrapper>
INLINE_GRAPH void
wrt_base(const Node *from,
	 InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
	 const Node *to, 
	 InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
	 TransitionWrapper &result,
	 TypeHandle graph_type) {
#ifndef NDEBUG
  if (!cache_wrt) {
    uncached_wrt_base(from, from_arcs_begin, from_arcs_end,
		      to, to_arcs_begin, to_arcs_end,
		      result, graph_type);
    return;
  }
#endif
  cached_wrt_base(from, from_arcs_begin, from_arcs_end,
		  to, to_arcs_begin, to_arcs_end,
		  result, graph_type);
}

template<class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from, const Node *to,
    TransitionWrapper &result, TypeHandle graph_type) {
  // In the normal wrt() interface, we have no from_arcs or to_arcs
  // list.  Supply empty lists for both.
  const NodeRelation *arc = NULL;
  wrt_base(from, &arc, &arc, to, &arc, &arc, result, graph_type);
}

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    InputIterator from_arcs_begin, InputIterator from_arcs_end,
    const Node *to, 
    TransitionWrapper &result, TypeHandle graph_type) {
  const NodeRelation *arc = NULL;
  wrt_base(from, from_arcs_begin, from_arcs_end, to, &arc, &arc,
	   result, graph_type);
}

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    const Node *to,  
    InputIterator to_arcs_begin, InputIterator to_arcs_end,
    TransitionWrapper &result, TypeHandle graph_type) {
  const NodeRelation *arc = NULL;
  wrt_base(from, &arc, &arc, to, to_arcs_begin, to_arcs_end,
	   result, graph_type);
}

template<class InputIterator1, class InputIterator2, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
    const Node *to, 
    InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
    TransitionWrapper &result, TypeHandle graph_type) {
  wrt_base(from, from_arcs_begin, from_arcs_end,
	   to, to_arcs_begin, to_arcs_end,
	   result, graph_type);
}

#ifndef NDEBUG
template<class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from, const Node *to,
	     TransitionWrapper &result, TypeHandle graph_type) {
  const NodeRelation *arc = NULL;
  uncached_wrt_base(from, &arc, &arc, to, &arc, &arc, result, graph_type);
}

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     InputIterator from_arcs_begin, InputIterator from_arcs_end,
	     const Node *to, 
	     TransitionWrapper &result, TypeHandle graph_type) {
  const NodeRelation *arc = NULL;
  uncached_wrt_base(from, from_arcs_begin, from_arcs_end, to, &arc, &arc,
		    result, graph_type);
}

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     const Node *to,  
	     InputIterator to_arcs_begin, InputIterator to_arcs_end,
	     TransitionWrapper &result, TypeHandle graph_type) {
  const NodeRelation *arc = NULL;
  uncached_wrt_base(from, &arc, &arc, to, to_arcs_begin, to_arcs_end,
		    result, graph_type);
}

template<class InputIterator1, class InputIterator2, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
	     const Node *to, 
	     InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
	     TransitionWrapper &result, TypeHandle graph_type) {
  uncached_wrt_base(from, from_arcs_begin, from_arcs_end,
		    to, to_arcs_begin, to_arcs_end,
		    result, graph_type);
}
#endif

#ifndef NDEBUG
template<class TransitionWrapper>
Node *
get_uncached_wrt_subtree(Node *node, Node *to, TransitionWrapper &result, 
			 TypeHandle graph_type) {
  nassertr(node != (Node *)NULL, to);

  const UpRelationPointers &urp = node->find_connection(graph_type).get_up();
  int num_parents = urp.size();

  if (num_parents == 0) {
    // This node has no parents.  Stop here.
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "get_uncached_wrt_subtree(" << *node << ") has no parents.\n";
    }
    result.make_identity();
    return (Node *)NULL;
  }

  if (num_parents != 1) {
    // There are multiple parents, so stop here.
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "get_uncached_wrt_subtree(" << *node << ") has " << num_parents
	<< " parents.\n";
    }
    result.make_identity();
    return node;
  }

  if (node == to) {
    // We've reached our stopping point.  Stop here.
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "get_uncached_wrt_subtree(" << *node << ") is at stopping point.\n";
    }
    result.make_identity();

    // Actually, we do need to keep walking up till we find the actual
    // top of the instanced subtree, so we can return the correct
    // pointer.
    num_parents = node->get_num_parents(graph_type);
    while (num_parents == 1) {
      node = node->get_parent(graph_type, 0)->get_parent();
      num_parents = node->get_num_parents(graph_type);
    }

    if (num_parents == 0) {
      return (Node *)NULL;
    } else {
      return node;
    }
  }

  const NodeRelation *parent_arc = urp[0];

  Node *stop = 
    get_uncached_wrt_subtree(parent_arc->get_parent(), to,
			     result, graph_type);

  TransitionWrapper next = TransitionWrapper::init_from(result);
  next.extract_from(parent_arc);

  result.compose_in_place(next);

  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "get_uncached_wrt_subtree(" << *node
      << "), top at ";
    if (stop == (Node *)NULL) {
      wrt_cat.spam(false) << "(top)";
    } else {
      wrt_cat.spam(false) << *stop;
    }
    wrt_cat.spam(false) << " is:\n";
    result.write(wrt_cat.spam(false), 2);
  }

  return stop;
}

template<class TransitionWrapper>
INLINE_GRAPH Node *
uncached_wrt_subtree(NodeRelation *arc, Node *to,
		     TransitionWrapper &result, TypeHandle graph_type) {
  Node *stop = 
    get_uncached_wrt_subtree(arc->get_parent(), to,
			     result, graph_type);

  TransitionWrapper next = TransitionWrapper::init_from(result);
  next.extract_from(arc);

  result.compose_in_place(next);

  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "uncached_wrt_subtree(" << *arc
      << "), top at ";
    if (stop == (Node *)NULL) {
      wrt_cat.spam(false) << "(top)";
    } else {
      wrt_cat.spam(false) << *stop;
    }
    wrt_cat.spam(false) << " is:\n";
    result.write(wrt_cat.spam(false), 2);
  }

  return stop;
}

#endif

template<class TransitionWrapper>
Node *
cached_wrt_subtree(NodeRelation *arc, Node *to, UpdateSeq as_of, UpdateSeq now,
		   TransitionWrapper &result, TypeHandle graph_type) {
  // First, determine the net transition up to the top of the current
  // subtree.
  Node *top_subtree;
  get_cached_net_transition(arc, top_subtree, as_of, now, result, graph_type);

  if (top_subtree == to || to == (Node *)NULL) {
    // If the top of the subtree is the node we asked to wrt to,
    // excellent!  Stop here.

#ifndef NDEBUG
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "cached_wrt_subtree(" << *arc << ", ";
      if (to == (Node *)NULL) {
	wrt_cat.spam(false) << "(top)";
      } else {
	wrt_cat.spam(false) << *to;
      }
      wrt_cat.spam(false) << ") stops at top, result is:\n";
      result.write(wrt_cat.spam(false), 2);
    }
#endif

  } else {

    // Otherwise, it must be the case that the node we want to wrt to is
    // some descendent of the top_subtree node.  It also therefore
    // follows that this node has exactly one parent.

#ifndef NDEBUG
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "cached_wrt_subtree(" << *arc << ", " << *to << ") stops at ";
      if (top_subtree == (Node *)NULL) {
	wrt_cat.spam(false) << "(top)";
      } else {
	wrt_cat.spam(false) << *top_subtree;
      }
      wrt_cat.spam(false) << ", first result is:\n";
      result.write(wrt_cat.spam(false), 2);
    }
#endif

    if (to->get_num_parents(graph_type) != 1) {
      // Oops!  We *don't* have exactly one parent.  This means our
      // cache is stale in some surprising way.  To recover, we'll
      // force a recompute of the cache by asking for the wrt() from
      // the very latest timestamp.

      // This, of course, assumes we haven't just asked for the wrt()
      // from the very latest timestamp.
#ifndef NDEBUG
      if (wrt_cat.is_warning()) {
	wrt_cat.warning()
	  << "Recovering from stale cache.\n";
      }
#endif
      nassertr(as_of != UpdateSeq::fresh(), NULL);
      return cached_wrt_subtree(arc, to, UpdateSeq::fresh(), now,
				result, graph_type);
    }
    
    nassertr(to->get_num_parents(graph_type) == 1, NULL);
    NodeRelation *to_arc = to->get_parent(graph_type, 0);
    
    // Save the result from the first pass.
    TransitionWrapper net_from_trans = result;
    
    // Now determine the net transition to the top of the subtree from
    // this arc.  It had better be the same subtree!
    Node *top_subtree_2;
    get_cached_net_transition(to_arc, top_subtree_2, as_of, now,
			      result, graph_type);
    nassertr(top_subtree == top_subtree_2, NULL);
    
    // And now compute the actual wrt.
    result.invert_compose_in_place(net_from_trans);

#ifndef NDEBUG
    if (wrt_cat.is_spam()) {
      wrt_cat.spam()
	<< "cached_wrt_subtree(" << *arc << ", " << *to << ") stops at ";
      if (top_subtree == (Node *)NULL) {
	wrt_cat.spam(false) << "(top)";
      } else {
	wrt_cat.spam(false) << *top_subtree;
      }
      wrt_cat.spam() << ", second result is:\n";
      net_from_trans.write(wrt_cat.spam(false), 2);
      wrt_cat.spam() << ", final result is:\n";
      result.write(wrt_cat.spam(false), 2);
    }
#endif
  }

#ifndef NDEBUG
  if (paranoid_wrt) {
    // Now check the results.
    TransitionWrapper check_trans =
      TransitionWrapper::init_from(result);
    Node *top_subtree_3 = 
      uncached_wrt_subtree(arc, to, check_trans, graph_type);

    if (top_subtree_3 != top_subtree) {
      wrt_cat.warning()
	<< "WRT subtree cache from " << *arc->get_child() << " to ";
      if (to == (Node *)NULL) {
	wrt_cat.warning(false) << "(top)";
      } else {
	wrt_cat.warning(false) << *to;
      }
      wrt_cat.warning(false)
	<< " computes incorrect top_subtree!\n"
	<< "  computed ";
      if (top_subtree == (Node *)NULL) {
	wrt_cat.warning(false) << "(top)\n";
      } else {
	wrt_cat.warning(false) << *top_subtree << "\n";
      }
      wrt_cat.warning(false)
	<< "  should be ";
      if (top_subtree_3 == (Node *)NULL) {
	wrt_cat.warning(false) << "(top)\n";
      } else {
	wrt_cat.warning(false) << *top_subtree_3 << "\n";
      }
      top_subtree = top_subtree_3;
    }

    int compare = check_trans.compare_to(result);
    if (compare != 0) {
      wrt_cat.warning()
	<< "WRT subtree cache from " << *arc->get_child() << " to ";
      if (to == (Node *)NULL) {
	wrt_cat.warning(false) << "(top)";
      } else {
	wrt_cat.warning(false) << *to;
      }
      wrt_cat.warning(false)
	<< " is invalid!\n"
	<< "  cached value is:\n";
      result.write(wrt_cat.warning(false), 4);
      wrt_cat.warning(false)
	<< "  should be:\n";
      check_trans.write(wrt_cat.warning(false), 4);
      result = check_trans;
    }
  }
#endif

  return top_subtree;
}

template<class TransitionWrapper>
INLINE_GRAPH Node *
wrt_subtree(NodeRelation *arc, Node *to, UpdateSeq as_of, UpdateSeq now,
	    TransitionWrapper &result, TypeHandle graph_type) {
#ifndef NDEBUG
  if (!cache_wrt) {
    // If we aren't caching wrt, compute it explicitly.
    return uncached_wrt_subtree(arc, to, result, graph_type);
  }
#endif
  return cached_wrt_subtree(arc, to, as_of, now, result, graph_type);
}


