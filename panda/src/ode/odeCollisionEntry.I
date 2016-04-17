/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeCollisionEntry.I
 * @author rdb
 * @date 2009-03-13
 */

/**
 *
 */
INLINE OdeCollisionEntry::
OdeCollisionEntry() {
}

/**
 * Returns the first geom in the collision.
 */
INLINE OdeGeom OdeCollisionEntry::
get_geom1() const {
  return OdeGeom(_geom1);
}

/**
 * Returns the second geom in the collision.
 */
INLINE OdeGeom OdeCollisionEntry::
get_geom2() const {
  return OdeGeom(_geom2);
}

/**
 * Returns the first body in the collision.
 */
INLINE OdeBody OdeCollisionEntry::
get_body1() const {
  return OdeBody(_body1);
}

/**
 * Returns the second body in the collision.
 */
INLINE OdeBody OdeCollisionEntry::
get_body2() const {
  return OdeBody(_body2);
}

/**
 * Returns the number of contacts in the collision.
 */
INLINE size_t OdeCollisionEntry::
get_num_contacts() const {
  return _num_contacts;
}

/**
 * Returns the nth contact geom in the collision.
 */
INLINE OdeContactGeom OdeCollisionEntry::
get_contact_geom(size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, OdeContactGeom());
  return _contact_geoms[n];
}

/**
 * Returns the nth contact geom in the collision.
 */
INLINE OdeContactGeom OdeCollisionEntry::
operator [] (size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, OdeContactGeom());
  return _contact_geoms[n];
}

/**
 * Returns the nth contact point in the collision.  This does exactly the same
 * as get_contact_geom(n).get_pos().
 */
INLINE LPoint3f OdeCollisionEntry::
get_contact_point(size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, LPoint3f::zero());
  return _contact_geoms[n].get_pos();
}

/**
 * An OdeCollisionEntry evaluates to False if it holds no contacts.
 */
INLINE OdeCollisionEntry::
operator bool () const {
  return (_num_contacts != 0);
}

/**
 * Returns true if the entry holds no contacts.
 */
INLINE bool OdeCollisionEntry::
is_empty() const {
  return (_num_contacts == 0);
}
