// Filename: nodeAttributes.I
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::size_type NodeAttributes::
size() const {
  return _attributes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::iterator NodeAttributes::
begin() {
  return _attributes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::iterator NodeAttributes::
end() {
  return _attributes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::const_iterator NodeAttributes::
begin() const {
  return _attributes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::const_iterator NodeAttributes::
end() const {
  return _attributes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::insert
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes::iterator NodeAttributes::
insert(NodeAttributes::iterator position, 
       const NodeAttributes::value_type &x) {
  return _attributes.insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::erase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeAttributes::
erase(NodeAttributes::iterator position) {
  _attributes.erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::apply_in_place
//       Access: Public
//  Description: Modifies the current NodeAttributes object to reflect
//               the application of the indicated transitions.
////////////////////////////////////////////////////////////////////
INLINE void NodeAttributes::
apply_in_place(const NodeTransitionCache &trans) {
  apply_from(*this, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::apply
//       Access: Public
//  Description: Allocates and returns a new NodeAttributes object
//               that represents the application of this
//               NodeAttributes to the indicated transition cache.
//               This NodeAttributes object is not changed.
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes *NodeAttributes::
apply(const NodeTransitionCache &trans) const {
  NodeAttributes *na = new NodeAttributes;
  na->apply_from(*this, trans);
  return na;
}

////////////////////////////////////////////////////////////////////
//     Function: get_attribute_into
//  Description: This external template function is handy for
//               extracting a attribute of a particular type from the
//               set.  If the attribute exists, it is automatically
//               downcasted to the correct type and stored in the
//               pointer given in the first parameter, and the return
//               value is true.  If the attribute does not exist, the
//               pointer is filled with NULL and the return value is
//               false.
////////////////////////////////////////////////////////////////////
template<class Attribute>
INLINE bool 
get_attribute_into(Attribute *&ptr, const NodeAttributes &attrib,
		   TypeHandle transition_type) {
  NodeAttribute *nt = attrib.get_attribute(transition_type);
  if (nt == (NodeAttribute *)NULL) {
    ptr = (Attribute *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
