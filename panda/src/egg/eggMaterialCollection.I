// Filename: eggMaterialCollection.I
// Created by:  drose (30Apr01)
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
