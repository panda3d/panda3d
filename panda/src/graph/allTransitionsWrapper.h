// Filename: allTransitionsWrapper.h
// Created by:  drose (21Mar00)
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

#ifndef ALLTRANSITIONSWRAPPER_H
#define ALLTRANSITIONSWRAPPER_H

//
// There are several flavors of TransitionWrappers.  These are classes
// that represent one or a number of transitions simultaneously and
// are passed to template functions like df_traverse() and wrt() so
// that the same functions can be used to operate on either one
// transition type or a number of them.
//
// Since these classes are used as template parameters, they do not
// need to use inheritance or virtual functions; but they all must
// have a similar interface.
//

#include <pandabase.h>

#include "nodeTransition.h"
#include "nodeTransitionCache.h"
#include "config_graph.h"
#include "graphHashGenerator.h"

#include <pointerTo.h>

class Node;
class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : AllTransitionsWrapper
// Description : This wrapper represents all transitions that might be
//               stored on arcs.  It is especially useful when
//               performing a traversal or computing a wrt() and you
//               want to determine the complete state at a given node.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AllTransitionsWrapper {
public:
  typedef AllTransitionsWrapper TransitionWrapper;
  typedef GraphHashGenerator HashGenerator;

  INLINE_GRAPH AllTransitionsWrapper();
  INLINE_GRAPH AllTransitionsWrapper(const AllTransitionsWrapper &copy);
  INLINE_GRAPH void operator = (const AllTransitionsWrapper &copy);
  INLINE_GRAPH ~AllTransitionsWrapper();

  INLINE_GRAPH static AllTransitionsWrapper
  init_from(const AllTransitionsWrapper &other);

  INLINE_GRAPH bool is_empty() const;
  PT(NodeTransition) set_transition(TypeHandle handle,
                                    NodeTransition *trans);
  INLINE_GRAPH PT(NodeTransition) set_transition(NodeTransition *trans);
  PT(NodeTransition) clear_transition(TypeHandle handle);
  INLINE_GRAPH bool has_transition(TypeHandle handle) const;
  INLINE_GRAPH NodeTransition *get_transition(TypeHandle handle) const;

  INLINE_GRAPH void clear();

  INLINE_GRAPH const NodeTransitionCache &get_transitions() const;

  INLINE_GRAPH bool is_identity() const;
  INLINE_GRAPH int compare_to(const AllTransitionsWrapper &other) const;
  INLINE_GRAPH void generate_hash(GraphHashGenerator &hashgen) const;

  INLINE_GRAPH void make_identity();
  INLINE_GRAPH void extract_from(const NodeRelation *arc);
  INLINE_GRAPH void store_to(NodeRelation *arc) const;

  INLINE_GRAPH void compose_in_place(const AllTransitionsWrapper &other);
  INLINE_GRAPH void compose_from(const AllTransitionsWrapper &a,
                                 const AllTransitionsWrapper &b);
  INLINE_GRAPH void invert_in_place();
  INLINE_GRAPH void invert_compose_in_place(const AllTransitionsWrapper &other);

  INLINE_GRAPH Node *extract_from_cache(const NodeRelation *arc);
  INLINE_GRAPH void store_to_cache(NodeRelation *arc, Node *top_subtree);
  INLINE_GRAPH void store_to_cache_partial(NodeRelation *arc, Node *top_subtree);
  INLINE_GRAPH bool is_cache_verified(UpdateSeq as_of) const;
  INLINE_GRAPH void set_computed_verified(UpdateSeq now);

  INLINE_GRAPH void cached_compose(const AllTransitionsWrapper &cache,
                                   const AllTransitionsWrapper &value,
                                   UpdateSeq now);

public:
  // STL-like definitions to allow read-only traversal of the
  // individual transitions.  Note that each of these is a
  // NodeTransitionCacheEntry, not simply a PT(NodeTransition).
  // Beware!  It is not safe to use this interface outside of
  // PANDA.DLL.
  typedef NodeTransitionCache::const_iterator iterator;
  typedef NodeTransitionCache::const_iterator const_iterator;
  typedef NodeTransitionCache::value_type value_type;
  typedef NodeTransitionCache::size_type size_type;

  INLINE_GRAPH size_type size() const;
  INLINE_GRAPH const_iterator begin() const;
  INLINE_GRAPH const_iterator end() const;

public:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  PT(NodeTransitionCache) _cache;
  UpdateSeq _all_verified;

  // This is a special cache object which is always around and always
  // empty.  It's used just so we can have a sensible return value
  // from begin() and end() when our pointer is NULL.
  static NodeTransitionCache _empty_cache;
};

EXPCL_PANDA INLINE_GRAPH ostream &operator << (ostream &out, const AllTransitionsWrapper &ntw);

#include "allTransitionsWrapper.T"

#ifndef DONT_INLINE_GRAPH
#include "allTransitionsWrapper.I"
#endif

#endif
