// Filename: nodeTransitionCache.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a pointer to
//               a NodeTransition only.  It infers the type of the
//               NodeTransition from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_transition; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_transition).
////////////////////////////////////////////////////////////////////
INLINE PT(NodeTransition) NodeTransitionCache::
set_transition(NodeTransition *trans) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionCache::size_type NodeTransitionCache::
size() const {
  return _cache.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionCache::const_iterator NodeTransitionCache::
begin() const {
  return _cache.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCache::end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionCache::const_iterator NodeTransitionCache::
end() const {
  return _cache.end();
}
