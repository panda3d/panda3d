// Filename: odeSpace.h
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

#ifndef ODESPACE_H
#define ODESPACE_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "bitMask.h"

//included for collision tests
#include "odeWorld.h"
#include "odeJointGroup.h"

#include "ode_includes.h"

#ifdef HAVE_PYTHON
  #include "py_panda.h"
  #include "Python.h"
#endif

class OdeGeom;
class OdeTriMeshGeom;
class OdeSimpleSpace;
class OdeHashSpace;
class OdeQuadTreeSpace;

////////////////////////////////////////////////////////////////////
//       Class : OdeSpace
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeSpace : public TypedObject {
  friend class OdeGeom;
  static const int MAX_CONTACTS;

protected:
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
  //static int get_surface_type(OdeSpace * self, dGeomID o1);
  
  INLINE OdeSpace get_space() const;

  virtual void write(ostream &out = cout, unsigned int indent=0) const;
  operator bool () const;

  OdeSimpleSpace convert_to_simple_space() const;
  OdeHashSpace convert_to_hash_space() const;
  OdeQuadTreeSpace convert_to_quad_tree_space() const;
  
  int auto_collide();
#ifdef HAVE_PYTHON
  int collide(PyObject* arg, PyObject* near_callback);
#endif
  static double get_contact_data(int data_index);
  int get_contact_id(int data_index, int first = 0);
  int set_collide_id(int collide_id, dGeomID id);
  int set_collide_id(OdeGeom& geom, int collide_id);
  void set_surface_type( int surface_type, dGeomID id);
  void set_surface_type(OdeGeom& geom, int surface_type);
  int get_surface_type(dGeomID o1);
  int get_surface_type(OdeGeom& geom);
  int get_collide_id(dGeomID o1);
  int get_collide_id(OdeGeom& geom);

  INLINE void set_collision_event(const string &event_name);
  INLINE string get_collision_event();

public:
  static void auto_callback(void*, dGeomID, dGeomID);
#ifdef HAVE_PYTHON
  static void near_callback(void*, dGeomID, dGeomID);
#endif
  
  INLINE dSpaceID get_id() const;
  static OdeWorld* _collide_world;
  static OdeSpace* _collide_space;
  static dJointGroupID _collide_joint_group;
#ifdef HAVE_PYTHON
  static PyObject* _python_callback;
#endif
  static int contactCount;
  string _collision_event;

  static double contact_data[192]; // 64 times three
  static int contact_ids[128]; // 64 times two

protected:
  dSpaceID _id;
  int _g; // REMOVE ME
  OdeWorld* my_world;

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

