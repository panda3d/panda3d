// Filename: builderAttribTempl.I
// Created by:  drose (17Sep97)
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
INLINE TYPENAME BuilderAttribTempl<VT, NT, TT, CT>::NType BuilderAttribTempl<VT, NT, TT, CT>::
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
//     Function: BuilderAttribTempl::clear_normal
//       Access: Public
//  Description: Removes the attribute's normal.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
clear_normal() {
  _flags &= ~BAF_normal;
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
INLINE TYPENAME BuilderAttribTempl<VT, NT, TT, CT>::CType BuilderAttribTempl<VT, NT, TT, CT>::
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
//     Function: BuilderAttribTempl::clear_color
//       Access: Public
//  Description: Removes the attribute's color.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
clear_color() {
  _flags &= ~BAF_color;
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
//     Function: BuilderAttribTempl::clear_pixel_size
//       Access: Public
//  Description: Removes the attribute's pixel_size.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderAttribTempl<VT, NT, TT, CT> &BuilderAttribTempl<VT, NT, TT, CT>::
clear_pixel_size() {
  _flags &= ~BAF_pixel_size;
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
  return compare_to(other) == 0;
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
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::compare_to
//       Access: Public
//  Description: Returns a number less than zero if this object sorts
//               before the indicated object, greater than zero if it
//               sorts after, and zero if the objects are equivalent.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
int BuilderAttribTempl<VT, NT, TT, CT>::
compare_to(const BuilderAttribTempl<VT, NT, TT, CT> &other) const {
  if (has_normal()) {
    int normal_compare = builder_compare(_normal, other._normal);
    if (normal_compare != 0) {
      return normal_compare;
    }
  }

  if (has_color()) {
    int color_compare = builder_compare(_color, other._color);
    if (color_compare != 0) {
      return color_compare;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribTempl::output
//       Access: Public
//  Description: Formats the attribs for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
ostream &BuilderAttribTempl<VT, NT, TT, CT>::
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
