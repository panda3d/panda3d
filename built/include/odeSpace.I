/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace.I
 * @author joswilso
 * @date 2006-12-27
 */

/**
 * Returns true if the ID is 0, meaning the OdeSpace does not point to a valid
 * space.  It is an error to call a method on an empty space.  Note that an
 * empty OdeSpace also evaluates to False.
 */
INLINE bool OdeSpace::
is_empty() const {
  return (_id == 0);
}

/**
 * Returns the underlying dSpaceID.
 */
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

INLINE void OdeSpace::
get_AABB(LVecBase3f &min, LVecBase3f &max) const {
  dReal result[6];
  dGeomGetAABB((dGeomID)_id, result);
  min.set(result[0], result[2], result[4]);
  max.set(result[1], result[3], result[5]);
}

INLINE int OdeSpace::
is_space() {
  return dGeomIsSpace((dGeomID)_id);
}

INLINE int OdeSpace::
get_class() const {
  return dGeomGetClass((dGeomID)_id);
}

INLINE void OdeSpace::
set_category_bits(const BitMask32 &bits) {
  dGeomSetCategoryBits((dGeomID)_id, bits.get_word());
}

INLINE void OdeSpace::
set_collide_bits(const BitMask32 &bits) {
  dGeomSetCollideBits((dGeomID)_id, bits.get_word());
}

INLINE BitMask32 OdeSpace::
get_category_bits() {
  return BitMask32(dGeomGetCategoryBits((dGeomID)_id));
}

INLINE BitMask32 OdeSpace::
get_collide_bits() {
  return BitMask32(dGeomGetCollideBits((dGeomID)_id));
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

INLINE void OdeSpace::
set_collision_event(const std::string &event_name) {
  _collision_event = event_name;
}

INLINE std::string OdeSpace::
get_collision_event() {
  return _collision_event;
}
