// Filename: eggMorphList.I
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE EggMorphList<MorphType>::
EggMorphList() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE EggMorphList<MorphType>::
EggMorphList(const EggMorphList<MorphType> &copy) :
  _morphs(copy._morphs)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE void EggMorphList<MorphType>::
operator = (const EggMorphList &copy) {
  _morphs = copy._morphs;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE EggMorphList<MorphType>::
~EggMorphList() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE bool EggMorphList<MorphType>::
operator == (const EggMorphList<MorphType> &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE bool EggMorphList<MorphType>::
operator != (const EggMorphList<MorphType> &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE bool EggMorphList<MorphType>::
operator < (const EggMorphList<MorphType> &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
int EggMorphList<MorphType>::
compare_to(const EggMorphList<MorphType> &other) const {
  if (_morphs.size() < other._morphs.size()) {
    return (int)_morphs.size() - (int)other._morphs.size();
  }
  for (size_t i = 0; i < _morphs.size(); i++) {
    int compare = _morphs[i].compare_to(other._morphs[i]);
    if (compare < 0) {
      return compare;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE TYPENAME EggMorphList<MorphType>::iterator EggMorphList<MorphType>::
begin() {
  return _morphs.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE TYPENAME EggMorphList<MorphType>::const_iterator EggMorphList<MorphType>::
begin() const {
  return _morphs.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE TYPENAME EggMorphList<MorphType>::iterator EggMorphList<MorphType>::
end() {
  return _morphs.end();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE TYPENAME EggMorphList<MorphType>::const_iterator EggMorphList<MorphType>::
end() const {
  return _morphs.end();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE TYPENAME EggMorphList<MorphType>::size_type EggMorphList<MorphType>::
size() const {
  return _morphs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE bool EggMorphList<MorphType>::
empty() const {
  return _morphs.empty();
}

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
pair<TYPENAME EggMorphList<MorphType>::iterator, bool> EggMorphList<MorphType>::
insert(const MorphType &value) {
  pair<iterator, bool> result;
  TYPENAME Morphs::iterator mi;
  for (mi = _morphs.begin(); mi != _morphs.end(); ++mi) {
    if ((*mi) == value) {
      // This value is already present.
      result.first = mi;
      result.second = false;
      return result;
    }
  }

  // This value is not already present; add it to the list.
  _morphs.push_back(value);
  result.first = _morphs.begin() + _morphs.size() - 1;
  result.second = true;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::clear
//       Access: Public
//  Description: Empties the list of morphs.
////////////////////////////////////////////////////////////////////
template<class MorphType>
INLINE void EggMorphList<MorphType>::
clear() {
  _morphs.clear();
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

