// Filename: nodeTransitionCache.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransitionCache.h"
#include "nodeTransitions.h"
#include "config_graph.h"
#include "setTransitionHelpers.h"

#include <indent.h>

TypeHandle NodeTransitionCache::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransitionCache::
NodeTransitionCache() {
  MemoryUsage::update_type(this, get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::Constructor from NodeTransitions
//       Access: Public
//  Description: Adds an entry into the cache for each non-identity
//               Transition in the NodeTransitions set.
////////////////////////////////////////////////////////////////////
NodeTransitionCache::
NodeTransitionCache(const NodeTransitions &nt) {
  MemoryUsage::update_type(this, get_class_type());

  NodeTransitions::Transitions::const_iterator ti;
  for (ti = nt._transitions.begin();
       ti != nt._transitions.end();
       ++ti) {
    TypeHandle handle = (*ti).first;
    NodeTransition *trans = (*ti).second;

    if (trans != (NodeTransition *)NULL) {
      _cache[handle] = NodeTransitionCacheEntry(trans);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransitionCache::
~NodeTransitionCache() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::is_identity
//       Access: Public
//  Description: Returns true if all of the transitions in the cache
//               are the identity transition.
////////////////////////////////////////////////////////////////////
bool NodeTransitionCache::
is_identity() const {
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    if (!(*ci).second.is_identity()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::compare_to
//       Access: Public
//  Description: This only compares the set of transitions in the
//               cache; it does not compare timestamps.
////////////////////////////////////////////////////////////////////
int NodeTransitionCache::
compare_to(const NodeTransitionCache &other) const {
   return tmap_compare_cache(_cache.begin(), _cache.end(),
			     other._cache.begin(), other._cache.end());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::generate_hash
//       Access: Public
//  Description: Adds the transitions to the indicated hash generator.
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
generate_hash(GraphHashGenerator &hash) const {
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    (*ci).second.generate_hash(hash);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::is_empty
//       Access: Public
//  Description: Returns true if there are no Transitions stored in
//               the cache, or false if there are any (even identity)
//               Transitions.
////////////////////////////////////////////////////////////////////
bool NodeTransitionCache::
is_empty() const {
  return _cache.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a specific
//               TypeHandle, indicating the type of transition that we
//               are setting, and a NodeTransition pointer indicating
//               the value of the transition.  The NodeTransition may
//               be NULL indicating that the transition should be
//               cleared.  If the NodeTransition is not NULL, it must
//               match the type indicated by the TypeHandle.
//
//               The return value is a pointer to the *previous*
//               transition in the cache, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) NodeTransitionCache::
set_transition(TypeHandle handle, NodeTransition *trans) {
  if (trans == (NodeTransition *)NULL) {
    return clear_transition(handle);

  } else {
    Cache::iterator ci;
    ci = _cache.find(handle);
    if (ci != _cache.end()) {
      PT(NodeTransition) result = (*ci).second.get_trans();
      (*ci).second = NodeTransitionCacheEntry(trans);
      return result;
    }

    _cache.insert(Cache::value_type(handle, NodeTransitionCacheEntry(trans)));
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) NodeTransitionCache::
clear_transition(TypeHandle handle) {
  nassertr(handle != TypeHandle::none(), NULL);

  Cache::iterator ci;
  ci = _cache.find(handle);
  if (ci != _cache.end()) {
    PT(NodeTransition) result = (*ci).second.get_trans();
    _cache.erase(ci);
    return result;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::has_transition
//       Access: Public
//  Description: Returns true if a transition associated with the
//               indicated handle has been stored in the cache, or
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool NodeTransitionCache::
has_transition(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), false);
  Cache::const_iterator ci;
  ci = _cache.find(handle);
  if (ci != _cache.end()) {
    return (*ci).second.has_trans();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::get_transition
//       Access: Public
//  Description: Returns the transition associated with the indicated
//               handle, or NULL if no such transition has been stored
//               in the cache.
////////////////////////////////////////////////////////////////////
NodeTransition *NodeTransitionCache::
get_transition(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), NULL);
  Cache::const_iterator ci;
  ci = _cache.find(handle);
  if (ci != _cache.end()) {
    return (*ci).second.get_trans();
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::clear
//       Access: Public
//  Description: Blows away all entries in the cache. 
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
clear() {
  _cache.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::lookup_entry
//       Access: Public
//  Description: Looks up the indicated entry in the cache.  If the
//               entry is defined in the cache, copies it to the
//               'entry' parameter and returns true; otherwise,
//               reinitializes the 'entry' parameter and returns
//               false.
////////////////////////////////////////////////////////////////////
bool NodeTransitionCache::
lookup_entry(TypeHandle handle, NodeTransitionCacheEntry &entry) const {
  Cache::const_iterator ci;
  ci = _cache.find(handle);
  if (ci != _cache.end()) {
    entry = (*ci).second;
    return true;
  }

  entry.clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::store_entry
//       Access: Public
//  Description: Adds or updates an entry in the cache for a
//               particular Transition type.
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
store_entry(TypeHandle handle, const NodeTransitionCacheEntry &entry) {
  _cache[handle] = entry;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::store_to
//       Access: Public, Static
//  Description: Adds a transition to the NodeTransitions set for each
//               entry in the cache, and tells the transition that it
//               has been added to the indicated arc.
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
store_to(const NodeTransitionCache *a, NodeRelation *arc, 
	 NodeTransitions &nt) {
  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty: no change.
    return;
  }

  // First, tell all of the entries in the cache that they're about
  // the be added to the arc.
  Cache::const_iterator ci;
  for (ci = a->_cache.begin(); ci != a->_cache.end(); ++ci) {
    NodeTransition *trans = (*ci).second.get_trans();
    trans->added_to_arc(arc);
  }

  // And now actually add them.
  NodeTransitions temp;
  tmap_override_union(nt._transitions.begin(), nt._transitions.end(),
		      a->_cache.begin(), a->_cache.end(),
		      inserter(temp._transitions, temp._transitions.begin()));
  nt._transitions.swap(temp._transitions);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::union
//       Access: Public, Static
//  Description: Returns a pointer to a new NodeTransitionCache (or
//               perhaps a pointer to one of the two source caches)
//               that represents the union of the two caches.  Either
//               or both pointers may be NULL, indicating identity,
//               and the return value might also be NULL.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
c_union(const NodeTransitionCache *a, const NodeTransitionCache *b) {
  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty, therefore identity: the result is the same as b.
    return (NodeTransitionCache *)b;

  } else if (b == (NodeTransitionCache *)NULL || b->_cache.empty()) {
    // b is empty, therefore identity: the result is the same as a.
    return (NodeTransitionCache *)a;

  } else {
    // Neither is empty.  Build and return a new list.
    NodeTransitionCache *result = new NodeTransitionCache;

    tmap_override_union(a->_cache.begin(), a->_cache.end(),
			b->_cache.begin(), b->_cache.end(),
			inserter(result->_cache, result->_cache.begin()));

    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::compose
//       Access: Public, Static
//  Description: Returns a pointer to a new NodeTransitionCache (or
//               perhaps a pointer to one of the two source caches)
//               that represents the memberwise composition of the two
//               caches.  Either or both pointers may be NULL,
//               indicating identity, and the return value might also
//               be NULL.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
compose(const NodeTransitionCache *a, const NodeTransitionCache *b) {
  // Since the absence of a transition from the object implies the
  // identity transition, we can make a quick special case for either
  // object being empty:

  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty, therefore identity: the result is the same as b.
    return (NodeTransitionCache *)b;

  } else if (b == (NodeTransitionCache *)NULL || b->_cache.empty()) {
    // b is empty, therefore identity: the result is the same as a.
    return (NodeTransitionCache *)a;

  } else {
    // Neither is empty.  Build and return a new list.
    NodeTransitionCache *result = new NodeTransitionCache;

    tmap_compose(a->_cache.begin(), a->_cache.end(),
		 b->_cache.begin(), b->_cache.end(),
		 inserter(result->_cache, result->_cache.begin()));

    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::invert
//       Access: Public, Static
//  Description: Returns a pointer to a new NodeTransitionCache (or
//               perhaps a pointer to the source caches) that
//               represents the memberwise inversion of the caches.
//               The source pointer may be NULL, indicating identity,
//               and the return value might also be NULL.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
invert(const NodeTransitionCache *a) {
  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty, therefore identity: the result is identity.
    return NULL;
  }

  // Build and return a new list.
  NodeTransitionCache *result = new NodeTransitionCache;

  tmap_invert(a->_cache.begin(), a->_cache.end(),
	      inserter(result->_cache, result->_cache.begin()));
  
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::invert_compose
//       Access: Public, Static
//  Description: Returns a pointer to a new NodeTransitionCache (or
//               perhaps a pointer to one of the two source caches)
//               that represents the memberwise invert-compose of the
//               two caches.  Either or both pointers may be NULL,
//               indicating identity, and the return value might also
//               be NULL.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
invert_compose(const NodeTransitionCache *a, const NodeTransitionCache *b) {
  // Since the absence of a transition from the object implies the
  // identity transition, we can make a quick special case for either
  // object being empty:

  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty, therefore identity: the result is the same as b.
    return (NodeTransitionCache *)b;

  } else if (b == (NodeTransitionCache *)NULL || b->_cache.empty()) {
    // b is empty, therefore identity: the result is the inverse of a.
    return invert(a);

  } else {
    // Neither is empty.  Build and return a new list.
    NodeTransitionCache *result = new NodeTransitionCache;

    tmap_invert_compose(a->_cache.begin(), a->_cache.end(),
			b->_cache.begin(), b->_cache.end(),
			inserter(result->_cache, result->_cache.begin()));

    return result;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::cached_compose
//       Access: Public, Static
//  Description: Returns a cache pointer (which might be a pointer to
//               the same cache object, or to a newly allocated
//               object) that represents the result of compose(a, b),
//               as computed using the cache value as a hint.  Mark
//               the result as computed at time 'now'.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
cached_compose(const NodeTransitionCache *a, 
	       const NodeTransitionCache *cache,
	       const NodeTransitionCache *b,
	       UpdateSeq now) {
  // Since the absence of a transition from the object implies the
  // identity transition, we can make a quick special case for either
  // object being empty:

  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    // a is empty, therefore identity: the result is the same as b.
    return set_computed_verified(b, now);

  } else if (b == (NodeTransitionCache *)NULL || b->_cache.empty()) {
    // b is empty, therefore identity: the result is the same as a.
    return set_computed_verified(a, now);

  } else {
    // Neither is empty.  Build and return a new list.
    NodeTransitionCache *result = new NodeTransitionCache;

    if (cache == (NodeTransitionCache *)NULL) {
      // If the cache is NULL, we must send in an empty list for the
      // cache list.  We'll use [a.begin .. a.begin).
      tmap_cached_compose(a->_cache.begin(), a->_cache.end(),
			  a->_cache.begin(), a->_cache.begin(),
			  b->_cache.begin(), b->_cache.end(),
			  now,
			  inserter(result->_cache, result->_cache.begin()));

    } else {
      tmap_cached_compose(a->_cache.begin(), a->_cache.end(),
			  cache->_cache.begin(), cache->_cache.end(),
			  b->_cache.begin(), b->_cache.end(),
			  now,
			  inserter(result->_cache, result->_cache.begin()));
    }

    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::set_computed_verified
//       Access: Public, Static
//  Description: Returns a new pointer that represents the adjustment
//               of the given cache to set each computed and verified
//               value to the current now value.  This may modify the
//               source cache only if there are no other pointers to
//               it.
////////////////////////////////////////////////////////////////////
NodeTransitionCache *NodeTransitionCache::
set_computed_verified(const NodeTransitionCache *a, UpdateSeq now) {
  if (a == (NodeTransitionCache *)NULL || a->_cache.empty()) {
    return NULL;
  }

  NodeTransitionCache *result = (NodeTransitionCache *)a;
  if (a->get_ref_count() > 1) {
    // Copy-on-write.
    result = new NodeTransitionCache(*a);
  }

  Cache::iterator ci;
  for (ci = result->_cache.begin(); ci != result->_cache.end(); ++ci) {
    (*ci).second.set_computed_verified(now);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
output(ostream &out) const {
  bool written_any = false;
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    if (!(*ci).second.is_identity()) {
      if (written_any) {
	out << " ";
      }
      out << (*ci).second;
      written_any = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionCache::
write(ostream &out, int indent_level) const {
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    indent(out, indent_level) << (*ci).first << "\n";
    (*ci).second.write(out, indent_level + 2);
  }
}
