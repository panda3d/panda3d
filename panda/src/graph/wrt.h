// Filename: wrt.h
// Created by:  drose (26Oct98)
//
////////////////////////////////////////////////////////////////////

#ifndef WRT_H
#define WRT_H

#include <pandabase.h>

#include "nodeRelation.h"
#include "node.h"
#include "config_graph.h"

#include <typeHandle.h>

class Node;

// The normal wrt() call, with-respect-to, computes the transition
// necessary to convert from the state at "to" to the state at "from".
// (If this seems backwards, think of it as returning the state at
// "from" with respect to the node "to", or in other words, the node
// "from" as seen from "to".)

// It's most useful for TransformTransitions, but it is meaningful to
// use wrt() with any kind of NodeTransition at all.

// The various additional flavors of wrt() are provided to resolve
// ambiguities in the case of a node having multiple parents in the
// scene graph.  If a given node in the ancestry of node "from" has
// multiple parents, and one of those parents is in the set
// from_arcs_begin..from_arcs_end, that arc is chosen.  Otherwise, a
// warning is issued and an arc is chosen arbitrarily.  Similarly for
// multiple parenthood in the ancestry of node "to" and
// to_arcs_begin..to_arcs_end.

template<class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from, const Node *to,
    TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    InputIterator from_arcs_begin, InputIterator from_arcs_end,
    const Node *to,
    TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    const Node *to,
    InputIterator to_arcs_begin, InputIterator to_arcs_end,
    TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator1, class InputIterator2, class TransitionWrapper>
INLINE_GRAPH void
wrt(const Node *from,
    InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
    const Node *to,
    InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
    TransitionWrapper &result, TypeHandle graph_type);

#ifndef NDEBUG
// Similar to the above, but always uncached.  Useful mainly for
// debugging, or when you suspect the cache is invalid.  Also note
// that you can configure 'cache-wrt' or 'paranoid-wrt' to disable or
// force verification of the cache implicitly.
template<class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from, const Node *to,
	     TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     InputIterator from_arcs_begin, InputIterator from_arcs_end,
	     const Node *to,
	     TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     const Node *to,
	     InputIterator to_arcs_begin, InputIterator to_arcs_end,
	     TransitionWrapper &result, TypeHandle graph_type);

template<class InputIterator1, class InputIterator2, class TransitionWrapper>
INLINE_GRAPH void
uncached_wrt(const Node *from,
	     InputIterator1 from_arcs_begin, InputIterator1 from_arcs_end,
	     const Node *to,
	     InputIterator2 to_arcs_begin, InputIterator2 to_arcs_end,
	     TransitionWrapper &result, TypeHandle graph_type);
#endif


// The following function is a bit different.  Rather than computing
// the relative transform between two nodes, it computes the net
// transform along the shortest unambiguous path from the indicated
// arc towards the root.  That is, this is the wrt between the child
// of the indicated arc and the closest ancestor node that has
// multiple parents, or the root of the scene graph if the arc only
// appears once in the scene graph.  The return value is the
// particular node with multiple parents at which the wrt stopped, or
// NULL if it went all the way to the root.

// This is extended just a bit further by allowing the user to specify
// a "to" node.  This must be either NULL, or the expected top node,
// or some node in between.  If it is a node in between, it indicates
// the point at which the relative computation should stop.  In
// effect, this makes wrt_subtree(arc, to) equivalent to
// wrt(arc->get_child(), to), except that it must be true that "to" is
// a linear ancestor of arc.

// This concept is important within the computation of wrt itself to
// manage cached state between multiple instances, but is of limited
// general utility; it is of primary interest to code (like the
// CullTraverser) that needs to cache a wrt-type value for many nodes
// across the entire tree.

// as_of is the most recent update between 'arc' and 'to'; now is the
// current most recent update sequence anywhere, used to use to mark
// any computed cache values.
template<class TransitionWrapper>
INLINE_GRAPH Node *
wrt_subtree(NodeRelation *arc, Node *to, UpdateSeq as_of, UpdateSeq now,
	    TransitionWrapper &result, TypeHandle graph_type);


#ifndef DONT_INLINE_GRAPH
#include "wrt.I"
#endif

#endif
