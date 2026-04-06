/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compassEffect.h
 * @author drose
 * @date 2002-07-16
 */

#ifndef COMPASSEFFECT_H
#define COMPASSEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

/**
 * A CompassEffect causes a node to inherit its rotation (or pos or scale, if
 * specified) from some other reference node in the graph, or more often from
 * the root.
 *
 * In its purest form, a CompassEffect is used to keep the node's rotation
 * fixed relative to the top of the scene graph, despite other transforms that
 * may exist above the node.  Hence the name: the node behaves like a magnetic
 * compass, always pointing in the same direction.
 *
 * As an couple of generalizing extensions, the CompassEffect may also be set
 * up to always orient its node according to some other reference node than
 * the root of the scene graph.  Furthermore, it may optionally adjust any of
 * pos, rotation, or scale, instead of necessarily rotation; and it may adjust
 * individual pos and scale components.  (Rotation may not be adjusted on an
 * individual component basis; that's just asking for trouble.)
 *
 * Be careful when using the pos and scale modes.  In these modes, it's
 * possible for the CompassEffect to move its node far from its normal
 * bounding volume, causing culling to fail.  If this is an issue, you may
 * need to explicitly set a large (or infinite) bounding volume on the effect
 * node.
 */
class EXPCL_PANDA_PGRAPH CompassEffect : public RenderEffect {
private:
  INLINE CompassEffect();

PUBLISHED:
  enum Properties {
    P_x     = 0x001,
    P_y     = 0x002,
    P_z     = 0x004,
    P_pos   = 0x007,
    P_rot   = 0x008,
    P_sx    = 0x010,
    P_sy    = 0x020,
    P_sz    = 0x040,
    P_scale = 0x070,
    P_all   = 0x07f,
  };
  static CPT(RenderEffect) make(const NodePath &reference,
                                int properties = P_rot);

  INLINE const NodePath &get_reference() const;
  INLINE int get_properties() const;

public:
  virtual bool safe_to_transform() const;
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  virtual bool has_adjust_transform() const;
  virtual void adjust_transform(CPT(TransformState) &net_transform,
                                CPT(TransformState) &node_transform,
                                const PandaNode *node) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  NodePath _reference;
  int _properties;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "CompassEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "compassEffect.I"

#endif
