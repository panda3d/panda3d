// Filename: allTransitionsWrapper.I
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeRelation.h"

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::
AllTransitionsWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::
AllTransitionsWrapper(const AllTransitionsWrapper &copy) :
  _cache(copy._cache),
  _all_verified(copy._all_verified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
operator = (const AllTransitionsWrapper &copy) {
  _cache = copy._cache;
  _all_verified = copy._all_verified;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::
~AllTransitionsWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               AllTransitionsWrapper ready to access the same type
//               of NodeTransition as the other.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper AllTransitionsWrapper::
init_from(const AllTransitionsWrapper &) {
  return AllTransitionsWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               AllTransitionsWrapper ready to access the same type
//               of NodeTransition as the other.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper AllTransitionsWrapper::
init_from(const AllAttributesWrapper &) {
  return AllTransitionsWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::is_empty
//       Access: Public
//  Description: Returns true if there are no Transitions stored in
//               the wrapper, or false if there are any (even identity)
//               Transitions.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllTransitionsWrapper::
is_empty() const {
  return 
    (_cache == (NodeTransitionCache *)NULL) ||
    _cache->is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::set_transition
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
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) AllTransitionsWrapper::
set_transition(TypeHandle handle, NodeTransition *trans) {
  if (_cache == (NodeTransitionCache *)NULL) {
    _cache = new NodeTransitionCache;
  }
  _all_verified.clear();
  return _cache->set_transition(handle, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a pointer to
//               a NodeTransition only.  It infers the type of the
//               NodeTransition from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_transition; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_transition).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) AllTransitionsWrapper::
set_transition(NodeTransition *trans) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) AllTransitionsWrapper::
clear_transition(TypeHandle handle) {
  if (_cache == (NodeTransitionCache *)NULL) {
    return NULL;
  }
  _all_verified.clear();
  return _cache->clear_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::has_transition
//       Access: Public
//  Description: Returns true if a transition associated with the
//               indicated handle has been stored in the wrapper, or
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllTransitionsWrapper::
has_transition(TypeHandle handle) const {
  if (_cache == (NodeTransitionCache *)NULL) {
    return false;
  }
  return _cache->has_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::get_transition
//       Access: Public
//  Description: Returns the transition associated with the indicated
//               handle, or NULL if no such transition has been stored
//               in the wrapper.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransition *AllTransitionsWrapper::
get_transition(TypeHandle handle) const {
  if (_cache == (NodeTransitionCache *)NULL) {
    return NULL;
  }
  return _cache->get_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::get_transitions
//       Access: Public
//  Description: Returns the entire cache of transitions.  You
//               shouldn't modify this, or probably even store it for
//               any length of time.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const NodeTransitionCache &AllTransitionsWrapper::
get_transitions() const {
  static NodeTransitionCache empty_transitions;

  if (_cache == (NodeTransitionCache *)NULL) {
    return empty_transitions;
  }
  return *_cache;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::is_identity
//       Access: Public
//  Description: Returns true if the wrapper represents an identity
//               transition.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllTransitionsWrapper::
is_identity() const {
  if (_cache == (NodeTransitionCache *)NULL) {
    return true;
  }
  return _cache->is_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int AllTransitionsWrapper::
compare_to(const AllTransitionsWrapper &other) const {
  if (_cache == other._cache) {
    return 0;
  }
  if (_cache == (NodeTransitionCache *)NULL) {
    return other._cache->is_identity() ? 0 : -1;
  }
  if (other._cache == (NodeTransitionCache *)NULL) {
    return _cache->is_identity() ? 0 : 1;
  }

  return _cache->compare_to(*other._cache);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::make_identity
//       Access: Public
//  Description: Resets the wrapper to the empty set.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
make_identity() {
  _cache = (NodeTransitionCache *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::extract_from
//       Access: Public
//  Description: Sets the wrapper to the set of transitions contained
//               on the arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
extract_from(const NodeRelation *arc) {
  nassertv(arc != (NodeRelation *)NULL);
  if (arc->_transitions.is_empty()) {
    _cache = (NodeTransitionCache *)NULL;
  } else {
    _cache = new NodeTransitionCache(arc->_transitions);
  }
  _all_verified.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::store_to
//       Access: Public
//  Description: Stores all the transitions in the wrapper to the arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
store_to(NodeRelation *arc) const {
  nassertv(arc != (NodeRelation *)NULL);
  NodeTransitionCache::store_to(_cache, arc, arc->_transitions);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::compose_in_place
//       Access: Public
//  Description: Sets this transition to the composition of this
//               transition and the following one.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
compose_in_place(const AllTransitionsWrapper &other) {
  _cache = NodeTransitionCache::compose(_cache, other._cache);
  _all_verified.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::invert_in_place
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
invert_in_place() {
  _cache = NodeTransitionCache::invert(_cache);
  _all_verified.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::invert_compose_in_place
//       Access: Public
//  Description: Sets this transition to the composition of this
//               transition and the following one.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
invert_compose_in_place(const AllTransitionsWrapper &other) {
  _cache = NodeTransitionCache::invert_compose(_cache, other._cache);
  _all_verified.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::extract_from_cache
//       Access: Public
//  Description: Sets this wrapper to the set of transitions indicated
//               in the cache on the arc.  Also returns the arc's
//               top_subtree indication.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH Node *AllTransitionsWrapper::
extract_from_cache(const NodeRelation *arc) {
  _cache = arc->_net_transitions;
  _all_verified = arc->_all_verified;
#ifndef NDEBUG
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "AllTransitionsWrapper::extract_from_cache(" << *arc
      << "), _all_verified = " << _all_verified << "\n";
  }
#endif
  return arc->_top_subtree;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::store_to_cache
//       Access: Public
//  Description: Completely replaces the cached state on the arc with
//               what is represented here.  This makes sense if we
//               have computed the complete set of net transitions to
//               the arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
store_to_cache(NodeRelation *arc, Node *top_subtree) {
  arc->_top_subtree = top_subtree;
  arc->_net_transitions = _cache;
  arc->_all_verified = _all_verified;

#ifndef NDEBUG
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "AllTransitionsWrapper::store_to_cache(" << *arc
      << "), _all_verified = " << _all_verified << "\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::store_to_cache_partial
//       Access: Public
//  Description: Updates the cached state on the arc with what is
//               represented here, but does not remove transitions on
//               the arc's cache that are not mentioned here.  This
//               makes sense if this cache represents only some of the
//               net transitions to the arc, but not necessarily all
//               of them.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
store_to_cache_partial(NodeRelation *arc, Node *top_subtree) {
  arc->_top_subtree = top_subtree;
  arc->_net_transitions = 
    NodeTransitionCache::c_union(arc->_net_transitions, _cache);

#ifndef NDEBUG
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "AllTransitionsWrapper::store_to_cache_partial(" << *arc
      << "), _all_verified = " << _all_verified << "\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::is_cache_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllTransitionsWrapper::
is_cache_verified(UpdateSeq as_of) const {
#ifndef NDEBUG
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "AllTransitionsWrapper::is_cache_verified(" << as_of 
      << "), _all_verified = " << _all_verified << ", result = "
      << (as_of <= _all_verified) << "\n";
  }
#endif
  return as_of <= _all_verified;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::set_computed_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
set_computed_verified(UpdateSeq now) {
  _cache = NodeTransitionCache::set_computed_verified(_cache, now);
  _all_verified = now;

#ifndef NDEBUG
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "AllTransitionsWrapper::set_computed_verified(" << now << ")\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::cached_compose
//       Access: Public
//  Description: Computes the composition of this wrapper's value with
//               that indicated by value, using the cache as a helper,
//               and stores the result in this wrapper.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllTransitionsWrapper::
cached_compose(const AllTransitionsWrapper &cache, 
	       const AllTransitionsWrapper &value,
	       UpdateSeq now) {
  _cache = NodeTransitionCache::cached_compose(_cache, cache._cache,
					       value._cache, now);
  _all_verified = now;
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::size_type AllTransitionsWrapper::
size() const {
  if (_cache == (NodeTransitionCache *)NULL) {
    return 0;
  }
  return _cache->size();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::const_iterator AllTransitionsWrapper::
begin() const {
  if (_cache == (NodeTransitionCache *)NULL) {
    return _empty_cache.begin();
  }
  return _cache->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllTransitionsWrapper::const_iterator AllTransitionsWrapper::
end() const {
  if (_cache == (NodeTransitionCache *)NULL) {
    // We intentionally return begin() instead of end() here, just in
    // case the "empty" cache accidentally gets something added to it.
    return _empty_cache.begin();
  }
  return _cache->end();
}

INLINE_GRAPH ostream &operator << (ostream &out, const AllTransitionsWrapper &ntw) {
  ntw.output(out);
  return out;
}
