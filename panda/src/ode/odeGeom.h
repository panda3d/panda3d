/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEGEOM_H
#define ODEGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "bitMask.h"

#include "ode_includes.h"
#include "odeSpace.h"
#include "odeBody.h"

class OdeBoxGeom;
class OdeCappedCylinderGeom;
// class OdeConvexGeom;
class OdeCylinderGeom;
// class OdeHeightfieldGeom;
class OdePlaneGeom;
class OdeRayGeom;
class OdeSphereGeom;
class OdeTriMeshGeom;
class OdeSimpleSpace;
class OdeHashSpace;
class OdeQuadTreeSpace;

class OdeUtil;
class OdeCollisionEntry;

/**
 *
 */
class EXPCL_PANDAODE OdeGeom : public TypedObject {
  friend class OdeContactGeom;
  friend class OdeSpace;
  friend class OdeUtil;
  friend class OdeCollisionEntry;

public:
  OdeGeom(dGeomID id);

PUBLISHED:
  enum GeomClass { GC_sphere = 0,
                   GC_box,
                   GC_capped_cylinder,
                   GC_cylinder,
                   GC_plane,
                   GC_ray,
                   // GC_convex, GC_geom_transform,
                   GC_tri_mesh = 8,
                   // GC_heightfield,

                   GC_simple_space = 10,
                   GC_hash_space,
                   GC_quad_tree_space,
  };

  virtual ~OdeGeom();
  void destroy();
  INLINE bool is_empty() const;
  INLINE dGeomID get_id() const;

  // INLINE void set_data(void* data);
  INLINE void set_body(OdeBody &body);
  INLINE bool has_body() const;
  INLINE OdeBody get_body() const;
  INLINE void set_position(dReal x, dReal y, dReal z);
  INLINE void set_position(const LVecBase3f &pos);
  INLINE void set_rotation(const LMatrix3f &r);
  INLINE void set_quaternion(const LQuaternionf &q);
  INLINE LPoint3f get_position() const;
  INLINE LMatrix3f get_rotation() const;
  INLINE LQuaternionf get_quaternion() const;
  INLINE void get_AABB(LVecBase3f &min, LVecBase3f &max) const;
  EXTENSION(INLINE PyObject *get_AA_bounds() const);
  INLINE int is_space();
  INLINE int get_class() const;
  INLINE void set_category_bits(const BitMask32 &bits);
  INLINE void set_collide_bits(const BitMask32 &bits);
  INLINE BitMask32 get_category_bits();
  INLINE BitMask32 get_collide_bits();
  INLINE void enable();
  INLINE void disable();
  INLINE int is_enabled();
  INLINE void set_offset_position(dReal x, dReal y, dReal z);
  INLINE void set_offset_position(const LVecBase3f &pos);
  INLINE void set_offset_rotation(const LMatrix3f &r);
  INLINE void set_offset_quaternion(const LQuaternionf &q);
  INLINE void set_offset_world_position(dReal x, dReal y, dReal z);
  INLINE void set_offset_world_position(const LVecBase3f &pos);
  INLINE void set_offset_world_rotation(const LMatrix3f &r);
  INLINE void set_offset_world_quaternion(const LQuaternionf &q);
  INLINE void clear_offset();
  INLINE int is_offset();
  INLINE LPoint3f get_offset_position() const;
  INLINE LMatrix3f get_offset_rotation() const;
  INLINE LQuaternionf get_offset_quaternion() const;

  // int get_surface_type() ; int get_collide_id() ; int set_collide_id( int
  // collide_id); void set_surface_type( int surface_type);

  // int test_collide_id( int collide_id);

  OdeSpace get_space() const;
  EXTENSION(INLINE PyObject *get_converted_space() const);

  virtual void write(std::ostream &out = std::cout, unsigned int indent=0) const;
  operator bool () const;
  INLINE int compare_to(const OdeGeom &other) const;

  EXTENSION(PyObject *convert() const);
  OdeBoxGeom convert_to_box() const;
  OdeCappedCylinderGeom convert_to_capped_cylinder() const;
  // OdeConvexGeom convert_to_convex() const;
  OdeCylinderGeom convert_to_cylinder() const;
  // OdeHeightfieldGeom convert_to_heightfield() const;
  OdePlaneGeom convert_to_plane() const;
  OdeRayGeom convert_to_ray() const;
  OdeSphereGeom convert_to_sphere() const;
  OdeTriMeshGeom convert_to_tri_mesh() const;
  OdeSimpleSpace convert_to_simple_space() const;
  OdeHashSpace convert_to_hash_space() const;
  OdeQuadTreeSpace convert_to_quad_tree_space() const;

public:
  INLINE static int get_geom_class() { return -1; };

protected:
  dGeomID _id;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeGeom",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeGeom.I"

#endif
