// Filename: builderBucket.I
// Created by:  drose (09Sep97)
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
//     Function: BuilderBucket::set_coords
//       Access: Public
//  Description: Sets the array that will be used to define the
//               vertices for any indexed geometry that is associated
//               with this bucket.
////////////////////////////////////////////////////////////////////
INLINE void BuilderBucket::
set_coords(const PTA_Vertexf &coords) {
  _coords = coords;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::get_coords
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PTA_Vertexf BuilderBucket::
get_coords() const {
  return _coords;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::set_normals
//       Access: Public
//  Description: Sets the array that will be used to define the
//               normals for any indexed geometry that is associated
//               with this bucket.
////////////////////////////////////////////////////////////////////
INLINE void BuilderBucket::
set_normals(const PTA_Normalf &normals) {
  _normals = normals;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::get_normals
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PTA_Normalf BuilderBucket::
get_normals() const {
  return _normals;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::set_texcoords
//       Access: Public
//  Description: Sets the array that will be used to define the
//               texture coordinates for any indexed geometry that is
//               associated with this bucket.
////////////////////////////////////////////////////////////////////
INLINE void BuilderBucket::
set_texcoords(const TexCoordName *name, const PTA_TexCoordf &texcoords) {
  _texcoords[name] = texcoords;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::get_texcoords
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PTA_TexCoordf BuilderBucket::
get_texcoords(const TexCoordName *name) const {
  TexCoords::const_iterator ti = _texcoords.find(name);
  if (ti != _texcoords.end()) {
    return (*ti).second;
  }
  return PTA_TexCoordf();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::set_colors
//       Access: Public
//  Description: Sets the array that will be used to define the
//               colors for any indexed geometry that is associated
//               with this bucket.
////////////////////////////////////////////////////////////////////
INLINE void BuilderBucket::
set_colors(const PTA_Colorf &colors) {
  _colors = colors;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::get_colors
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PTA_Colorf BuilderBucket::
get_colors() const {
  return _colors;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::get_default_bucket
//       Access: Public, Static
//  Description: Returns a pointer to the BuilderBucket that is used
//               to initialize any BuilderBuckets created with a
//               default constructor.  This is just a convenient way
//               to set some global parameters.
////////////////////////////////////////////////////////////////////
INLINE BuilderBucket *BuilderBucket::
get_default_bucket() {
  if (_default_bucket==NULL) {
    _default_bucket = new BuilderBucket(true);
  }
  return _default_bucket;
}
