// Filename: switchNode.h
// Created by:  drose (15May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef SWITCHNODE_H
#define SWITCHNODE_H

#include <pandabase.h>

#include <namedNode.h>

class RenderTraverser;

////////////////////////////////////////////////////////////////////
//       Class : SwitchNode
// Description : A base class for nodes that may choose to render only
//               some subset of their children, based on some internal
//               criteria.
//
//               This is an abstract base class, and is only
//               interface; it is defined in this directory so that
//               FrustumCullTraverser can access it.  All of the
//               implementations of this class are defined in the
//               switchnode directory.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SwitchNode : public NamedNode {
PUBLISHED:
  INLINE SwitchNode(const string &name = "");
  INLINE SwitchNode(const SwitchNode &copy);
  INLINE void operator = (const SwitchNode &copy);

public:
  virtual void compute_switch(RenderTraverser *trav)=0;

  virtual bool is_child_visible(TypeHandle type, int index)=0;

public:
  static TypeHandle get_class_type() {
      return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type( _type_handle, "SwitchNode",
                NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle		_type_handle;
};

#include "switchNode.I"

#endif
