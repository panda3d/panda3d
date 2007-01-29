// Filename: odeSpace.I
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


INLINE dSpaceID OdeSpace::
get_id() const {
  return _id;
}

INLINE void OdeSpace::
set_cleanup(int mode) {
  dSpaceSetCleanup(_id, mode);
}

INLINE int OdeSpace::
get_cleanup() const {
  return dSpaceGetCleanup(_id);
}

INLINE int OdeSpace::
get_num_geoms() const {
  return dSpaceGetNumGeoms(_id);
}

INLINE OdeSpace OdeSpace::
get_space() const {
  return OdeSpace(dGeomGetSpace((dGeomID)_id));
}

/*
INLINE void OdeSpace::
get_aabb(dReal aabb[6]) const {
  return dGeomGetAABB((dGeomID)_id, aabb[6]);
}
*/

INLINE int OdeSpace::
is_space() {
  return dGeomIsSpace((dGeomID)_id);
}

INLINE int OdeSpace::
get_class() const {
  return dGeomGetClass((dGeomID)_id);
}

INLINE void OdeSpace::
set_category_bits(unsigned long bits) {
  dGeomSetCategoryBits((dGeomID)_id, bits);
}

INLINE void OdeSpace::
set_collide_bits(unsigned long bits) {
  dGeomSetCollideBits((dGeomID)_id, bits);
}

INLINE void OdeSpace::
enable() {
  dGeomEnable((dGeomID)_id);
}

INLINE void OdeSpace::
disable() {
  dGeomDisable((dGeomID)_id);
}

INLINE int OdeSpace::
is_enabled() {
  return dGeomIsEnabled((dGeomID)_id);
}

