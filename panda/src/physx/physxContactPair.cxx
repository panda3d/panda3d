/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxContactPair.cxx
 * @author enn0x
 * @date 2009-12-19
 */

#include "physxContactPair.h"
#include "physxManager.h"
#include "physxActor.h"
#include "physxContactPoint.h"

TypeHandle PhysxContactPair::_type_handle;

/**
 * Returns the first of the two actors that makes up this pair.
 */
PhysxActor *PhysxContactPair::
get_actor_a() const {

  if (_pair.isDeletedActor[0]) {
    physx_cat.warning() << "actor A has been deleted" << std::endl;
    return nullptr;
  }

  NxActor *actorPtr = _pair.actors[0];
  return (actorPtr == nullptr) ? nullptr : (PhysxActor *)actorPtr->userData;
}

/**
 * Returns the second of the two actors that make up his pair.
 */
PhysxActor *PhysxContactPair::
get_actor_b() const {

  if (_pair.isDeletedActor[1]) {
    physx_cat.warning() << "actor B has been deleted" << std::endl;
    return nullptr;
  }

  NxActor *actorPtr = _pair.actors[1];
  return (actorPtr == nullptr) ? nullptr : (PhysxActor *)actorPtr->userData;
}

/**
 * Returns true if the first of the two actors is deleted.
 */
bool PhysxContactPair::
is_deleted_a() const {

  return _pair.isDeletedActor[0];
}

/**
 * Returns true if the second of the two actors is deleted.
 */
bool PhysxContactPair::
is_deleted_b() const {

  return _pair.isDeletedActor[1];
}

/**
 * Returns the total contact normal force that was applied for this pair, to
 * maintain nonpenetration constraints.
 *
 * You should set the ContactPairFlag CPF_notify_forces in order to receive
 * this value.
 *
 * @see PhysxScene::set_actor_pair_flag @see
 * PhysxScene::set_actor_group_pair_flag
 */
LVector3f PhysxContactPair::
get_sum_normal_force() const {

  return PhysxManager::nxVec3_to_vec3(_pair.sumNormalForce);
}

/**
 * Returns the total tangential force that was applied for this pair.
 *
 * You should set the ContactPairFlag CPF_notify_forces in order to receive
 * this value.
 *
 * @see PhysxScene::set_actor_pair_flag @see
 * PhysxScene::set_actor_group_pair_flag
 */
LVector3f PhysxContactPair::
get_sum_friction_force() const {

  return PhysxManager::nxVec3_to_vec3(_pair.sumFrictionForce);
}

/**
 * Returns the total number of contact points reported in this pair's contact
 * stream.
 *
 * This method is a helper for iterating over the pair's contact stream.
 */
unsigned int PhysxContactPair::
get_num_contact_points() {

  if (_contacts.size() == 0) {
    NxContactStreamIterator it(_pair.stream);
    while(it.goNextPair()) {
      while(it.goNextPatch()) {
        while(it.goNextPoint()) {
          PhysxContactPoint cp;
          cp.set(it);
          _contacts.push_back(cp);
        }
      }
    }
  }

  return _contacts.size();
}

/**
 * Returns an instance of PhysxContactPoint, which represents a single entry
 * of this pair's contact stream.
 *
 * This method is a helper for iterating over the pair's contact stream.
 */
PhysxContactPoint PhysxContactPair::
get_contact_point(unsigned int idx) const {

  nassertr(idx < _contacts.size(), PhysxContactPoint::empty());
  return _contacts[idx];
}
