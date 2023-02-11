/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletContactResult.h
 * @author enn0x
 * @date 2010-03-08
 */

#ifndef BULLETCONTACTRESULT_H
#define BULLETCONTACTRESULT_H

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletManifoldPoint.h"

#include "pvector.h"

/**
 *
 */
struct EXPCL_PANDABULLET BulletContact {

public:
  BulletContact();
  BulletContact(btManifoldPoint &mp);
  BulletContact(const BulletContact &other);

PUBLISHED:
  INLINE BulletManifoldPoint get_manifold_point();
  INLINE PandaNode *get_node0() const;
  INLINE PandaNode *get_node1() const;
  INLINE int get_idx0() const;
  INLINE int get_idx1() const;
  INLINE int get_part_id0() const;
  INLINE int get_part_id1() const;

  MAKE_PROPERTY(manifold_point, get_manifold_point);
  MAKE_PROPERTY(node0, get_node0);
  MAKE_PROPERTY(node1, get_node1);
  MAKE_PROPERTY(idx0, get_idx0);
  MAKE_PROPERTY(idx1, get_idx1);
  MAKE_PROPERTY(part_id0, get_part_id0);
  MAKE_PROPERTY(part_id1, get_part_id1);

private:
  static btManifoldPoint _empty;

  BulletManifoldPoint _mp;

  PT(PandaNode) _node0;
  PT(PandaNode) _node1;

  int _part_id0;
  int _part_id1;
  int _idx0;
  int _idx1;

  friend struct BulletContactResult;
};

/**
 *
 */
struct EXPCL_PANDABULLET BulletContactResult : public btCollisionWorld::ContactResultCallback {

PUBLISHED:
  INLINE int get_num_contacts() const;
  INLINE BulletContact get_contact(int idx);
  MAKE_SEQ(get_contacts, get_num_contacts, get_contact);
  MAKE_SEQ_PROPERTY(contacts, get_num_contacts, get_contact);

public:
#if BT_BULLET_VERSION >= 281
  virtual bool needsCollision(btBroadphaseProxy *proxy0) const;

  virtual btScalar addSingleResult(btManifoldPoint &mp,
      const btCollisionObjectWrapper *wrap0, int part_id0, int idx0,
      const btCollisionObjectWrapper *wrap1, int part_id1, int idx1);
#else
  virtual btScalar addSingleResult(btManifoldPoint &mp,
      const btCollisionObject *obj0, int part_id0, int idx0,
      const btCollisionObject *obj1, int part_id1, int idx1);
#endif

protected:
  BulletContactResult();

#if BT_BULLET_VERSION >= 281
  void use_filter(btOverlapFilterCallback *cb, btBroadphaseProxy *proxy);
#endif

private:
  static BulletContact _empty;

  btAlignedObjectArray<BulletContact> _contacts;

#if BT_BULLET_VERSION >= 281
  bool _filter_set;
  btOverlapFilterCallback *_filter_cb;
  btBroadphaseProxy *_filter_proxy;
#endif

  friend class BulletWorld;
};

#include "bulletContactResult.I"

#endif // BULLETCONTACTRESULT_H
