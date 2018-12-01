/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODESPACE_H
#define ODESPACE_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "bitMask.h"

// included for collision tests
#include "odeWorld.h"
#include "odeJointGroup.h"

#include "ode_includes.h"

class OdeGeom;
class OdeTriMeshGeom;
class OdeSimpleSpace;
class OdeHashSpace;
class OdeQuadTreeSpace;

/**
 *
 */
class EXPCL_PANDAODE OdeSpace : public TypedObject {
  friend class OdeGeom;
  static const int MAX_CONTACTS;

public:
  OdeSpace(dSpaceID id);

PUBLISHED:
  virtual ~OdeSpace();
  void destroy();
  INLINE bool is_empty() const;

  INLINE void set_cleanup(int mode);
  INLINE int get_cleanup() const;
  int query(const OdeGeom& geom) const;
  int query(const OdeSpace& space) const;
  INLINE int get_num_geoms() const;
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
  void set_auto_collide_world(OdeWorld&);
  void set_auto_collide_joint_group(OdeJointGroup&);

  void add(OdeGeom& geom);
  void add(OdeSpace& space);
  void remove(OdeGeom& geom);
  void remove(OdeSpace& space);
  void clean();
  OdeGeom get_geom(int i); // Not INLINE because of forward declaration
  // static int get_surface_type(OdeSpace * self, dGeomID o1);

  INLINE OdeSpace get_space() const;

  virtual void write(std::ostream &out = std::cout, unsigned int indent=0) const;
  operator bool () const;

  OdeSimpleSpace convert_to_simple_space() const;
  OdeHashSpace convert_to_hash_space() const;
  OdeQuadTreeSpace convert_to_quad_tree_space() const;

  EXTENSION(PyObject *convert() const);
  EXTENSION(INLINE PyObject *get_converted_geom(int i) const);
  EXTENSION(INLINE PyObject *get_converted_space() const);

  void auto_collide();
  EXTENSION(int collide(PyObject* arg, PyObject* near_callback));
  int set_collide_id(int collide_id, dGeomID id);
  int set_collide_id(OdeGeom& geom, int collide_id);
  void set_surface_type( int surface_type, dGeomID id);
  void set_surface_type(OdeGeom& geom, int surface_type);
  int get_surface_type(dGeomID o1);
  int get_surface_type(OdeGeom& geom);
  int get_collide_id(dGeomID o1);
  int get_collide_id(OdeGeom& geom);

  INLINE void set_collision_event(const std::string &event_name);
  INLINE std::string get_collision_event();

public:
  static void auto_callback(void*, dGeomID, dGeomID);

  INLINE dSpaceID get_id() const;
  static OdeWorld* _static_auto_collide_world;
  static OdeSpace* _static_auto_collide_space;
  static dJointGroupID _static_auto_collide_joint_group;
  static int contactCount;
  std::string _collision_event;

protected:
  dSpaceID _id;
  OdeWorld* _auto_collide_world;
  dJointGroupID _auto_collide_joint_group;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeSpace",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  typedef pmap<dGeomID, int> GeomSurfaceMap;
  GeomSurfaceMap _geom_surface_map;

  typedef pmap<dGeomID, int> GeomCollideIdMap;
  GeomCollideIdMap _geom_collide_id_map;

};

#include "odeSpace.I"

#endif
