// Filename: physxHeightFieldShapeDesc.h
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

#ifndef PHYSXHEIGHTFIELDSHAPEDESC_H
#define PHYSXHEIGHTFIELDSHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

class PhysxHeightField;

////////////////////////////////////////////////////////////////////
//       Class : PhysxHeightFieldShapeDesc
// Description : Descriptor class for PhysxHeightFieldShape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxHeightFieldShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxHeightFieldShapeDesc();
  INLINE ~PhysxHeightFieldShapeDesc();

  void set_to_default();
  INLINE bool is_valid() const;

  void set_height_field(const PhysxHeightField &hf);
  void set_dimensions(const LVector3f &dimensions);
  void set_hole_material(unsigned short index);
  void set_material_index_high_bits(unsigned short index);

  unsigned short get_hole_material() const;
  unsigned short get_material_index_hight_bits() const;

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxHeightFieldShapeDesc _desc;
};

#include "physxHeightFieldShapeDesc.I"

#endif // PHYSXHEIGHTFIELDSHAPEDESC_H
