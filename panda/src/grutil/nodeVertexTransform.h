// Filename: nodeVertexTransform.h
// Created by:  drose (22eb07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef NODEVERTEXTRANSFORM_H
#define NODEVERTEXTRANSFORM_H

#include "pandabase.h"
#include "pandaNode.h"
#include "vertexTransform.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : NodeVertexTransform
// Description : This VertexTransform gets its matrix from the
//               Transform stored on a node.  It can also compose its
//               node's transform with another VertexTransform,
//               allowing you to build up a chain of
//               NodeVertexTransforms that represent a list of
//               composed matrices.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL NodeVertexTransform : public VertexTransform {
PUBLISHED:
  NodeVertexTransform(const PandaNode *node, 
                      const VertexTransform *prev = NULL);

  INLINE const PandaNode *get_node() const;
  INLINE const VertexTransform *get_prev() const;

  virtual void get_matrix(LMatrix4 &matrix) const;

  virtual void output(ostream &out) const;

private:
  CPT(PandaNode) _node;
  CPT(VertexTransform) _prev;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexTransform::init_type();
    register_type(_type_handle, "NodeVertexTransform",
                  VertexTransform::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class RigidBodyCombiner;
};

#include "nodeVertexTransform.I"

#endif
