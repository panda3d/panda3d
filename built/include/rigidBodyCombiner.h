/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rigidBodyCombiner.h
 * @author drose
 * @date 2007-02-22
 */

#ifndef RIGIDBODYCOMBINER_H
#define RIGIDBODYCOMBINER_H

#include "pandabase.h"

#include "pandaNode.h"
#include "nodeVertexTransform.h"
#include "pvector.h"

class NodePath;

/**
 * This is a special node that combines multiple independently-moving rigid
 * nodes into one Geom internally (or as few Geoms as possible), for the
 * purposes of improving rendering performance.
 *
 * To use it, parent a number of moving objects to this node and call
 * collect().  A child node is identified as "moving" if (a) it has a non-
 * identity transform initially, or (b) it is a ModelNode with the
 * preserve_transform flag set.  Any other nodes will be considered static,
 * and later transforms applied to them will not be identified.
 *
 * You should call collect() only at startup or if you change the set of
 * children; it is a relatively expensive call.
 *
 * Once you call collect(), you may change the transforms on the child nodes
 * freely without having to call collect() again.
 *
 * RenderEffects such as Billboards are not supported below this node.
 */
class EXPCL_PANDA_GRUTIL RigidBodyCombiner : public PandaNode {
PUBLISHED:
  explicit RigidBodyCombiner(const std::string &name);
protected:
  RigidBodyCombiner(const RigidBodyCombiner &copy);
  virtual PandaNode *make_copy() const;

PUBLISHED:
  void collect();

  NodePath get_internal_scene();
  MAKE_PROPERTY(internal_scene, get_internal_scene);

public:
  // From parent class PandaNode
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

private:
  void r_collect(PandaNode *node, const RenderState *state,
                 const VertexTransform *transform);
  PT(GeomVertexData) convert_vd(const VertexTransform *transform,
                                const GeomVertexData *orig);

  PT(PandaNode) _internal_root;

  typedef pvector< PT(NodeVertexTransform) > Transforms;
  Transforms _internal_transforms;

  class VDUnifier {
  public:
    INLINE VDUnifier(const VertexTransform *transform,
                     const GeomVertexData *orig);
    INLINE bool operator < (const VDUnifier &other) const;

    const VertexTransform *_transform;
    const GeomVertexData *_orig;
  };

  typedef pmap<VDUnifier, PT(GeomVertexData) > VDTable;
  VDTable _vd_table;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "RigidBodyCombiner",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "rigidBodyCombiner.I"

#endif
