// Filename: nodeAttributeWrapper.T
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////


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
INLINE_GRAPH bool 
get_attribute_into(Attribute *&ptr, const NodeAttributeWrapper &attrib) {
  NodeAttribute *nt = attrib.get_attrib();
  if (nt == (NodeAttribute *)NULL) {
    ptr = (Attribute *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
