// Filename: cLwoSurfaceBlockTMap.cxx
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

#include "cLwoSurfaceBlockTMap.h"
#include "lwoToEggConverter.h"

#include "lwoSurfaceBlockTransform.h"
#include "lwoSurfaceBlockRefObj.h"
#include "compose_matrix.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurfaceBlockTMap::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLwoSurfaceBlockTMap::
CLwoSurfaceBlockTMap(LwoToEggConverter *converter, const LwoSurfaceBlockTMap *tmap) :
  _converter(converter),
  _tmap(tmap)
{
  _center.set(0.0, 0.0, 0.0);
  _size.set(1.0, 1.0, 1.0);
  _rotation.set(0.0, 0.0, 0.0);
  _csys = LwoSurfaceBlockCoordSys::T_object;
  _reference_object = "(none)";

  // Scan the chunks in the body.
  int num_chunks = _tmap->get_num_chunks();
  for (int i = 0; i < num_chunks; i++) {
    const IffChunk *chunk = _tmap->get_chunk(i);

    if (chunk->is_of_type(LwoSurfaceBlockTransform::get_class_type())) {
      const LwoSurfaceBlockTransform *trans = DCAST(LwoSurfaceBlockTransform, chunk);
      if (trans->get_id() == IffId("CNTR")) {
        _center = trans->_vec;
      } else if (trans->get_id() == IffId("SIZE")) {
        _size = trans->_vec;
      } else if (trans->get_id() == IffId("ROTA")) {
        _rotation = trans->_vec;
      }

    } else if (chunk->is_of_type(LwoSurfaceBlockRefObj::get_class_type())) {
      const LwoSurfaceBlockRefObj *ref = DCAST(LwoSurfaceBlockRefObj, chunk);
      _reference_object = ref->_name;

    } else if (chunk->is_of_type(LwoSurfaceBlockCoordSys::get_class_type())) {
      const LwoSurfaceBlockCoordSys *csys = DCAST(LwoSurfaceBlockCoordSys, chunk);
      _csys = csys->_type;
   }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurfaceBlockTMap::get_transform
//       Access: Public
//  Description: Fills up the indicated matrix with the net transform
//               indicated by the TMAP chunk, accounting for scale,
//               rotate, and translate.
////////////////////////////////////////////////////////////////////
void CLwoSurfaceBlockTMap::
get_transform(LMatrix4d &mat) const {
  LPoint3d hpr(rad_2_deg(_rotation[0]),
               rad_2_deg(-_rotation[1]),
               rad_2_deg(-_rotation[2]));
  compose_matrix(mat, LCAST(double, _size), hpr,
                 LCAST(double, _center), CS_yup_left);
}
