// Filename: nodeTransitionWrapper.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODETRANSITIONWRAPPER_H
#define NODETRANSITIONWRAPPER_H

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
class NodeAttributeWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : NodeTransitionWrapper
// Description : This is a wrapper around a single transition type,
//               selected at runtime by its handle.  It is useful when
//               computing a wrt() based on one transition type only
//               (for instance, a MatrixTransition).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransitionWrapper {
public:
  typedef NodeTransitionWrapper TransitionWrapper;
  typedef NodeAttributeWrapper AttributeWrapper;

  INLINE_GRAPH NodeTransitionWrapper(TypeHandle handle);
  INLINE_GRAPH NodeTransitionWrapper(const NodeTransitionWrapper &copy);
  INLINE_GRAPH void operator = (const NodeTransitionWrapper &copy);

  INLINE_GRAPH static NodeTransitionWrapper 
  init_from(const NodeTransitionWrapper &other);
  static NodeTransitionWrapper init_from(const NodeAttributeWrapper &attrib);

  INLINE_GRAPH TypeHandle get_handle() const;
  INLINE_GRAPH bool has_trans() const;
  INLINE_GRAPH NodeTransition *get_trans() const;

  INLINE_GRAPH bool is_identity() const;
  INLINE_GRAPH int compare_to(const NodeTransitionWrapper &other) const;

  INLINE_GRAPH void make_identity();
  INLINE_GRAPH void extract_from(const NodeRelation *arc);
  INLINE_GRAPH void store_to(NodeRelation *arc) const;

  INLINE_GRAPH void compose_in_place(const NodeTransitionWrapper &other);
  INLINE_GRAPH void invert_in_place();
  INLINE_GRAPH void invert_compose_in_place(const NodeTransitionWrapper &other);

  INLINE_GRAPH Node *extract_from_cache(const NodeRelation *arc);
  INLINE_GRAPH void store_to_cache(NodeRelation *arc, Node *top_subtree);
  INLINE_GRAPH bool is_cache_verified(UpdateSeq as_of) const;
  INLINE_GRAPH void set_computed_verified(UpdateSeq now);

  INLINE_GRAPH void cached_compose(const NodeTransitionWrapper &cache, 
			     const NodeTransitionWrapper &value,
			     UpdateSeq now);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  TypeHandle _handle;
  NodeTransitionCacheEntry _entry;
  friend class NodeAttributeWrapper;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitionWrapper &ntw);

#ifndef DONT_INLINE_GRAPH
#include "nodeTransitionWrapper.I"
#endif

#include "nodeTransitionWrapper.T"

#endif
