// Filename: internalName.I
// Created by:  masad (15Jul04)
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
//     Function: InternalName::make
//       Access: Published, Static
//  Description: The public interface for constructing an InternalName
//               pointer.  This will return a new InternalName
//               representing the indicated name, if this is the first
//               time the particular name has been requested; if the
//               name is already in use, it will return the existing
//               pointer.
//
//               If the string contains the '.' character, the string
//               will be divided at the dots and the so-defined
//               hierarchy of names will be registered.  This is
//               handled transparently.
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
make(const string &name) {
  return get_root()->append(name);
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_parent
//       Access: Published
//  Description: Return the parent of this InternalName.  All names
//               have a parent, except the root name.
////////////////////////////////////////////////////////////////////
INLINE InternalName *InternalName::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_basename
//       Access: Published
//  Description: Return the name represented by just this particular
//               InternalName object, ignoring its parents names.
//               This is everything after the rightmost dot.
////////////////////////////////////////////////////////////////////
INLINE const string &InternalName::
get_basename() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_root
//       Access: Published, Static
//  Description: Returns the standard root InternalName.  This is the
//               root of all other InternalNames.  It has no name
//               itself, and it is the only InternalName with no
//               parent.
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_root() {
  if (_root == (InternalName *)NULL) {
    _root = new InternalName(NULL, "");
  }
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_error
//       Access: Published, Static
//  Description: Returns the standard InternalName "error".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_error() {
  if (_error == (InternalName *)NULL) {
    _error = InternalName::make("error");
  }
  return _error;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_vertex
//       Access: Published, Static
//  Description: Returns the standard InternalName "vertex".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_vertex() {
  if (_vertex == (InternalName *)NULL) {
    _vertex = InternalName::make("vertex");
  }
  return _vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_normal
//       Access: Published, Static
//  Description: Returns the standard InternalName "normal".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_normal() {
  if (_normal == (InternalName *)NULL) {
    _normal = InternalName::make("normal");
  }
  return _normal;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_tangent
//       Access: Published, Static
//  Description: Returns the standard InternalName "tangent".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_tangent() {
  if (_tangent == (InternalName *)NULL) {
    _tangent = InternalName::make("tangent");
  }
  return _tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_binormal
//       Access: Published, Static
//  Description: Returns the standard InternalName "binormal".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_binormal() {
  if (_binormal == (InternalName *)NULL) {
    _binormal = InternalName::make("binormal");
  }
  return _binormal;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_texcoord
//       Access: Published, Static
//  Description: Returns the standard InternalName "texcoord".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_texcoord() {
  if (_texcoord == (InternalName *)NULL) {
    _texcoord = InternalName::make("texcoord");
  }
  return _texcoord;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_texcoord_name
//       Access: Published, Static
//  Description: Returns the InternalName "texcoord.name", where name
//               is the supplied string.  This, by convention,
//               represents the named texture coordinate set.
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_texcoord_name(const string &name) {
  return get_texcoord()->append(name);
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_color
//       Access: Published, Static
//  Description: Returns the standard InternalName "color".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_color() {
  if (_color == (InternalName *)NULL) {
    _color = InternalName::make("color");
  }
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_transform_blend
//       Access: Published, Static
//  Description: Returns the standard InternalName "transform_blend".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_transform_blend() {
  if (_transform_blend == (InternalName *)NULL) {
    _transform_blend = InternalName::make("transform_blend");
  }
  return _transform_blend;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_transform_weight
//       Access: Published, Static
//  Description: Returns the standard InternalName "transform_weight".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_transform_weight() {
  if (_transform_weight == (InternalName *)NULL) {
    _transform_weight = InternalName::make("transform_weight");
  }
  return _transform_weight;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_transform_index
//       Access: Published, Static
//  Description: Returns the standard InternalName "transform_index".
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_transform_index() {
  if (_transform_index == (InternalName *)NULL) {
    _transform_index = InternalName::make("transform_index");
  }
  return _transform_index;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_morph
//       Access: Published, Static
//  Description: Returns an InternalName suitable for defining the
//               morph deltas in the GeomVertexData to apply to the
//               indicated data type name, using the named slider.
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) InternalName::
get_morph(InternalName *column, const string &slider) {
  // Returns column.morph.slider
  return column->append("morph")->append(slider);
}

INLINE ostream &
operator << (ostream &out, const InternalName &tcn) {
  tcn.output(out);
  return out;
}
