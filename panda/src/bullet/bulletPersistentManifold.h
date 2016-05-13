/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletPersistentManifold.h
 * @author enn0x
 * @date 2010-03-07
 */

#ifndef __BULLET_PERSISTENT_MANIFOLD_H__
#define __BULLET_PERSISTENT_MANIFOLD_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "pandaNode.h"

class BulletManifoldPoint;

/**
 *
 */
class EXPCL_PANDABULLET BulletPersistentManifold {

PUBLISHED:
  INLINE ~BulletPersistentManifold();

  PandaNode *get_node0();
  PandaNode *get_node1();

  int get_num_manifold_points() const;
  BulletManifoldPoint *get_manifold_point(int idx) const;
  MAKE_SEQ(get_manifold_points, get_num_manifold_points, get_manifold_point);

  PN_stdfloat get_contact_breaking_threshold() const;
  PN_stdfloat get_contact_processing_threshold() const;

  void clear_manifold();

  MAKE_PROPERTY(node0, get_node0);
  MAKE_PROPERTY(node1, get_node1);
  MAKE_SEQ_PROPERTY(manifold_points, get_num_manifold_points, get_manifold_point);
  MAKE_PROPERTY(contact_breaking_threshold, get_contact_breaking_threshold);
  MAKE_PROPERTY(contact_processing_threshold, get_contact_processing_threshold);

public:
  BulletPersistentManifold(btPersistentManifold *manifold);

private:
  btPersistentManifold *_manifold;
};

#include "bulletPersistentManifold.I"

#endif // __BULLET_PERSISTENT_MANIFOLD_H__
