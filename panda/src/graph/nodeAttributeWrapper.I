// Filename: nodeAttributeWrapper.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributeWrapper::
NodeAttributeWrapper(TypeHandle handle) : _handle(handle) {
  nassertv(_handle != TypeHandle::none());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributeWrapper::
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
INLINE_GRAPH void NodeAttributeWrapper::
operator = (const NodeAttributeWrapper &copy) {
  _handle = copy._handle;
  _attrib = copy._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH TypeHandle NodeAttributeWrapper::
get_handle() const {
  return _handle;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::get_attrib
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttribute *NodeAttributeWrapper::
get_attrib() const {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::set_attrib
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeAttributeWrapper::
set_attrib(NodeAttribute *attrib) {
  _attrib = attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::is_initial
//       Access: Public
//  Description: Returns true if the wrapper represents an initial
//               attribute.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttributeWrapper::
is_initial() const {
  return (_attrib == (NodeAttribute *)NULL);
  // || _attrib->is_initial()
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeAttributeWrapper::
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
INLINE_GRAPH void NodeAttributeWrapper::
make_initial() {
  _attrib.clear();
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttributeWrapper &naw) {
  naw.output(out);
  return out;
}
