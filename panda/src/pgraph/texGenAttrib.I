// Filename: texGenAttrib.I
// Created by:  masad (21Jun04)
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
//     Function: TexGenAttrib::Constructor
//       Access: Protected
//  Description: Use TexGenAttrib::make() to construct a new
//               TexGenAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::
TexGenAttrib() :
  _num_point_sprites(0)
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
  _num_point_sprites(copy._num_point_sprites)
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
  if ((geom_rendering & qpGeom::GR_point) != 0) {
    if (_num_point_sprites > 0) {
      return geom_rendering |= qpGeom::GR_point_sprite;
    }
  }

  return geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_no_texcoords
//       Access: Public
//  Description: Returns the set of TextureStages that have texture
//               coordinates computed for them, and hence do not need
//               to have texture coordinates sent from the Geom.  This
//               is, of course, the set of TextureStages that is
//               listed in the attrib (except for those listed with M_off).
////////////////////////////////////////////////////////////////////
INLINE const Geom::NoTexCoordStages &TexGenAttrib::
get_no_texcoords() const {
  return _no_texcoords;
}
