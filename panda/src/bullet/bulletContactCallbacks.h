/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletContactCallbacks.h
 * @author enn0x
 * @date 2010-04-10
 */

#ifndef __BULLET_CONTACT_CALLBACKS_H__
#define __BULLET_CONTACT_CALLBACKS_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletWorld.h"
#include "bulletContactCallbackData.h"
#include "config_bullet.h" // required for: bullet_cat.debug()

#include "event.h"
#include "eventQueue.h"
#include "eventParameter.h"
#include "pandaNode.h"

struct UserPersistentData {
  PT(PandaNode) node0;
  PT(PandaNode) node1;
};

/**
 *
 */
static bool
contact_added_callback(btManifoldPoint &cp,
#if BT_BULLET_VERSION >= 281
                       const btCollisionObjectWrapper *wrap0,
#else
                       const btCollisionObject *obj0,
#endif
                       int id0,
                       int index0,
#if BT_BULLET_VERSION >= 281
                       const btCollisionObjectWrapper *wrap1,
#else
                       const btCollisionObject *obj1,
#endif
                       int id1,
                       int index1) {

  if (cp.m_userPersistentData == nullptr) {

#if BT_BULLET_VERSION >= 281
    PT(PandaNode) node0 = (PandaNode *)wrap0->getCollisionObject()->getUserPointer();
    PT(PandaNode) node1 = (PandaNode *)wrap1->getCollisionObject()->getUserPointer();
#else
    PT(PandaNode) node0 = (PandaNode *)obj0->getUserPointer();
    PT(PandaNode) node1 = (PandaNode *)obj1->getUserPointer();
#endif

    if (bullet_cat.is_debug()) {
      bullet_cat.debug() << "contact added: " << cp.m_userPersistentData << std::endl;
    }

    // Gather persistent data
    UserPersistentData *data = new UserPersistentData();
    data->node0 = node0;
    data->node1 = node1;

    cp.m_userPersistentData = (void *)data;

    // Send event
    if (bullet_enable_contact_events) {

      Event *event = new Event("bullet-contact-added");
      event->add_parameter(EventParameter(node0));
      event->add_parameter(EventParameter(node1));

      EventQueue::get_global_event_queue()->queue_event(event);
    }

    // Callback
    if (bullet_contact_added_callback) {

      BulletManifoldPoint mp(cp);
      BulletContactCallbackData cbdata(mp, node0, node1, id0, id1, index0, index1);

      bullet_contact_added_callback->do_callback(&cbdata);
    }
  }

  return true;
}

/**
 *
 */
static bool
contact_processed_callback(btManifoldPoint &cp,
                           void *body0,
                           void *body1) {

/*
  btCollisionObject *obj0 = (btCollisionObject *)body0;
  btCollisionObject *obj1 = (btCollisionObject *)body1;

  int flags0 = obj0->getCollisionFlags();
  int flags1 = obj1->getCollisionFlags();

  if ((flags0 & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
   || (flags1 & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)) {

    // do something...
  }
*/

  return false;
}

/**
 *
 */
static bool
contact_destroyed_callback(void *userPersistentData) {
  if (bullet_cat.is_debug()) {
    bullet_cat.debug() << "contact removed: " << userPersistentData << std::endl;
  }

  UserPersistentData *data = (UserPersistentData *)userPersistentData;

  // Send event
  if (bullet_enable_contact_events) {

    Event *event = new Event("bullet-contact-destroyed");
    event->add_parameter(EventParameter(data->node0));
    event->add_parameter(EventParameter(data->node1));

    EventQueue::get_global_event_queue()->queue_event(event);
  }

  // Delete persitent data
  delete data;

  return false;
}

#endif // __BULLET_CONTACT_CALLBACKS_H__
