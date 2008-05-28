// Filename: texMatrixAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: TexMatrixAttrib::Constructor
//       Access: Protected
//  Description: Use TexMatrixAttrib::make() to construct a new
//               TexMatrixAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexMatrixAttrib::
TexMatrixAttrib() : _stage_list_stale(true) {
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::Copy Constructor
//       Access: Protected
//  Description: Use TexMatrixAttrib::make() to construct a new
//               TexMatrixAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexMatrixAttrib::
TexMatrixAttrib(const TexMatrixAttrib &copy) :
  _stages(copy._stages),
  _stage_list_stale(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_geom_rendering
//       Access: Published
//  Description: Returns the union of the Geom::GeomRendering bits
//               that will be required once this TexMatrixAttrib is
//               applied to a geom which includes the indicated
//               geom_rendering bits.
////////////////////////////////////////////////////////////////////
INLINE int TexMatrixAttrib::
get_geom_rendering(int geom_rendering) const {
  if ((geom_rendering & Geom::GR_point_sprite) != 0) {
    if (!is_empty()) {
      geom_rendering |= Geom::GR_point_sprite_tex_matrix;
    }
  }

  return geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::check_stage_list
//       Access: Private
//  Description: Builds the linear list of TextureStages if it needs
//               to be built.
////////////////////////////////////////////////////////////////////
INLINE void TexMatrixAttrib::
check_stage_list() const {
  if (_stage_list_stale) {
    ((TexMatrixAttrib *)this)->rebuild_stage_list();
  }
}
