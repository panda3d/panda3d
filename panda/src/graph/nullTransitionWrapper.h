// Filename: nullTransitionWrapper.h
// Created by:  drose (22Mar00)
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

#ifndef NULLTRANSITIONWRAPPER_H
#define NULLTRANSITIONWRAPPER_H

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
#include "nodeTransitionCacheEntry.h"

class Node;
class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : NullTransitionWrapper
// Description : This is a wrapper around *no* transitions at all.  It
//               does absolutely nothing.  It's pointless to pass to
//               wrt(), but it's useful for passing to df_traverse()
//               to perform a traversal without bothering to keep
//               track of state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullTransitionWrapper {
public:
  typedef NullTransitionWrapper TransitionWrapper;

  INLINE_GRAPH NullTransitionWrapper();
  INLINE_GRAPH NullTransitionWrapper(const NullTransitionWrapper &copy);
  INLINE_GRAPH void operator = (const NullTransitionWrapper &copy);

  INLINE_GRAPH static NullTransitionWrapper
  init_from(const NullTransitionWrapper &other);

  INLINE_GRAPH bool is_identity() const;
  INLINE_GRAPH int compare_to(const NullTransitionWrapper &other) const;

  INLINE_GRAPH void make_identity();
  INLINE_GRAPH void extract_from(const NodeRelation *arc);
  INLINE_GRAPH void store_to(NodeRelation *arc) const;

  INLINE_GRAPH void compose_in_place(const NullTransitionWrapper &other);
  INLINE_GRAPH void invert_in_place();
  INLINE_GRAPH void invert_compose_in_place(const NullTransitionWrapper &other);

  INLINE_GRAPH Node *extract_from_cache(const NodeRelation *arc);
  INLINE_GRAPH void store_to_cache(NodeRelation *arc, Node *top_subtree);
  INLINE_GRAPH bool is_cache_verified(UpdateSeq now) const;
  INLINE_GRAPH void set_computed_verified(UpdateSeq now);

  INLINE_GRAPH void cached_compose(const NullTransitionWrapper &cache,
                             const NullTransitionWrapper &value,
                             UpdateSeq now);

  INLINE_GRAPH void output(ostream &out) const;
  INLINE_GRAPH void write(ostream &out, int indent_level = 0) const;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NullTransitionWrapper &ntw);

#ifndef DONT_INLINE_GRAPH
#include "nullTransitionWrapper.I"
#endif

#endif

