// Filename: nullTransitionWrapper.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NULLTRANSITIONWRAPPER_H
#define NULLTRANSITIONWRAPPER_H

//
// There are several flavors of TransitionWrappers (and their
// corresponding AttributeWrappers).  These are classes that represent
// one or a number of transitions (or attributes) simultaneously and
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
class NodeAttribute;
class NullAttributeWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : NullTransitionWrapper
// Description : This is a wrapper around *no* transitions at all.  It
//               does absolutely nothing.  It's pointless to pass to
//               wrt(), but it's useful for passing to df_traverse()
//               to perform a traversal without bothering to keep
//               track of state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullTransitionWrapper {
public:
  typedef NullTransitionWrapper TransitionWrapper;
  typedef NullAttributeWrapper AttributeWrapper;

  INLINE NullTransitionWrapper();
  INLINE NullTransitionWrapper(const NullTransitionWrapper &copy);
  INLINE void operator = (const NullTransitionWrapper &copy);

  INLINE static NullTransitionWrapper 
  init_from(const NullTransitionWrapper &other);
  INLINE static NullTransitionWrapper
  init_from(const NullAttributeWrapper &attrib);

  INLINE bool is_identity() const;
  INLINE int compare_to(const NullTransitionWrapper &other) const;

  INLINE void make_identity();
  INLINE void extract_from(const NodeRelation *arc);
  INLINE void store_to(NodeRelation *arc) const;

  INLINE void compose_in_place(const NullTransitionWrapper &other);
  INLINE void invert_in_place();
  INLINE void invert_compose_in_place(const NullTransitionWrapper &other);

  INLINE Node *extract_from_cache(const NodeRelation *arc);
  INLINE void store_to_cache(NodeRelation *arc, Node *top_subtree);
  INLINE bool is_cache_verified(UpdateSeq now) const;
  INLINE void set_computed_verified(UpdateSeq now);

  INLINE void cached_compose(const NullTransitionWrapper &cache, 
			     const NullTransitionWrapper &value,
			     UpdateSeq now);

  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;
};

INLINE ostream &operator << (ostream &out, const NullTransitionWrapper &ntw) {
  ntw.output(out);
  return out;
}

#include "nullTransitionWrapper.I"

#endif

