// Filename: eggMorph.I
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
//     Function: EggMorph::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE EggMorph<Parameter>::
EggMorph(const string &name, const Parameter &offset)
  : Namable(name), _offset(offset) {
}



////////////////////////////////////////////////////////////////////
//     Function: EggMorph::set_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE void EggMorph<Parameter>::
set_offset(const Parameter &offset) {
  _offset = offset;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMorph::get_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE const Parameter &EggMorph<Parameter>::
get_offset() const {
  return _offset;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMorph::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator < (const EggMorph<Parameter> &other) const {
  return get_name() < other.get_name();
}


////////////////////////////////////////////////////////////////////
//     Function: EggMorph::Equality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator == (const EggMorph<Parameter> &other) const {
  return get_name() == other.get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorph::Inequality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator != (const EggMorph<Parameter> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMorph::compare_to
//       Access: Public
//  Description: compare_to() compares a different space than the
//               operator methods, which only check the name.
//               compare_to() compares the name and the value as well.
////////////////////////////////////////////////////////////////////
template<class Parameter>
INLINE int EggMorph<Parameter>::
compare_to(const EggMorph<Parameter> &other) const {
  int compare = strcmp(get_name().c_str(), other.get_name().c_str());
  if (compare != 0) {
    return compare;
  }
  return _offset.compare_to(other._offset);
}


////////////////////////////////////////////////////////////////////
//     Function: EggMorphVertex output operator
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const EggMorphVertex &m) {
  return out << "<Dxyz> " << m.get_name() << " { " << m.get_offset() << " }";
}


/*
////////////////////////////////////////////////////////////////////
//     Function: EggMorphNormal output operator
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const EggMorphNormal &m) {
  return out << "<DNormal> " << m.get_name() << " { " << m.get_offset() << " }";
}
*/


////////////////////////////////////////////////////////////////////
//     Function: EggMorphTexCoord output operator
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const EggMorphTexCoord &m) {
  return out << "<Duv> " << m.get_name() << " { " << m.get_offset() << " }";
}


////////////////////////////////////////////////////////////////////
//     Function: EggMorphColor output operator
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const EggMorphColor &m) {
  return out << "<Drgba> " << m.get_name() << " { " << m.get_offset() << " }";
}
