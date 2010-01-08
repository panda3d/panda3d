// Filename: physxHeightFieldDesc.h
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

#ifndef PHYSXHEIGHTFIELDDESC_H
#define PHYSXHEIGHTFIELDDESC_H

#include "pandabase.h"
#include "pnmImage.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxHeightFieldDesc
// Description : Descriptor class for height fields. The height
//               field data is copied when a PhysxHeightField object
//               is created from this descriptor. After the call 
//               the user may discard the original height data (e.g.
//               release the PNGImage).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxHeightFieldDesc {

PUBLISHED:
  INLINE PhysxHeightFieldDesc();
  INLINE ~PhysxHeightFieldDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  INLINE void set_size(unsigned int num_rows, unsigned int num_columns);

  void set_image(const PNMImage &image, unsigned short materialIndex=0);
  void set_thickness(float thickness);
  void set_convex_edge_threshold(float threshold);

  void set_height(unsigned int row, unsigned int column, short height);
  void set_tess_flag(unsigned int row, unsigned int column, unsigned short value);
  void set_material_index(unsigned int row, unsigned int column,
    unsigned short materialIndex0, unsigned short materialIndex1);

public:
  NxHeightFieldDesc _desc;

private:
  NxU32 *_samples;

  INLINE void unset_size();
};

#include "physxHeightFieldDesc.I"

#endif // PHYSXHEIGHTFIELDDESC_H
