// Filename: eggMorphList.I
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::insert
//       Access: Public
//  Description: This is similar to the insert() interface for sets,
//               except it does not guarantee that the resulting list
//               is sorted.
//
//               We have this member function so the EggMorphList
//               resembles a set.  It used to *be* a set, but we
//               cannot export STL sets from a Windows DLL.
////////////////////////////////////////////////////////////////////
template<class MorphType>
pair<EggMorphList<MorphType>::iterator, bool> EggMorphList<MorphType>::
insert(const MorphType &value) {
  pair<iterator, bool> result;
  iterator i;
  for (i = begin(); i != end(); ++i) {
    if ((*i) == value) {
      // This value is already present.
      result.first = i;
      result.second = false;
      return result;
    }
  }

  // This value is not already present; add it to the list.
  push_back(value);
  result.first = begin() + size() - 1;
  result.second = true;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MorphType>
void EggMorphList<MorphType>::
write(ostream &out, int indent_level) const {
  const_iterator i;

  for (i = begin(); i != end(); ++i) {
    indent(out, indent_level) << *i << "\n";
  }
}

