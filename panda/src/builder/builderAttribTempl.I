// Filename: builderAttribTempl.I
// Created by:  drose (17Sep97)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT>::
BuilderAttribTempl() {
  _flags = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT>::
BuilderAttribTempl(const BuilderAttribTempl &copy) {
  (*this) = copy;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
operator = (const BuilderAttribTempl<VT, NT, TT, CT> &copy) {
  _normal = copy._normal;
  _color = copy._color;
  _pixel_size = copy._pixel_size;
  _flags = copy._flags;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::clear
//       Access: Public
//  Description: Resets the attribute flags to their original, empty
//               state--where no attributes have been applied.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
clear() {
  _flags = 0;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::has_normal
//       Access: Public
//  Description: Returns true if the attribute has a normal.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
has_normal() const {
  return (_flags & BAF_normal)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::get_normal
//       Access: Public
//  Description: Returns the attribute's normal.  It is an error to
//               call this without first verifying that has_normal() is
//               true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT>::NType BuilderAttribTempl<VT, NT, TT, CT>::
get_normal() const {
  nassertr(has_normal(), _normal);
  return _normal;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::set_normal
//       Access: Public
//  Description: Resets the attribute's normal.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
set_normal(const NType &n) {
  _flags |= BAF_normal;
  _normal = n;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::has_color
//       Access: Public
//  Description: Returns true if the attribute has a color.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
has_color() const {
  return (_flags & BAF_color)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::get_color
//       Access: Public
//  Description: Returns the attribute's color.  It is an error to
//               call this without first verifying that has_color() is
//               true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT>::CType BuilderAttribTempl<VT, NT, TT, CT>::
get_color() const {
  nassertr(has_color(), _color);
  return _color;
}

template <class VT, class NT, class TT, class CT>

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::set_color
//       Access: Public
//  Description: Resets the attribute's color.
////////////////////////////////////////////////////////////////////
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
set_color(const CType &c) {
  _flags |= BAF_color;
  _color = c;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::has_pixel_size
//       Access: Public
//  Description: Returns true if the attribute has a pixel_size.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
has_pixel_size() const {
  return (_flags & BAF_pixel_size)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::get_pixel_size
//       Access: Public
//  Description: Returns the attribute's pixel_size.  It is an error to
//               call this without first verifying that has_pixel_size() is
//               true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE float BuilderAttribTempl<VT, NT, TT, CT>::
get_pixel_size() const {
  nassertr(has_pixel_size(), _pixel_size);
  return _pixel_size;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::set_pixel_size
//       Access: Public
//  Description: Resets the attribute's pixel_size.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
set_pixel_size(float s) {
  _flags |= BAF_pixel_size;
  _pixel_size = s;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::operator ==
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices in the locus of consideration will
//               share the same state: with or without normals,
//               texcoords, etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
operator == (const BuilderAttribTempl<VT, NT, TT, CT> &other) const {
  if (has_normal() && !(_normal == other._normal))
    return false;

  if (has_color() && !(_color == other._color))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::operator !=
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices in the locus of consideration will
//               share the same state: with or without normals,
//               texcoords, etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
operator != (const BuilderAttribTempl<VT, NT, TT, CT> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::operator <
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices in the locus of consideration will
//               share the same state: with or without normals,
//               texcoords, etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderAttribTempl<VT, NT, TT, CT>::
operator < (const BuilderAttribTempl<VT, NT, TT, CT> &other) const {
  if (has_normal() && !(_normal == other._normal))
    return _normal < other._normal;

  if (has_color() && !(_color == other._color))
    return _color < other._color;

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::output
//       Access: Public
//  Description: Formats the attribs for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE ostream &BuilderAttribTempl<VT, NT, TT, CT>::
output(ostream &out) const {
  if (this!=NULL) {
    if (has_normal()) {
      out << " normal " << get_normal();
    }
    
    if (has_color()) {
      out << " color " << get_color();
    }
  }
  return out;
}
