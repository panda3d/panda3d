// Filename: cullState.I
// Created by:  drose (14Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CullState::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::
CullState(const AllTransitionsWrapper &trans) :
  _trans(trans)
{
  _attrib = (NodeAttributes *)NULL;
  _bin = (GeomBin *)NULL;
  _empty_frames_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::
~CullState() {
  // This shouldn't be destructing while it's still assigned to a bin.
  // If it is, something went screwy with the reference counts or with
  // the bin logic somewhere.
  nassertv(_bin == (GeomBin *)NULL);

  if (_attrib != (NodeAttributes *)NULL) {
    delete _attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int CullState::
compare_to(const CullState &other) const {
  return _trans.compare_to(other._trans);
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::mark_verified
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullState::
mark_verified(Node *node, UpdateSeq now) {
  _verified[node] = now;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::clear_current_nodes
//       Access: Public
//  Description: Called by the CullTraverser at the beginning of the
//               frame to reset the list of GeomNodes currently
//               visible, in preparation for building up a new list
//               (via repeated calls to record_current_node()).
////////////////////////////////////////////////////////////////////
INLINE void CullState::
clear_current_nodes() {
  if (_current_geom_nodes.empty() && _current_direct_nodes.empty()) {
    _empty_frames_count++;
  }
  _current_geom_nodes.clear();
  _current_direct_nodes.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::record_current_geom_node
//       Access: Public
//  Description: Called by the CullTraverser to indicate a particular
//               GeomNode, associated with this state, that is known to
//               be visible this frame.
////////////////////////////////////////////////////////////////////
INLINE void CullState::
record_current_geom_node(const ArcChain &arc_chain) {
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Recording geom node " << arc_chain << " with state " << (void *)this
      << "\n";
  }
  _current_geom_nodes.push_back(arc_chain);
  _empty_frames_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::record_current_direct_node
//       Access: Public
//  Description: Called by the CullTraverser to indicate a particular
//               Node, to be directly rendered, that is known to be
//               visible this frame.
////////////////////////////////////////////////////////////////////
INLINE void CullState::
record_current_direct_node(const ArcChain &arc_chain) {
  _current_direct_nodes.push_back(arc_chain);
  _empty_frames_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::is_empty
//       Access: Public
//  Description: Returns true if the CullState has no current nodes to
//               render (i.e. count_current_nodes() == 0), false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullState::
is_empty() const {
  return _current_geom_nodes.empty() && _current_direct_nodes.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::count_current_nodes
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int CullState::
count_current_nodes() const {
  return _current_geom_nodes.size() + _current_direct_nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::get_empty_frames_count
//       Access: Public
//  Description: Returns the number of consecutive frames the state
//               has been empty, i.e. has represented no visible
//               nodes.
////////////////////////////////////////////////////////////////////
INLINE int CullState::
get_empty_frames_count() const {
  return _empty_frames_count;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::apply_to
//       Access: Public
//  Description: Indicates the initial state this frame; this is
//               applied to the net transitions indicated by the
//               CullState and stored within the CullState object.
////////////////////////////////////////////////////////////////////
INLINE void CullState::
apply_to(const AllAttributesWrapper &initial_state) {
  if (_attrib != (NodeAttributes *)NULL) {
    delete _attrib;
  }
  _attrib = initial_state.apply(_trans);
  nassertv(_attrib != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::get_attributes
//       Access: Public
//  Description: Returns the current state indicated by the CullState
//               object, based on the CullState's net transitions set
//               and the initial state set by apply_to().
////////////////////////////////////////////////////////////////////
INLINE const NodeAttributes &CullState::
get_attributes() const {
#ifndef NDEBUG
  static NodeAttributes empty_attributes;
#endif

  nassertr(_attrib != NULL, empty_attributes);
  return *_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::get_transitions
//       Access: Public
//  Description: Returns the net transitions indicated by the
//               CullState object.
////////////////////////////////////////////////////////////////////
INLINE const AllTransitionsWrapper &CullState::
get_transitions() const {
  return _trans;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::has_bin
//       Access: Public
//  Description: Returns true if the CullState has been assigned to a
//               particular GeomBin.
////////////////////////////////////////////////////////////////////
INLINE bool CullState::
has_bin() const {
  return _bin != (GeomBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::get_bin
//       Access: Public
//  Description: Returns the particular GeomBin the CullState has been
//               assigned to, or NULL if it has not been assigned to a
//               bin.
////////////////////////////////////////////////////////////////////
INLINE GeomBin *CullState::
get_bin() const {
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::geom_size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::geom_size_type CullState::
geom_size() const {
  return _current_geom_nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::geom_begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::geom_iterator CullState::
geom_begin() const {
  return _current_geom_nodes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::geom_end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::geom_iterator CullState::
geom_end() const {
  return _current_geom_nodes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::direct_size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::direct_size_type CullState::
direct_size() const {
  return _current_direct_nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::direct_begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::direct_iterator CullState::
direct_begin() const {
  return _current_direct_nodes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::direct_end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullState::direct_iterator CullState::
direct_end() const {
  return _current_direct_nodes.end();
}
