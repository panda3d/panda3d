// Filename: texGenAttrib.I
// Created by:  masad (21Jun04)
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
//     Function: TexGenAttrib::Constructor
//       Access: Protected
//  Description: Use TexGenAttrib::make() to construct a new
//               TexGenAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::
TexGenAttrib() :
  _num_point_sprites(0),
  _point_geom_rendering(0),
  _geom_rendering(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::Copy Constructor
//       Access: Protected
//  Description: Use TexGenAttrib::make() to construct a new
//               TexGenAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::
TexGenAttrib(const TexGenAttrib &copy) :
  _stages(copy._stages),
  _no_texcoords(copy._no_texcoords),
  _num_point_sprites(copy._num_point_sprites),
  _point_geom_rendering(copy._point_geom_rendering),
  _geom_rendering(copy._geom_rendering)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_geom_rendering
//       Access: Published
//  Description: Returns the union of the Geom::GeomRendering bits
//               that will be required once this TexGenAttrib is
//               applied to a geom which includes the indicated
//               geom_rendering bits.
////////////////////////////////////////////////////////////////////
INLINE int TexGenAttrib::
get_geom_rendering(int geom_rendering) const {
  if ((geom_rendering & Geom::GR_point) != 0) {
    geom_rendering |= _point_geom_rendering;
  }

  return geom_rendering | _geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::ModeDef::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::ModeDef::
ModeDef() :
  _mode(M_off)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::ModeDef::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int TexGenAttrib::ModeDef::
compare_to(const TexGenAttrib::ModeDef &other) const {
  if (_mode != other._mode) {
    return (int)_mode < (int)other._mode ? -1 : 1;
  }
  int compare = _light.compare_to(other._light);
  if (compare != 0) {
    return compare;
  }
  compare = strcmp(_source_name.c_str(), other._source_name.c_str());
  if (compare != 0) {
    return compare;
  }
  compare = _constant_value.compare_to(other._constant_value);
  return compare;
}

