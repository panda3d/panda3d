/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file polylightEffect.h
 * @author sshodhan
 * @date 2004-06-01
 */

#ifndef POLYLIGHTEFFECT_H
#define POLYLIGHTEFFECT_H

#include "pandabase.h"


#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"
#include "polylightNode.h"
#include "pmap.h"
#include "pnotify.h"
#include "sceneSetup.h"


/**
 * A PolylightEffect can be used on a node to define a LightGroup  for that
 * node.  A LightGroup contains PolylightNodes which are essentially nodes
 * that add color to the polygons of a model based on distance.  PolylightNode
 * is a cheap way to get lighting effects specially for night scenes
 */
class EXPCL_PANDA_PGRAPH PolylightEffect : public RenderEffect {
PUBLISHED:
  enum ContribType {
    CT_proximal,
    CT_all,
  };

  typedef pvector< NodePath > LightGroup;

protected:
  INLINE PolylightEffect();
  INLINE PolylightEffect(const PolylightEffect &copy);

PUBLISHED:
  static CPT(RenderEffect) make();
  static CPT(RenderEffect) make(PN_stdfloat weight, ContribType contrib, const LPoint3 &effect_center);
  static CPT(RenderEffect) make(PN_stdfloat weight, ContribType contrib, const LPoint3 &effect_center, const LightGroup &lights);
  CPT(RenderEffect) add_light(const NodePath &newlight) const;
  CPT(RenderEffect) remove_light(const NodePath &newlight) const;
  CPT(RenderEffect) set_weight(PN_stdfloat w) const;
  CPT(RenderEffect) set_contrib(ContribType c) const;
  CPT(RenderEffect) set_effect_center(const LPoint3 &ec) const;
  INLINE PN_stdfloat get_weight() const;
  INLINE ContribType get_contrib() const;
  INLINE LPoint3 get_effect_center()const;

  bool has_light(const NodePath &light) const;

public:
  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  CPT(RenderAttrib) do_poly_light(const SceneSetup *scene, const CullTraverserData *data, const TransformState *node_transform) const;
  // CPT(RenderAttrib) do_poly_light(const NodePath &root, const
  // CullTraverserData *data, const TransformState *node_transform) const;

  virtual void output(std::ostream &out) const;

private:
  ContribType _contribution_type;
  PN_stdfloat _weight;
  LightGroup _lightgroup;
  LPoint3 _effect_center;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "PolylightEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "polylightEffect.I"

std::ostream &operator << (std::ostream &out, PolylightEffect::ContribType ct);

#endif
