// Filename: builderVertexTempl.I
// Created by:  drose (11Sep97)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT>::
BuilderVertexTempl() {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::Constructor (with VType)
//       Access: Public
//  Description: Initializes the vertex coordinate with an initial
//               value.  A handy constructor.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT>::
BuilderVertexTempl(const VType &c) {
  set_coord(c);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT>::
BuilderVertexTempl(const BuilderVertexTempl &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
operator = (const BuilderVertexTempl<VT, NT, TT, CT> &copy) {
  BuilderAttribTempl<VT, NT, TT, CT>::operator = (copy);
  _coord = copy._coord;
  _texcoord = copy._texcoord;
  _pixel_size = copy._pixel_size;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::is_valid
//       Access: Public
//  Description: Returns true if the vertex is valid, i.e. if it has a
//               vertex coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
is_valid() const {
  return has_coord(); 
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear
//       Access: Public
//  Description: Resets the vertex to its initial, empty state.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear() {
  BuilderAttribTempl<VT, NT, TT, CT>::clear();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::has_coord
//       Access: Public
//  Description: Returns true if the vertex has a vertex coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
has_coord() const {
  return _flags & BAF_coord;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::get_coord
//       Access: Public
//  Description: Returns the vertex's coordinate.  It is an error to
//               call this without first verifying that has_coord() is
//               true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT>::VType BuilderVertexTempl<VT, NT, TT, CT>::
get_coord() const {
  nassertr(has_coord(), _coord);
  return _coord;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_coord
//       Access: Public
//  Description: Resets the vertex's coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_coord(const VType &c) {
  _flags |= BAF_coord;
  _coord = c;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_normal
//       Access: Public
//  Description: Resets the vertex's normal.  This is overridden from
//               BuilderAttrib just so we can typecast the return
//               value to BuilderVertex.  The other functions,
//               has_normal() and get_normal(), are inherited
//               untouched from BuilderAttrib.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_normal(const NType &n) {
  BuilderAttribTempl<VT, NT, TT, CT>::set_normal(n);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear_normal
//       Access: Public
//  Description: Removes the vertex's normal.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear_normal() {
  BuilderAttribTempl<VT, NT, TT, CT>::clear_normal();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::has_texcoord
//       Access: Public
//  Description: Returns true if the vertex has a texture coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
has_texcoord() const {
  return (_flags & BAF_texcoord) != 0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_texcoord
//       Access: Public
//  Description: Resets the vertex's texture coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_texcoord(const TType &t) {
  _flags |= BAF_texcoord;
  _texcoord = t;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear_texcoord
//       Access: Public
//  Description: Removes the vertex's texcoord.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear_texcoord() {
  _flags &= ~BAF_texcoord;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::get_texcoord
//       Access: Public
//  Description: Returns the vertex's texture coordinate.  It is an
//               error to call this without first verifying that
//               has_texcoord() is true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT>::TType BuilderVertexTempl<VT, NT, TT, CT>::
get_texcoord() const {
  nassertr(has_texcoord(), _texcoord);
  return _texcoord;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_color
//       Access: Public
//  Description: Resets the vertex's color.  This is overridden from
//               BuilderAttrib just so we can typecast the return
//               value to BuilderVertex.  The other functions,
//               has_color() and get_color(), are inherited
//               untouched from BuilderAttrib.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_color(const CType &c) {
  BuilderAttribTempl<VT, NT, TT, CT>::set_color(c);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear_color
//       Access: Public
//  Description: Removes the vertex's color.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear_color() {
  BuilderAttribTempl<VT, NT, TT, CT>::clear_color();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_pixel_size
//       Access: Public
//  Description: Resets the vertex's pixel_size.  This is overridden
//               from BuilderAttrib just so we can typecast the return
//               value to BuilderVertex.  The other functions,
//               has_pixel_size() and get_pixel_size(), are inherited
//               untouched from BuilderAttrib.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_pixel_size(float s) {
  BuilderAttribTempl<VT, NT, TT, CT>::set_pixel_size(s);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear_pixel_size
//       Access: Public
//  Description: Removes the vertex's pixel_size.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear_pixel_size() {
  BuilderAttribTempl<VT, NT, TT, CT>::clear_pixel_size();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::operator ==
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices in the locus of consideration will
//               share the same state: with or without normals,
//               texcoords, etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
bool BuilderVertexTempl<VT, NT, TT, CT>::
operator == (const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  if (has_coord() && !(_coord == other._coord))
    return false;

  if (has_texcoord() && !(_texcoord == other._texcoord))
    return false;

  return BuilderAttribTempl<VT, NT, TT, CT>::operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::operator !=
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices in the locus of consideration will
//               share the same state: with or without normals,
//               texcoords, etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
operator != (const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::operator <
//       Access: Public
//  Description: Assigns an ordering to the vertices.  This is used by
//               the Mesher to group identical vertices.  This assumes
//               that all vertices to be meshed together must share
//               the same state: with or without normals, texcoords,
//               etc.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
bool BuilderVertexTempl<VT, NT, TT, CT>::
operator < (const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  if (has_coord() && !(_coord == other._coord))
    return _coord < other._coord;

  if (has_texcoord() && !(_texcoord == other._texcoord))
    return _texcoord < other._texcoord;

  return BuilderAttribTempl<VT, NT, TT, CT>::operator < (other);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::output
//       Access: Public
//  Description: Formats the vertex for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
ostream &BuilderVertexTempl<VT, NT, TT, CT>::
output(ostream &out) const {
  if (this!=NULL) {
    if (has_coord()) {
      out << get_coord();
    }

    /*
    if (has_normal()) {
      out << " normal " << get_normal();
    }
    
    if (has_texcoord()) {
      out << " texcoord " << get_texcoord();
    }
    
    if (has_color()) {
      out << " color " << get_color();
    }
    
    if (has_pixel_size()) {
      out << " pixel_size " << get_pixel_size();
    }
    */
  }
  return out;
}
