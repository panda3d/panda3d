// Filename: allAttributesWrapper.I
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllAttributesWrapper::
AllAttributesWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllAttributesWrapper::
AllAttributesWrapper(const NodeAttributes &attrib) :
  _attrib(attrib)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllAttributesWrapper::
AllAttributesWrapper(const AllAttributesWrapper &copy) :
  _attrib(copy._attrib)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllAttributesWrapper::
operator = (const AllAttributesWrapper &copy) {
  _attrib = copy._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               AllAttributesWrapper ready to access the same type
//               of AllAttributes as the other.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllAttributesWrapper AllAttributesWrapper::
init_from(const AllTransitionsWrapper &) {
  return AllAttributesWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH AllAttributesWrapper::
~AllAttributesWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::is_empty
//       Access: Public
//  Description: Returns true if there are no Attributes stored in
//               the set, or false if there are any (even initial)
//               Attributes.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllAttributesWrapper::
is_empty() const {
  return _attrib.is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::set_attribute
//       Access: Public
//  Description: This flavor of set_attribute() accepts a specific
//               TypeHandle, indicating the type of attribute that we
//               are setting, and a NodeAttribute pointer indicating
//               the value of the attribute.  The NodeAttribute may
//               be NULL indicating that the attribute should be
//               cleared.  If the NodeAttribute is not NULL, it must
//               match the type indicated by the TypeHandle.
//
//               The return value is a pointer to the *previous*
//               attribute in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeAttribute) AllAttributesWrapper::
set_attribute(TypeHandle handle, NodeAttribute *trans) {
  return _attrib.set_attribute(handle, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::set_attribute
//       Access: Public
//  Description: This flavor of set_attribute() accepts a pointer to
//               a NodeAttribute only.  It infers the type of the
//               NodeAttribute from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_attribute; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_attribute).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeAttribute) AllAttributesWrapper::
set_attribute(NodeAttribute *trans) {
  nassertr(trans != (NodeAttribute *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_attribute(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::clear_attribute
//       Access: Public
//  Description: Removes any attribute associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               attribute in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeAttribute) AllAttributesWrapper::
clear_attribute(TypeHandle handle) {
  return _attrib.clear_attribute(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::has_attribute
//       Access: Public
//  Description: Returns true if ab attribute associated with the
//               indicated handle has been stored in the set (even if
//               it is the initial attribute), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllAttributesWrapper::
has_attribute(TypeHandle handle) const {
  return _attrib.has_attribute(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::get_attribute
//       Access: Public
//  Description: Returns the attribute associated with the indicated
//               handle, or NULL if no such attribute has been stored
//               in the set.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttribute *AllAttributesWrapper::
get_attribute(TypeHandle handle) const {
  return _attrib.get_attribute(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::get_attributes
//       Access: Public
//  Description: Returns the entire set of attributes associated with
//               the wrapper.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const NodeAttributes &AllAttributesWrapper::
get_attributes() const {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::get_attributes
//       Access: Public
//  Description: Returns the entire set of attributes associated with
//               the wrapper.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes &AllAttributesWrapper::
get_attributes() {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::is_initial
//       Access: Public
//  Description: Returns true if the wrapper represents an initial
//               attribute.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool AllAttributesWrapper::
is_initial() const {
  return _attrib.is_initial();
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int AllAttributesWrapper::
compare_to(const AllAttributesWrapper &other) const {
  return _attrib.compare_to(other._attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::make_initial
//       Access: Public
//  Description: Resets the wrapper to the initial attribute.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void AllAttributesWrapper::
make_initial() {
  _attrib.clear();
}

INLINE_GRAPH ostream &operator << (ostream &out, const AllAttributesWrapper &a) {
  a.output(out);
  return out;
}
