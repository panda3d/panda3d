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
  INLINE NodeTransitionCacheEntry(NodeTransition *trans = NULL);
  INLINE NodeTransitionCacheEntry(const NodeTransitionCacheEntry &copy);
  INLINE void operator = (const NodeTransitionCacheEntry &copy);

  INLINE bool is_identity() const;
  INLINE int compare_to(const NodeTransitionCacheEntry &other) const;

  INLINE void clear();

  INLINE void set_trans(NodeTransition *trans);
  INLINE void clear_trans();
  INLINE bool has_trans() const;
  INLINE NodeTransition *get_trans() const;

  INLINE bool is_cache_verified(UpdateSeq as_of) const;
  INLINE bool is_freshly_computed(UpdateSeq changed) const;
  INLINE void set_computed_verified(UpdateSeq now);

  INLINE operator const PT(NodeTransition) &() const;

  INLINE UpdateSeq get_computed() const;
  INLINE UpdateSeq get_verified() const;

public:
  // The following functions perform the basic transition operations
  // on the entry's _trans pointer.  In general, they don't bother to
  // keep the _computed and _verified members consistent across these
  // operations.

  INLINE static NodeTransitionCacheEntry 
  invert(const NodeTransitionCacheEntry &a);

  INLINE static NodeTransitionCacheEntry
  compose(const NodeTransitionCacheEntry &a, 
	  const NodeTransitionCacheEntry &b);

  INLINE static NodeTransitionCacheEntry
  invert_compose(const NodeTransitionCacheEntry &a, 
		 const NodeTransitionCacheEntry &b);

  INLINE static NodeTransitionCacheEntry
  cached_compose(const NodeTransitionCacheEntry &a, 
		 const NodeTransitionCacheEntry &cache, 
		 const NodeTransitionCacheEntry &b,
		 UpdateSeq now);

  INLINE static NodeAttribute *
  apply(const NodeAttribute *a, const NodeTransitionCacheEntry &b);

  INLINE NodeAttribute *make_attrib() const;

public:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  PT(NodeTransition) _trans;
  UpdateSeq _computed;
  UpdateSeq _verified;
};

INLINE ostream &operator << (ostream &out, const NodeTransitionCacheEntry &e) {
  e.output(out);
  return out;
}

#include "nodeTransitionCacheEntry.I"

#endif
