// Filename: eggMaterialCollection.I
// Created by:  drose (30Apr01)
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

INLINE EggMaterialCollection::iterator EggMaterialCollection::
begin() const {
  nassertr(_ordered_materials.size() == _materials.size(),
           _ordered_materials.begin());
  return _ordered_materials.begin();
}

INLINE EggMaterialCollection::iterator EggMaterialCollection::
end() const {
  return _ordered_materials.end();
}

INLINE bool EggMaterialCollection::
empty() const {
  return _ordered_materials.empty();
}

INLINE EggMaterialCollection::size_type EggMaterialCollection::
size() const {
  nassertr(_ordered_materials.size() == _materials.size(), 0);
  return _ordered_materials.size();
}
