// Filename: allAttributesWrapper.T
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

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
INLINE_GRAPH bool 
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
