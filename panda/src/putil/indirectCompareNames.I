// Filename: indirectCompareNames.I
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: IndirectCompareNames::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
INLINE bool IndirectCompareNames<ObjectType>::
operator () (const ObjectType *a, const ObjectType *b) const {
  return (a != b && a->get_name() < b->get_name());
}
