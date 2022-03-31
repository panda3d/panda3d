/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeSpace.h"
#include "odeGeom.h"
#include "odeCollisionEntry.h"
#include "odeSimpleSpace.h"
#include "odeQuadTreeSpace.h"
#include "odeHashSpace.h"

#include "throw_event.h"

TypeHandle OdeSpace::_type_handle;
// this data is used in auto_collide
const int OdeSpace::MAX_CONTACTS = 16;
OdeWorld* OdeSpace::_static_auto_collide_world;
OdeSpace* OdeSpace::_static_auto_collide_space;
dJointGroupID OdeSpace::_static_auto_collide_joint_group;

OdeSpace::
OdeSpace(dSpaceID id) :
  _id(id) {
  _auto_collide_world = nullptr;
  _auto_collide_joint_group = nullptr;
}

OdeSpace::
~OdeSpace() {
}

void OdeSpace::
destroy() {
  nassertv(_id);
  dSpaceDestroy(_id);
}

int OdeSpace::
query(const OdeGeom& geom) const {
  nassertr(_id, 0);
  return dSpaceQuery(_id, geom.get_id());
}

int OdeSpace::
query(const OdeSpace& space) const {
  nassertr(_id, 0);
  return dSpaceQuery(_id, (dGeomID)space.get_id());
}

void OdeSpace::
add(OdeSpace& space) {
  nassertv(_id);
  dSpaceAdd(_id, (dGeomID)space.get_id());
}

void OdeSpace::
remove(OdeSpace& space) {
  nassertv(_id);
  dSpaceRemove(_id, (dGeomID)space.get_id());
}

void OdeSpace::
add(OdeGeom& geom) {
  nassertv(_id);
  dSpaceAdd(_id, geom.get_id());
}

void OdeSpace::
remove(OdeGeom& geom) {
  nassertv(_id);
  dSpaceRemove(_id, geom.get_id());
}

void OdeSpace::
clean() {
  nassertv(_id);
  dSpaceClean(_id);
}

OdeGeom  OdeSpace::
get_geom(int i) {
  nassertr(_id, OdeGeom(nullptr));
  return OdeGeom(dSpaceGetGeom(_id, i));
}


void OdeSpace::
write(std::ostream &out, unsigned int indent) const {
  out.width(indent); out << "" << get_type() << "(id = " << _id << ")";
}

OdeSpace::
operator bool () const {
  return (_id != nullptr);
}

void OdeSpace::
set_auto_collide_world(OdeWorld &world) {
  _auto_collide_world = &world;
}

void OdeSpace::
set_auto_collide_joint_group(OdeJointGroup &joint_group) {
  _auto_collide_joint_group = joint_group.get_id();
}

void OdeSpace::
auto_collide() {
  if (_auto_collide_world == nullptr) {
    odespace_cat.error() << "No collide world has been set!\n";
  } else {
    nassertv(_id != nullptr);
    _static_auto_collide_space = this;
    _static_auto_collide_world = _auto_collide_world;
    _static_auto_collide_joint_group = _auto_collide_joint_group;
    dSpaceCollide(_id, this, &auto_callback);
  }
}

