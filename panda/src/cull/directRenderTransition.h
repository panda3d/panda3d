// Filename: directRenderTransition.h
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DIRECTRENDERTRANSITION_H
#define DIRECTRENDERTRANSITION_H

#include <pandabase.h>

#include <immediateTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : DirectRenderTransition
// Description : When this transition is attached to an arc, it
//               indicates that the node below this arc, and all
//               subsequent nodes below, should be treated as a single
//               binnable unit.  All the nodes at this point and below
//               will be placed into the same bin, according to the
//               state as of the top node, and when rendered, they
//               will be rendered using a depth-first traversal rooted
//               at this top node.
//
//               This can be a way to establish a local draw-order
//               within a few related nodes.  It does not, however,
//               affect view-frustum culling.
//
//               This is the only way to achieve certain special
//               effects that depend on scene-graph relationships, for
//               instance decalling, while using the CullTraverser.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectRenderTransition : public ImmediateTransition {
PUBLISHED:
  INLINE DirectRenderTransition();

public:  
  virtual NodeTransition *make_copy() const;

  virtual bool has_sub_render() const;

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
    register_type(_type_handle, "DirectRenderTransition",
		  ImmediateTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class DirectRenderAttribute;
};

#include "directRenderTransition.I"

#endif
