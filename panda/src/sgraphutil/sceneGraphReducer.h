// Filename: sceneGraphReducer.h
// Created by:  drose (22May00)
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

#ifndef SCENEGRAPHREDUCER_H
#define SCENEGRAPHREDUCER_H

#include "pandabase.h"

#include "graphReducer.h"
#include "typedObject.h"
#include "pointerTo.h"
#include "transformTransition.h"
#include "colorTransition.h"
#include "texMatrixTransition.h"
#include "colorMatrixTransition.h"
#include "alphaTransformTransition.h"
#include "geomTransformer.h"
#include "renderRelation.h"

class Geom;

///////////////////////////////////////////////////////////////////
//       Class : SceneGraphReducer
// Description : A specialization on GraphReducer for reducing scene
//               graphs.  This GraphReducer knows about special kinds
//               of nodes like GeomNodes and can combine them
//               appropriately.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneGraphReducer : public GraphReducer {
public:
  SceneGraphReducer(TypeHandle graph_type = RenderRelation::get_class_type());

  enum TransitionTypes {
    TT_transform       = 0x001,
    TT_color           = 0x002,
    TT_texture_matrix  = 0x004,
    TT_color_matrix    = 0x008,
  };

  void apply_transitions(NodeRelation *arc, int transition_types = ~0);

protected:
  class AccumulatedTransitions {
  public:
    INLINE AccumulatedTransitions();
    INLINE AccumulatedTransitions(const AccumulatedTransitions &copy);
    INLINE void operator = (const AccumulatedTransitions &copy);

    void write(ostream &out, int transition_types, int indent_level) const;
    void apply_to_arc(NodeRelation *arc, int transition_types);

    PT(TransformTransition) _transform;
    PT(ColorTransition) _color;
    PT(TexMatrixTransition) _texture_matrix;
    PT(ColorMatrixTransition) _color_matrix;
    PT(AlphaTransformTransition) _alpha_transform;
  };

  void r_apply_transitions(NodeRelation *arc, int transition_types,
                           AccumulatedTransitions trans,
                           bool duplicate);

  virtual bool consider_siblings(Node *parent,
                                 NodeRelation *arc1, NodeRelation *arc2);

  virtual Node *collapse_nodes(Node *node1, Node *node2, bool siblings);

private:
  GeomTransformer _transformer;
};

#include "sceneGraphReducer.I"

#endif
