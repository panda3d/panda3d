// Filename: allAttributesWrapper.I
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AllAttributesWrapper::
AllAttributesWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AllAttributesWrapper::
AllAttributesWrapper(const NodeAttributes &attrib) :
  _attrib(attrib)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AllAttributesWrapper::
AllAttributesWrapper(const AllAttributesWrapper &copy) :
  _attrib(copy._attrib)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void AllAttributesWrapper::
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
INLINE AllAttributesWrapper AllAttributesWrapper::
init_from(const AllTransitionsWrapper &) {
  return AllAttributesWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AllAttributesWrapper::
~AllAttributesWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::is_empty
//       Access: Public
//  Description: Returns true if there are no Attributes stored in
//               the set, or false if there are any (even initial)
//               Attributes.
////////////////////////////////////////////////////////////////////
INLINE bool AllAttributesWrapper::
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
INLINE PT(NodeAttribute) AllAttributesWrapper::
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
INLINE PT(NodeAttribute) AllAttributesWrapper::
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
INLINE PT(NodeAttribute) AllAttributesWrapper::
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
INLINE bool AllAttributesWrapper::
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
INLINE NodeAttribute *AllAttributesWrapper::
get_attribute(TypeHandle handle) const {
  return _attrib.get_attribute(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::get_attributes
//       Access: Public
//  Description: Returns the entire set of attributes associated with
//               the wrapper.
////////////////////////////////////////////////////////////////////
INLINE const NodeAttributes &AllAttributesWrapper::
get_attributes() const {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::get_attributes
//       Access: Public
//  Description: Returns the entire set of attributes associated with
//               the wrapper.
////////////////////////////////////////////////////////////////////
INLINE NodeAttributes &AllAttributesWrapper::
get_attributes() {
  return _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::is_initial
//       Access: Public
//  Description: Returns true if the wrapper represents an initial
//               attribute.
////////////////////////////////////////////////////////////////////
INLINE bool AllAttributesWrapper::
is_initial() const {
  return _attrib.is_initial();
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int AllAttributesWrapper::
compare_to(const AllAttributesWrapper &other) const {
  return _attrib.compare_to(other._attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::make_initial
//       Access: Public
//  Description: Resets the wrapper to the initial attribute.
////////////////////////////////////////////////////////////////////
INLINE void AllAttributesWrapper::
make_initial() {
  _attrib.clear();
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
get_attribute_into(Attribute *&ptr, const AllAttributesWrapper &attrib,
		   TypeHandle transition_type) {
  NodeAttribute *nt = attrib.get_attribute(transition_type);
  if (nt == (NodeAttribute *)NULL) {
    ptr = (Attribute *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
