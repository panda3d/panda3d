// Filename: polylightEffect.h
// Created by:  sshodhan (01Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef POLYLIGHTEFFECT_H
#define POLYLIGHTEFFECT_H

#include "pandabase.h"


#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"
#include "polylightNode.h"
#include "pmap.h"
#include "notify.h"


////////////////////////////////////////////////////////////////////
//       Class : PolylightEffect
// Description : A PolylightEffect can be used on a node to define a
//               LightGroup  for that node. A LightGroup contains 
//               PolylightNodes which are essentially nodes that add 
//               color to the polygons of a model based on distance.
//               PolylightNode is a cheap way to get lighting effects
//               specially for night scenes
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolylightEffect : public RenderEffect {
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
  static CPT(RenderEffect) make(float weight, ContribType contrib, LPoint3f effect_center);
  static CPT(RenderEffect) make(float weight, ContribType contrib, LPoint3f effect_center, const LightGroup &lights);
  CPT(RenderEffect) add_light(const NodePath &newlight) const;
  CPT(RenderEffect) remove_light(const NodePath &newlight) const;
  CPT(RenderEffect) set_weight(float w) const;
  CPT(RenderEffect) set_contrib(ContribType c) const;
  CPT(RenderEffect) set_effect_center(LPoint3f ec) const;
  INLINE float get_weight() const;
  INLINE ContribType get_contrib() const;
  INLINE LPoint3f get_effect_center()const;

  bool has_light(const NodePath &light) const;

public:
  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  CPT(RenderAttrib) do_poly_light(const NodePath &root, const CullTraverserData *data, const TransformState *node_transform) const;

  virtual void output(ostream &out) const;

private:
  ContribType _contribution_type;
  float _weight;
  LightGroup _lightgroup;
  LPoint3f _effect_center;

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

ostream &operator << (ostream &out, PolylightEffect::ContribType ct);

#endif



