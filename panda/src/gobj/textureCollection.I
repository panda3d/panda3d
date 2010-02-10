// Filename: textureCollection.I
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureCollection::
~TextureCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void TextureCollection::
operator += (const TextureCollection &other) {
  add_textures_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::operator +
//       Access: Published
//  Description: Returns a TextureCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE TextureCollection TextureCollection::
operator + (const TextureCollection &other) const {
  TextureCollection a(*this);
  a += other;
  return a;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::append
//       Access: Published
//  Description: Adds a new Texture to the collection.  This method
//               duplicates the add_texture() method; it is provided to
//               satisfy Python's naming convention.
////////////////////////////////////////////////////////////////////
void TextureCollection::
append(Texture *texture) {
  add_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::extend
//       Access: Published
//  Description: Appends the other list onto the end of this one.
//               This method duplicates the += operator; it is
//               provided to satisfy Python's naming convention.
////////////////////////////////////////////////////////////////////
INLINE void TextureCollection::
extend(const TextureCollection &other) {
  operator += (other);
}
