// Filename: shaderTransition.h
// Created by:  mike (19Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef SHADERTRANSITION_H
#define SHADERTRANSITION_H

#include "pandabase.h"

#include "shader.h"

#include <immediateTransition.h>
#include "pointerTo.h"
#include <nodeRelation.h>

#include "plist.h"
#include "pmap.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderTransition
// Description : A ShaderTransition holds the set of shaders that
//               might be in effect to render a particular part of the
//               subgraph.  These shaders can range from lightweight
//               rendering effects to full-blown multipass renderings;
//               use with caution.
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER ShaderTransition : public ImmediateTransition {
private:
  //  typedef pvector< PT(Shader) > Shaders;
  typedef plist< PT(Shader) > Shaders;
  typedef pmap<TypeHandle, int> ShaderOrder;
  typedef pmap<PT(Shader), int> ShaderOverride;
  typedef pset<TypeHandle> ShaderBlend;

public:
  INLINE ShaderTransition();

public:
  // Functions to access and adjust the list of shaders.
  typedef Shaders::const_iterator iterator;
  typedef Shaders::const_iterator const_iterator;

  void clear();
  bool is_empty() const;
  void insert(Shader *shader);

  //If you wish to override the ordering of your shader.  Say for some
  //reason you don't want this Shadow to appear in the reflections, or you want
  //Spheremaped objects to reflect the reflections in planar objects instead of
  //the other way around.   Then set override appropriately.
  bool set_shader(Shader *shader, int override = -1);
  bool clear_shader(Shader *shader);
  bool has_shader(Shader *shader) const;

  const_iterator begin() const;
  const_iterator end() const;

public:
  static void set_shader_order(TypeHandle shader, int order);
  static void set_shader_always_blend(TypeHandle shader);

private:
  bool must_blend();

public:
  virtual NodeTransition *make_copy() const;

  virtual bool sub_render(NodeRelation *arc,
                          const AllTransitionsWrapper &input_trans,
                          AllTransitionsWrapper &modify_trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

private:
  Shaders _shaders;
  static ShaderOrder* _shader_order;
  static ShaderBlend* _shader_always_blend;
  ShaderOverride _overrides;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImmediateTransition::init_type();
    register_type(_type_handle, "ShaderTransition",
                  ImmediateTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ShaderAttribute;
};

INLINE bool set_shader(NodeRelation *arc, Shader *shader);
INLINE bool clear_shader(NodeRelation *arc, Shader *shader);
INLINE bool has_shader(const NodeRelation *arc, Shader *shader);

#include "shaderTransition.I"

#endif


