/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHelper.h
 * @author enn0x
 * @date 2011-01-19
 */

#ifndef __BULLET_HELPER_H__
#define __BULLET_HELPER_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "collisionNode.h"
#include "nodePath.h"
#include "nodePathCollection.h"

class BulletSoftBodyNode;

/**
 *
 */
class EXPCL_PANDABULLET BulletHelper {

PUBLISHED:

  // Collision shapes
  static NodePathCollection from_collision_solids(NodePath &np, bool clear=false);

  // Internal names
  INLINE static PT(InternalName) get_sb_index();
  INLINE static PT(InternalName) get_sb_flip();

  // Geom vertex data
  static CPT(GeomVertexFormat) add_sb_index_column(const GeomVertexFormat *format);
  static CPT(GeomVertexFormat) add_sb_flip_column(const GeomVertexFormat *format);

  // Geom utils
  static PT(Geom) make_geom_from_faces(BulletSoftBodyNode *node,
      const GeomVertexFormat *format=nullptr,
      bool two_sided=false);

  static PT(Geom) make_geom_from_links(BulletSoftBodyNode *node,
      const GeomVertexFormat *format=nullptr);

  static void make_texcoords_for_patch(Geom *geom, int resx, int resy);

  MAKE_PROPERTY(sb_index, get_sb_index);
  MAKE_PROPERTY(sb_flip, get_sb_flip);

private:
  static PT(InternalName) _sb_index;
  static PT(InternalName) _sb_flip;

  static bool is_tangible(CollisionNode *cnode);

  static PT(Geom) make_geom(BulletSoftBodyNode *node,
      const GeomVertexFormat *format,
      bool use_faces,
      bool two_sided);
};

#include "bulletHelper.I"

#endif // __BULLET_HELPER_H__
