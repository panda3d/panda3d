// Filename: allTransitionsWrapper.h
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ALLTRANSITIONSWRAPPER_H
#define ALLTRANSITIONSWRAPPER_H

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
#include "nodeTransitionCache.h"
#include "config_graph.h"

#include <pointerTo.h>

class Node;
class NodeRelation;
class NodeAttribute;
class AllAttributesWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : AllTransitionsWrapper
// Description : This wrapper represents all transitions that might be
//               stored on arcs.  It is especially useful when
//               performing a traversal or computing a wrt() and you
//               want to determine the complete state at a given node.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AllTransitionsWrapper {
public:
  typedef AllTransitionsWrapper TransitionWrapper;
  typedef AllAttributesWrapper AttributeWrapper;

  INLINE AllTransitionsWrapper();
  INLINE AllTransitionsWrapper(const AllTransitionsWrapper &copy);
  INLINE void operator = (const AllTransitionsWrapper &copy);
  INLINE ~AllTransitionsWrapper();

  INLINE static AllTransitionsWrapper 
  init_from(const AllTransitionsWrapper &other);
  INLINE static AllTransitionsWrapper 
  init_from(const AllAttributesWrapper &attrib);

  INLINE bool is_empty() const;
  INLINE PT(NodeTransition) set_transition(TypeHandle handle,
					   NodeTransition *trans);
  INLINE PT(NodeTransition) set_transition(NodeTransition *trans);
  INLINE PT(NodeTransition) clear_transition(TypeHandle handle);
  INLINE bool has_transition(TypeHandle handle) const;
  INLINE NodeTransition *get_transition(TypeHandle handle) const;

  INLINE const NodeTransitionCache &get_transitions() const;

  INLINE bool is_identity() const;
  INLINE int compare_to(const AllTransitionsWrapper &other) const;

  INLINE void make_identity();
  INLINE void extract_from(const NodeRelation *arc);
  INLINE void store_to(NodeRelation *arc) const;

  INLINE void compose_in_place(const AllTransitionsWrapper &other);
  INLINE void invert_in_place();
  INLINE void invert_compose_in_place(const AllTransitionsWrapper &other);

  INLINE Node *extract_from_cache(const NodeRelation *arc);
  INLINE void store_to_cache(NodeRelation *arc, Node *top_subtree);
  INLINE void store_to_cache_partial(NodeRelation *arc, Node *top_subtree);
  INLINE bool is_cache_verified(UpdateSeq as_of) const;
  INLINE void set_computed_verified(UpdateSeq now);

  INLINE void cached_compose(const AllTransitionsWrapper &cache, 
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

  INLINE size_type size() const;
  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

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
friend class AllAttributesWrapper;
};

INLINE ostream &operator << (ostream &out, const AllTransitionsWrapper &ntw) {
  ntw.output(out);
  return out;
}

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const AllTransitionsWrapper &trans,
		    TypeHandle transition_type);

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const AllTransitionsWrapper &trans);

#include "allTransitionsWrapper.I"

#endif
