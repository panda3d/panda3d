// Filename: eggTextureCollection.I
// Created by:  drose (16Feb00)
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

INLINE EggTextureCollection::iterator EggTextureCollection::
begin() const {
  nassertr(_ordered_textures.size() == _textures.size(),
           _ordered_textures.begin());
  return _ordered_textures.begin();
}

INLINE EggTextureCollection::iterator EggTextureCollection::
end() const {
  return _ordered_textures.end();
}

INLINE bool EggTextureCollection::
empty() const {
  return _ordered_textures.empty();
}

INLINE EggTextureCollection::size_type EggTextureCollection::
size() const {
  nassertr(_ordered_textures.size() == _textures.size(), 0);
  return _ordered_textures.size();
}
