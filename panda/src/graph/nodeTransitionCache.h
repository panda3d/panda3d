// Filename: nodeTransitionCache.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODETRANSITIONCACHE_H
#define NODETRANSITIONCACHE_H

#include <pandabase.h>

#include "nodeTransitionCacheEntry.h"
#include "graphHashGenerator.h"

#include <referenceCount.h>

#include <map>

////////////////////////////////////////////////////////////////////
// 	 Class : NodeTransitionCache
// Description : This represents the cached values of accumulated
//               NodeTransitions values, associated with a particular
//               arc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransitionCache : public ReferenceCount {
public:
  typedef GraphHashGenerator HashGenerator;

  NodeTransitionCache();
  NodeTransitionCache(const NodeTransitions &nt);
  ~NodeTransitionCache();

  bool is_identity() const;
  int compare_to(const NodeTransitionCache &other) const;
  void generate_hash(GraphHashGenerator &hash) const;

  bool is_empty() const;
  PT(NodeTransition) set_transition(TypeHandle handle, NodeTransition *trans);
  INLINE_GRAPH PT(NodeTransition) set_transition(NodeTransition *trans);
  PT(NodeTransition) clear_transition(TypeHandle handle);
  bool has_transition(TypeHandle handle) const;
  NodeTransition *get_transition(TypeHandle handle) const;

  void clear();

  bool
  lookup_entry(TypeHandle handle, NodeTransitionCacheEntry &entry) const;

  void
  store_entry(TypeHandle handle, const NodeTransitionCacheEntry &entry);

private:
  typedef map<TypeHandle, NodeTransitionCacheEntry> Cache;
public:
  // STL-like definitions to allow read-only traversal of the
  // individual transitions.  Note that each of these is a
  // NodeTransitionCacheEntry, not simply a PT(NodeTransition).
  // Beware!  These are not safe to use outside of PANDA.DLL.
  typedef Cache::const_iterator iterator;
  typedef Cache::const_iterator const_iterator;
  typedef Cache::value_type value_type;
  typedef Cache::size_type size_type;

  INLINE_GRAPH size_type size() const;
  INLINE_GRAPH const_iterator begin() const;
  INLINE_GRAPH const_iterator end() const;


public:
  // The following functions are support functions for
  // AllTransitionsWrapper; they have limited utility elsewhere.

  static void
  store_to(const NodeTransitionCache *a, NodeRelation *arc, 
	   NodeTransitions &nt);

  static NodeTransitionCache *
  c_union(const NodeTransitionCache *a, const NodeTransitionCache *b);

  static NodeTransitionCache *
  compose(const NodeTransitionCache *a, const NodeTransitionCache *b);

  static NodeTransitionCache *
  invert(const NodeTransitionCache *a);

  static NodeTransitionCache *
  invert_compose(const NodeTransitionCache *a, const NodeTransitionCache *b);

  static NodeTransitionCache *
  cached_compose(const NodeTransitionCache *a, 
		 const NodeTransitionCache *cache,
		 const NodeTransitionCache *b,
		 UpdateSeq now);

  static NodeTransitionCache *
  set_computed_verified(const NodeTransitionCache *a, UpdateSeq now);


public:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  Cache _cache;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "NodeTransitionCache",
		  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class NodeAttributes;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitionCache &ntc);

#ifndef DONT_INLINE_GRAPH
#include "nodeTransitionCache.I"
#endif

#endif
