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
//				 LightGroup  for that node. A LightGroup contains 
//               Polylights which are essentially nodes that add 
//			     color to the polygons of a model based on distance.
//				 PolylightNode is a cheap way to get lighting effects
//               specially for night scenes
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolylightEffect : public RenderEffect {
private:
  INLINE PolylightEffect();
  

PUBLISHED:
  
  static CPT(RenderEffect) make(string contribution_type= "proximal", float weight=0.9);
  INLINE void enable();
  INLINE void disable();
  INLINE bool add_light(string lightname, NodePath *newlight);
  INLINE bool remove_light(string lightname);
  INLINE bool remove_all();
  INLINE bool set_weight(float w);
  INLINE float get_weight() const;
  INLINE bool set_contrib(string type);  
  INLINE string get_contrib() const;
  INLINE bool is_enabled()const;

public:
  CPT(RenderAttrib) do_poly_light(const CullTraverserData *data, const TransformState *node_transform) const;


protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  bool _enabled;
  string _contribution_type;
  float _weight;
  typedef pmap<string, NodePath *> LIGHTGROUP;
  LIGHTGROUP _lightgroup;
  

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

#endif



