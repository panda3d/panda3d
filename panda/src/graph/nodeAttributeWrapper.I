// Filename: nodeAttributeWrapper.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributeWrapper::
NodeAttributeWrapper(TypeHandle handle) : _handle(handle) {
  nassertv(_handle != TypeHandle::none());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttributeWrapper::
NodeAttributeWrapper(const NodeAttributeWrapper &copy) :
  _handle(copy._handle),
  _attrib(copy._attrib)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeAttributeWrapper::
operator = (const NodeAttributeWrapper &copy) {
  _handle = copy._handle;
  _attrib = copy._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypeHandle NodeAttributeWrapper::
get_handle() const {
  return _handle;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::get_attrib
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeAttribute *NodeAttributeWrapper::
get_attrib() const {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::set_attrib
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeAttributeWrapper::
set_attrib(NodeAttribute *attrib) {
  _attrib = attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::is_initial
//       Access: Public
//  Description: Returns true if the wrapper represents an initial
//               attribute.
////////////////////////////////////////////////////////////////////
INLINE bool NodeAttributeWrapper::
is_initial() const {
  return (_attrib == (NodeAttribute *)NULL);
  // || _attrib->is_initial()
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int NodeAttributeWrapper::
compare_to(const NodeAttributeWrapper &other) const {
  nassertr(_handle == other._handle, false);
  if (_attrib == other._attrib) {
    return 0;
  }
  if (_attrib == (NodeAttribute *)NULL) {
    return -1;
  }
  if (other._attrib == (NodeAttribute *)NULL) {
    return 1;
  }
  return _attrib->compare_to(*other._attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::make_initial
//       Access: Public
//  Description: Resets the wrapper to the initial attribute.
////////////////////////////////////////////////////////////////////
INLINE void NodeAttributeWrapper::
make_initial() {
  _attrib.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: get_attribute_into
//  Description: This external template function is handy for
//               extracting the attribute (of a known type) from the
//               wrapper.  If the attribute exists, it is
//               automatically downcasted to the correct type and
//               stored in the pointer given in the first parameter,
//               and the return value is true.  If the attribute does
//               not exist, the pointer is filled with NULL and the
//               return value is false.
////////////////////////////////////////////////////////////////////
template<class Attribute>
INLINE bool 
get_attribute_into(Attribute *&ptr, const NodeAttributeWrapper &attrib) {
  NodeAttribute *nt = attrib.get_attrib();
  if (nt == (NodeAttribute *)NULL) {
    ptr = (Attribute *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
