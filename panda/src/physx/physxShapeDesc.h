// Filename: physxShapeDesc.h
// Created by:  pratt (Apr 7, 2006)
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

#ifndef PHYSXSHAPEDESC_H
#define PHYSXSHAPEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxShapeDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxShapeDesc {
PUBLISHED:
  ~PhysxShapeDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE float get_density() const;
  unsigned short get_group() const;
  LMatrix4f get_local_pose() const;
  INLINE float get_mass() const;
  unsigned short get_material_index() const;
  INLINE const char * get_name() const;
  INLINE unsigned int get_shape_flags() const;
  INLINE float get_skin_width() const;

  INLINE void set_density( float value );
  void set_group(unsigned short value);
  void set_local_pose( LMatrix4f value );
  INLINE void set_mass( float value );
  void set_material_index(unsigned short value);
  INLINE void set_name( const char * value );
  INLINE void set_shape_flags( unsigned int value );
  INLINE void set_skin_width( float value );

public:
  NxShapeDesc *nShapeDesc;

protected:
  PhysxShapeDesc(NxShapeDesc *subShapeDesc);

private:
  string _name_store;
};

#include "physxShapeDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXSHAPEDESC_H
