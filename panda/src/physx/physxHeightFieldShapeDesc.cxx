// Filename: physxHeightFieldShapeDesc.cxx
// Created by:  enn0x (15Oct09)
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

#include "physxHeightFieldShapeDesc.h"
#include "physxHeightField.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");

  _desc.shapeFlags = NX_SF_FEATURE_INDICES | NX_SF_VISUALIZATION;
  _desc.meshFlags = NX_MESH_SMOOTH_SPHERE_COLLISIONS;
  _desc.materialIndexHighBits = (NxMaterialIndex)0;
  _desc.holeMaterial = (NxMaterialIndex)0;
  _desc.localPose = PhysxManager::mat4_to_nxMat34(LMatrix4f::y_to_z_up_mat());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::set_hole_material
//       Access: Published
//  Description: Sets the the material index that designates holes
//               in the height field. This number is compared
//               directly to sample materials. Consequently the
//               high 9 bits must be zero.
//               Default value is 0.
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldShapeDesc::
set_hole_material(unsigned short index) {

  _desc.holeMaterial = (NxMaterialIndex)index;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::set_material_index_high_bits
//       Access: Published
//  Description: Sets the high 9 bits of this number are used to
//               complete the material indices in the samples. The
//               remaining low 7 bits must be zero.
//               Default value is 0.
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldShapeDesc::
set_material_index_high_bits(unsigned short index) {

  _desc.materialIndexHighBits = (NxMaterialIndex)index;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::get_hole_material
//       Access: Published
//  Description: Returns the the material index that designates
//               holes in the height field.
////////////////////////////////////////////////////////////////////
unsigned short PhysxHeightFieldShapeDesc::
get_hole_material() const {

  return (unsigned short)_desc.holeMaterial;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::get_material_index_hight_bits
//       Access: Published
//  Description: Returns the high 9 bits of this number are used to
//               complete the material indices in the samples.
////////////////////////////////////////////////////////////////////
unsigned short PhysxHeightFieldShapeDesc::
get_material_index_hight_bits() const {

  return (unsigned short)_desc.materialIndexHighBits;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::set_dimensions
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldShapeDesc::
set_dimensions(const LVector3f &dimensions) {

  NxU32 _64K = 65535; // (1<<16)-1;

  NxU32 nbRows    = _desc.heightField->getNbRows();
  NxU32 nbColumns = _desc.heightField->getNbColumns();

  _desc.rowScale    = dimensions.get_x() / nbRows;
  _desc.columnScale = dimensions.get_y() / nbColumns;
  _desc.heightScale = dimensions.get_z() / _64K;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldShapeDesc::set_height_field
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldShapeDesc::
set_height_field(const PhysxHeightField &hf) {

   _desc.heightField = hf.ptr();
}

