// Filename: builderVertexTempl.I
// Created by:  drose (11Sep97)
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
  this->_coord = copy._coord;
  this->_texcoords = copy._texcoords;
  this->_pixel_size = copy._pixel_size;

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
  return (this->_flags) & BAF_coord;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::get_coord
//       Access: Public
//  Description: Returns the vertex's coordinate.  It is an error to
//               call this without first verifying that has_coord() is
//               true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE TYPENAME BuilderVertexTempl<VT, NT, TT, CT>::VType BuilderVertexTempl<VT, NT, TT, CT>::
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
  this->_flags |= BAF_coord;
  this->_coord = c;
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
has_texcoord(const InternalName *name) const {
  TYPENAME TexCoords::const_iterator ti = _texcoords.find(name);
  return (ti != _texcoords.end());
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::get_texcoord
//       Access: Public
//  Description: Returns the vertex's texture coordinate.  It is an
//               error to call this without first verifying that
//               has_texcoord() is true.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE TYPENAME BuilderVertexTempl<VT, NT, TT, CT>::TType BuilderVertexTempl<VT, NT, TT, CT>::
get_texcoord(const InternalName *name) const {
  TYPENAME TexCoords::const_iterator ti = _texcoords.find(name);
  nassertr(ti != _texcoords.end(), (*ti).second);
  return (*ti).second;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::set_texcoord
//       Access: Public
//  Description: Resets the vertex's texture coordinate.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
set_texcoord(const InternalName *name, const TType &t) {
  _texcoords[name] = t;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::clear_texcoord
//       Access: Public
//  Description: Removes the vertex's texcoord.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE BuilderVertexTempl<VT, NT, TT, CT> &BuilderVertexTempl<VT, NT, TT, CT>::
clear_texcoord(const InternalName *name) {
  _texcoords.erase(name);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::tc_begin
//       Access: Public
//  Description: Returns an iterator that can be used to walk through
//               the set of TexCoord names and their definitions.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE TYPENAME BuilderVertexTempl<VT, NT, TT, CT>::tc_const_iterator BuilderVertexTempl<VT, NT, TT, CT>::
tc_begin() const {
  return _texcoords.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::tc_end
//       Access: Public
//  Description: Returns an iterator that can be used to walk through
//               the set of TexCoord names and their definitions.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE TYPENAME BuilderVertexTempl<VT, NT, TT, CT>::tc_const_iterator BuilderVertexTempl<VT, NT, TT, CT>::
tc_end() const {
  return _texcoords.end();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::tc_size
//       Access: Public
//  Description: Returns the number of TexCoord definitions.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
INLINE TYPENAME BuilderVertexTempl<VT, NT, TT, CT>::tc_size_type BuilderVertexTempl<VT, NT, TT, CT>::
tc_size() const {
  return _texcoords.size();
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
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
operator == (const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  return compare_to(other) == 0;
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
INLINE bool BuilderVertexTempl<VT, NT, TT, CT>::
operator < (const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertexTempl::compare_to
//       Access: Public
//  Description: Returns a number less than zero if this vertex sorts
//               before the indicated vertex, greater than zero if it
//               sorts after, and zero if the vertices are equivalent.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
int BuilderVertexTempl<VT, NT, TT, CT>::
compare_to(const BuilderVertexTempl<VT, NT, TT, CT> &other) const {
  if (has_coord()) {
    int coord_compare = builder_compare(_coord, other._coord);
    if (coord_compare != 0) {
      return coord_compare;
    }
  }

  TYPENAME TexCoords::const_iterator ai, bi;
  ai = _texcoords.begin();
  bi = other._texcoords.begin();
  while (ai != _texcoords.end() && bi != other._texcoords.end()) {
    if ((*ai).first < (*bi).first) {
      return -1;
    } else if ((*bi).first < (*ai).first) {
      return 1;
    } else {
      int texcoord_compare = builder_compare((*ai).second, (*bi).second);
      if (texcoord_compare != 0) {
        return texcoord_compare;
      }
    }
    ++ai;
    ++bi;
  }
  if (bi != other._texcoords.end()) {
    return -1;
  }
  if (ai != _texcoords.end()) {
    return 1;
  }

  return BuilderAttribTempl<VT, NT, TT, CT>::compare_to(other);
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
    if (this->has_coord()) {
      out << this->get_coord();
    }

    if (this->has_normal()) {
      out << " normal " << this->get_normal();
    }

    if (this->has_color()) {
      out << " color " << this->get_color();
    }

    TYPENAME TexCoords::const_iterator ti;
    for (ti = _texcoords.begin(); ti != _texcoords.end(); ++ti) {
      const InternalName *name = (*ti).first;
      out << " texcoord \"" << name->get_name() << "\" " << (*ti).second;
    }

    if (this->has_pixel_size()) {
      out << " pixel_size " << this->get_pixel_size();
    }
  }
  return out;
}
