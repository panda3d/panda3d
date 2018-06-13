/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUtil.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "odeUtil.h"

dReal OdeUtil::OC_infinity = dInfinity;

/**
 * Returns the joint that connects the given bodies.
 */
OdeJoint OdeUtil::
get_connecting_joint(const OdeBody &body1, const OdeBody &body2) {
  return OdeJoint(dConnectingJoint(body1.get_id(),body2.get_id()));
}

/**
 * Returns a collection of joints connecting the specified bodies.
 */
OdeJointCollection OdeUtil::
get_connecting_joint_list(const OdeBody &body1, const OdeBody &body2) {
  const int max_possible_joints = std::min(body1.get_num_joints(), body1.get_num_joints());

  dJointID *joint_list = (dJointID *)PANDA_MALLOC_ARRAY(max_possible_joints * sizeof(dJointID));
  int num_joints = dConnectingJointList(body1.get_id(), body2.get_id(),
          joint_list);
  OdeJointCollection joints;
  for (int i = 0; i < num_joints; i++) {
    joints.add_joint(OdeJoint(joint_list[i]));
  }

  PANDA_FREE_ARRAY(joint_list);
  return joints;
}

/**
 * Returns 1 if the given bodies are connected by a joint, returns 0
 * otherwise.
 */
int OdeUtil::
are_connected(const OdeBody &body1, const OdeBody &body2) {
  return dAreConnected(body1.get_id(),body2.get_id());
}

/**
 * Returns 1 if the given bodies are connected by a joint that does not match
 * the given joint_type, returns 0 otherwise.  This is useful for deciding
 * whether to add contact joints between two bodies: if they are already
 * connected by non-contact joints then it may not be appropriate to add
 * contacts, however it is okay to add more contact between bodies that
 * already have contacts.
 */
int OdeUtil::
are_connected_excluding(const OdeBody &body1,
                        const OdeBody &body2,
                        const int joint_type) {
  return dAreConnectedExcluding(body1.get_id(),
        body2.get_id(),
        joint_type);
}

/**
 * Given two geometry objects that potentially touch (geom1 and geom2),
 * generate contact information for them.  Returns an OdeCollisionEntry.
 */
PT(OdeCollisionEntry) OdeUtil::
collide(const OdeGeom &geom1, const OdeGeom &geom2, const short int max_contacts) {
  dContactGeom *contact_list = (dContactGeom *)PANDA_MALLOC_ARRAY(max_contacts * sizeof(dContactGeom));
  int num_contacts = dCollide(geom1.get_id(), geom2.get_id(), max_contacts, contact_list, sizeof(dContactGeom));
  PT(OdeCollisionEntry) entry = new OdeCollisionEntry();
  entry->_geom1 = geom1.get_id();
  entry->_geom2 = geom2.get_id();
  entry->_body1 = dGeomGetBody(geom1.get_id());
  entry->_body2 = dGeomGetBody(geom2.get_id());
  entry->_num_contacts = num_contacts;
  entry->_contact_geoms = new OdeContactGeom[num_contacts];
  for (int i = 0; i < num_contacts; i++) {
    entry->_contact_geoms[i] = contact_list[i];
  }

  PANDA_FREE_ARRAY(contact_list);
  return entry;
}

OdeGeom OdeUtil::
space_to_geom(const OdeSpace &space) {
  return OdeGeom((dGeomID)space.get_id());
}
