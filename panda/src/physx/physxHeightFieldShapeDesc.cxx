/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxHeightFieldShapeDesc.cxx
 * @author enn0x
 * @date 2009-10-15
 */

#include "physxHeightFieldShapeDesc.h"
#include "physxHeightField.h"

/**
 * (re)sets the structure to the default.
 */
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

/**
 * Sets the the material index that designates holes in the height field.
 * This number is compared directly to sample materials.  Consequently the
 * high 9 bits must be zero.  Default value is 0.
 */
void PhysxHeightFieldShapeDesc::
set_hole_material(unsigned short index) {

  _desc.holeMaterial = (NxMaterialIndex)index;
}

/**
 * Sets the high 9 bits of this number are used to complete the material
 * indices in the samples.  The remaining low 7 bits must be zero.  Default
 * value is 0.
 */
void PhysxHeightFieldShapeDesc::
set_material_index_high_bits(unsigned short index) {

  _desc.materialIndexHighBits = (NxMaterialIndex)index;
}

/**
 * Returns the the material index that designates holes in the height field.
 */
unsigned short PhysxHeightFieldShapeDesc::
get_hole_material() const {

  return (unsigned short)_desc.holeMaterial;
}

/**
 * Returns the high 9 bits of this number are used to complete the material
 * indices in the samples.
 */
unsigned short PhysxHeightFieldShapeDesc::
get_material_index_hight_bits() const {

  return (unsigned short)_desc.materialIndexHighBits;
}

/**
 *
 */
void PhysxHeightFieldShapeDesc::
set_dimensions(const LVector3f &dimensions) {

  NxU32 _64K = 65535; // (1<<16)-1;

  NxU32 nbRows    = _desc.heightField->getNbRows();
  NxU32 nbColumns = _desc.heightField->getNbColumns();

  _desc.rowScale    = dimensions.get_x() / nbRows;
  _desc.columnScale = dimensions.get_y() / nbColumns;
  _desc.heightScale = dimensions.get_z() / _64K;
}

/**
 *
 */
void PhysxHeightFieldShapeDesc::
set_height_field(const PhysxHeightField &hf) {

   _desc.heightField = hf.ptr();
}
