// Filename: odeHeightfieldGeom.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODEHEIGHTFIELDGEOM_H
#define ODEHEIGHTFIELDGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeHeightfieldGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeHeightfieldGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeHeightfieldGeom(dGeomID id);

PUBLISHED:
  OdeHeightfieldGeom();
  virtual ~OdeHeightfieldGeom();

  INLINE dHeightfieldDataID heightfield_data_create();
  INLINE void heightfield_data_destroy(dHeightfieldDataID d);
  INLINE void heightfield_data_build_callback(dHeightfieldDataID d, 
                                              void* p_user_data, 
                                              dHeightfieldGetHeight* p_callback,
                                              dReal width,
                                              dReal depth, 
                                              int width_samples,
                                              int depth_samples,
                                              dReal scale, 
                                              dReal offset,
                                              dReal thickness,
                                              int b_wrap);
  INLINE void heightfield_data_build_byte(dHeightfieldDataID d, 
                                          const unsigned char* p_height_data,
                                          int b_copy_height_data, 
                                          dReal width, 
                                          dReal depth, 
                                          int width_samples,
                                          int depth_samples,
                                          dReal scale, 
                                          dReal offset,
                                          dReal thickness,
                                          int b_wrap);
  INLINE void heightfield_data_build_short(dHeightfieldDataID d,
                                           const short* p_height_data,
                                           int b_copy_height_data,
                                           dReal width,
                                           dReal depth,
                                           int width_samples,
                                           int depth_samples, 
                                           dReal scale, 
                                           dReal offset, 
                                           dReal thickness, 
                                           int b_wrap);
  INLINE void heightfield_data_build_single(dHeightfieldDataID d, 
                                            const float* p_height_data,
                                            int b_copy_height_data, 
                                            dReal width, 
                                            dReal depth, 
                                            int width_samples,
                                            int depth_samples,
                                            dReal scale,
                                            dReal offset, 
                                            dReal thickness, 
                                            int b_wrap);
  INLINE void heightfield_data_build_double(dHeightfieldDataID d,
                                            const double* p_height_data,
                                            int b_copy_height_data, 
                                            dReal width, 
                                            dReal depth,
                                            int width_samples,
                                            int depth_samples,
                                            dReal scale, 
                                            dReal offset, 
                                            dReal thickness,
                                            int b_wrap);
  INLINE void heightfield_data_set_bounds(dHeightfieldDataID d,
                                          dReal min_height, 
                                          dReal max_height);
  INLINE void heightfield_set_heightfield_data(dHeightfieldDataID d);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeHeightfieldGeom",
                  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeHeightfieldGeom.I"

#endif
