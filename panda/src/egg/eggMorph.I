// Filename: eggMorph.I
// Created by:  drose (29Jan99)
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
