// Filename: nodeTransitionCacheEntry.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODETRANSITIONCACHEENTRY_H
#define NODETRANSITIONCACHEENTRY_H

#include <pandabase.h>

#include "nodeTransition.h"
#include "config_graph.h"

#include <updateSeq.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// 	 Class : NodeTransitionCacheEntry
// Description : This is a single cached accumulated NodeTransition
//               value, representing the net Transition for one
//               particular transition type on an arc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransitionCacheEntry {
public:
  INLINE_GRAPH NodeTransitionCacheEntry(NodeTransition *trans = NULL);
  INLINE_GRAPH NodeTransitionCacheEntry(const NodeTransitionCacheEntry &copy);
  INLINE_GRAPH void operator = (const NodeTransitionCacheEntry &copy);

  INLINE_GRAPH bool is_identity() const;
  INLINE_GRAPH int compare_to(const NodeTransitionCacheEntry &other) const;

  INLINE_GRAPH void clear();

  INLINE_GRAPH void set_trans(NodeTransition *trans);
  INLINE_GRAPH void clear_trans();
  INLINE_GRAPH bool has_trans() const;
  INLINE_GRAPH NodeTransition *get_trans() const;

  INLINE_GRAPH bool is_cache_verified(UpdateSeq as_of) const;
  INLINE_GRAPH bool is_freshly_computed(UpdateSeq changed) const;
  INLINE_GRAPH void set_computed_verified(UpdateSeq now);

  INLINE_GRAPH operator const PT(NodeTransition) &() const;

  INLINE_GRAPH UpdateSeq get_computed() const;
  INLINE_GRAPH UpdateSeq get_verified() const;

public:
  // The following functions perform the basic transition operations
  // on the entry's _trans pointer.  In general, they don't bother to
  // keep the _computed and _verified members consistent across these
  // operations.

  INLINE_GRAPH static NodeTransitionCacheEntry 
  invert(const NodeTransitionCacheEntry &a);

  INLINE_GRAPH static NodeTransitionCacheEntry
  compose(const NodeTransitionCacheEntry &a, 
	  const NodeTransitionCacheEntry &b);

  INLINE_GRAPH static NodeTransitionCacheEntry
  invert_compose(const NodeTransitionCacheEntry &a, 
		 const NodeTransitionCacheEntry &b);

  INLINE_GRAPH static NodeTransitionCacheEntry
  cached_compose(const NodeTransitionCacheEntry &a, 
		 const NodeTransitionCacheEntry &cache, 
		 const NodeTransitionCacheEntry &b,
		 UpdateSeq now);

  INLINE_GRAPH static NodeAttribute *
  apply(const NodeAttribute *a, const NodeTransitionCacheEntry &b);

  INLINE_GRAPH NodeAttribute *make_attrib() const;

public:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  PT(NodeTransition) _trans;
  UpdateSeq _computed;
  UpdateSeq _verified;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitionCacheEntry &e);

#ifndef DONT_INLINE_GRAPH
#include "nodeTransitionCacheEntry.I"
#endif

#endif