void OdeSpace::
auto_callback(void *data, dGeomID o1, dGeomID o2) {
// uses data stored on the world to resolve collisions so you don't have to
// use near_callbacks in python
  int i;
  dBodyID b1 = dGeomGetBody(o1);
  dBodyID b2 = dGeomGetBody(o2);

  dContact contact[OdeSpace::MAX_CONTACTS];

  int surface1 = _static_auto_collide_space->get_surface_type(o1);
  int surface2 = _static_auto_collide_space->get_surface_type(o2);

  nassertv(_static_auto_collide_world != nullptr);
  sSurfaceParams collide_params;
  collide_params = _static_auto_collide_world->get_surface(surface1, surface2);

  for (i=0; i < OdeSpace::MAX_CONTACTS; i++) {
    contact[i].surface.mode = collide_params.colparams.mode;
    contact[i].surface.mu = collide_params.colparams.mu;
    contact[i].surface.mu2 = collide_params.colparams.mu2;
    contact[i].surface.bounce = collide_params.colparams.bounce;
    contact[i].surface.bounce_vel = collide_params.colparams.bounce_vel;
    contact[i].surface.soft_cfm = collide_params.colparams.soft_cfm;
  }

  static int numc = 0;
  numc = dCollide(o1, o2, OdeSpace::MAX_CONTACTS, &contact[0].geom, sizeof(dContact));

  if (numc) {
    if (odespace_cat.is_debug()) {
      odespace_cat.debug() << "collision between geoms " << o1 << " and " << o2 << "\n";
      odespace_cat.debug() << "collision between body " << b1 << " and " << b2 << "\n";
      odespace_cat.debug() << "surface1= "<< surface1 << " surface2=" << surface2 << "\n";
    }

    PT(OdeCollisionEntry) entry;
    if (!_static_auto_collide_space->_collision_event.empty()) {
      entry = new OdeCollisionEntry;
      entry->_geom1 = o1;
      entry->_geom2 = o2;
      entry->_body1 = b1;
      entry->_body2 = b2;
      entry->_num_contacts = numc;
      entry->_contact_geoms = new OdeContactGeom[numc];
    }

    for(i=0; i < numc; i++) {
      dJointID c = dJointCreateContact(_static_auto_collide_world->get_id(), _static_auto_collide_joint_group, contact + i);
      if ((_static_auto_collide_space->get_collide_id(o1) >= 0) && (_static_auto_collide_space->get_collide_id(o2) >= 0)) {
        dJointAttach(c, b1, b2);
      }
      if (!_static_auto_collide_space->_collision_event.empty()) {
        entry->_contact_geoms[i] = contact[i].geom;
      }
    }
    _static_auto_collide_world->set_dampen_on_bodies(b1, b2, collide_params.dampen);

    if (!_static_auto_collide_space->_collision_event.empty()) {
      throw_event(_static_auto_collide_space->_collision_event, EventParameter(entry));
    }
  }
}

OdeSimpleSpace OdeSpace::
convert_to_simple_space() const {
  nassertr(_id != nullptr, OdeSimpleSpace(nullptr));
  nassertr(get_class() == OdeGeom::GC_simple_space, OdeSimpleSpace(nullptr));
  return OdeSimpleSpace(_id);
}

OdeHashSpace OdeSpace::
convert_to_hash_space() const {
  nassertr(_id != nullptr, OdeHashSpace(nullptr));
  nassertr(get_class() == OdeGeom::GC_hash_space, OdeHashSpace(nullptr));
  return OdeHashSpace(_id);
}

OdeQuadTreeSpace OdeSpace::
convert_to_quad_tree_space() const {
  nassertr(_id != nullptr, OdeQuadTreeSpace(nullptr));
  nassertr(get_class() == OdeGeom::GC_quad_tree_space, OdeQuadTreeSpace(nullptr));
  return OdeQuadTreeSpace(_id);
}


void OdeSpace::
set_surface_type( int surfaceType, dGeomID id){
  _geom_surface_map[id]= surfaceType;
}

void OdeSpace::
set_surface_type(OdeGeom& geom,  int surfaceType){
  dGeomID id = geom.get_id();
  set_surface_type(surfaceType, id);
}

int OdeSpace::
get_surface_type(dGeomID id) {
  GeomSurfaceMap::iterator iter = _geom_surface_map.find(id);
  if (iter != _geom_surface_map.end()) {
    return iter->second;
  }
  return 0;
}

int OdeSpace::
get_surface_type(OdeGeom& geom) {
  dGeomID id = geom.get_id();
  return get_surface_type(id);
}


int OdeSpace::
set_collide_id( int collide_id, dGeomID id) {
  _geom_collide_id_map[id]= collide_id;
  return _geom_collide_id_map[id];
}

int OdeSpace::
set_collide_id( OdeGeom& geom, int collide_id) {
  dGeomID id = geom.get_id();
  return set_collide_id(collide_id, id);
}

int OdeSpace::
get_collide_id(OdeGeom& geom) {
  dGeomID id = geom.get_id();
  return get_collide_id(id);
}


int OdeSpace::
get_collide_id(dGeomID id) {
  GeomCollideIdMap::iterator iter = _geom_collide_id_map.find(id);
  if (iter != _geom_collide_id_map.end()) {
    return iter->second;
  }
  return 0;
}
