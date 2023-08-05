/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletContactCallbackData.h
 * @author enn0x
 * @date 2012-11-22
 */

#ifndef BULLETCONTACTCALLBACKDATA_H
#define BULLETCONTACTCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"
#include "callbackObject.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletManifoldPoint.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletContactCallbackData : public CallbackData {

PUBLISHED:
  INLINE explicit BulletContactCallbackData(BulletManifoldPoint &mp,
                                            PandaNode *node0, PandaNode *node1,
                                            int id0, int id1,
                                            int index0, int index1);

  INLINE BulletManifoldPoint &get_manifold() const;
  INLINE PandaNode *get_node0() const;
  INLINE PandaNode *get_node1() const;
  INLINE int get_part_id0() const;
  INLINE int get_part_id1() const;
  INLINE int get_index0() const;
  INLINE int get_index1() const;

  MAKE_PROPERTY(manifold, get_manifold);
  MAKE_PROPERTY(node0, get_node0);
  MAKE_PROPERTY(node1, get_node1);
  MAKE_PROPERTY(part_id0, get_part_id0);
  MAKE_PROPERTY(part_id1, get_part_id1);
  MAKE_PROPERTY(index0, get_index0);
  MAKE_PROPERTY(index1, get_index1);

private:
  BulletManifoldPoint &_mp;
  PandaNode *_node0;
  PandaNode *_node1;
  int _id0;
  int _id1;
  int _index0;
  int _index1;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "BulletContactCallbackData",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletContactCallbackData.I"

#endif // BULLETCONTACTCALLBACKDATA_H
