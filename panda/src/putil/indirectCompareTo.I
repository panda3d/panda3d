// Filename: indirectCompareTo.I
// Created by:  drose (04Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: IndirectCompareTo::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
INLINE bool IndirectCompareTo<ObjectType>::
operator () (const ObjectType *a, const ObjectType *b) const {
  return (a != b && a->compare_to(*b) < 0);
}
