// Filename: immediateTransition.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMMEDIATETRANSITION_H
#define IMMEDIATETRANSITION_H

#include <pandabase.h>

#include "nodeTransition.h"

class NodeAttribute;
class NodeRelation;

////////////////////////////////////////////////////////////////////
// 	 Class : ImmediateTransition
// Description : This is a special kind of transition that doesn't
//               accumulate state, but instead has some immediate
//               effect when it is encountered in the scene graph.
//               BillboardTransition and ShaderTransition are examples
//               of these; rather than changing the state of geometry
//               below them, they cause everything to be rendered
//               differently somehow from that point and below.
//
//               The compose() etc. methods for an ImmediateTransition
//               do nothing.  Presumably any transitions that inherit
//               from ImmediateTransition will redefine sub_render()
//               to do something suitably interesting.
//
//               Classes that derive from ImmediateTransition need not
//               also define a corresponding derivation from
//               ImmediateAttribute; since the attribute does not
//               represent any actual state, you may use
//               ImmediateAttribute directly (without extending it).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ImmediateTransition : public NodeTransition {
protected:
  INLINE_GRAPH ImmediateTransition();
  INLINE_GRAPH ImmediateTransition(const ImmediateTransition &copy);
  INLINE_GRAPH void operator = (const ImmediateTransition &copy);

public:
  virtual NodeAttribute *make_attrib() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

  //ImmediateTransition is merely an interface class, so no need to
  //define the code necessary for anything relating to reading/writing

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle, "ImmediateTransition",
		  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#ifndef DONT_INLINE_GRAPH
#include "immediateTransition.I"
#endif

#endif


