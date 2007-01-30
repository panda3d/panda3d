// Filename: odeBoxGeom.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODEBOXGEOM_H
#define ODEBOXGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeBoxGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeBoxGeom : public OdeGeom {
PUBLISHED:
  OdeBoxGeom();
  virtual ~OdeBoxGeom();

  INLINE dHeightfieldDataID heightfield_data_create();
  INLINE void heightfield_data_destroy(dHeightfieldDataID d);
  INLINE void heightfield_data_build_callback(dHeightfieldDataID d, void* p_user_data, dHeightfieldGetHeight* p_callback, dReal width, dReal depth, int width_samples, int depth_samples, dReal scale, dReal offset, dReal thickness, int b_wrap);
  INLINE void heightfield_data_build_byte(dHeightfieldDataID d, const unsigned char* p_height_data, int b_copy_height_data, dReal width, dReal depth, int width_samples, int depth_samples, dReal scale, dReal offset, dReal thickness, int b_wrap);
  INLINE void heightfield_data_build_short(dHeightfieldDataID d, const short* p_height_data, int b_copy_height_data, dReal width, dReal depth, int width_samples, int depth_samples, dReal scale, dReal offset, dReal thickness, int b_wrap);
  INLINE void heightfield_data_build_single(dHeightfieldDataID d, const float* p_height_data, int b_copy_height_data, dReal width, dReal depth, int width_samples, int depth_samples, dReal scale, dReal offset, dReal thickness, int b_wrap);
  INLINE void heightfield_data_build_double(dHeightfieldDataID d, const double* p_height_data, int b_copy_height_data, dReal width, dReal depth, int width_samples, int depth_samples, dReal scale, dReal offset, dReal thickness, int b_wrap);
  INLINE void heightfield_data_set_bounds(dHeightfieldDataID d, dReal min_height, dReal max_height);
  INLINE void heightfield_set_heightfield_data(dHeightfieldDataID d);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeBoxGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeBoxGeom.I"

#endif
