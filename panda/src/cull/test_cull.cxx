// Filename: test_cull.cxx
// Created by:  drose (07Apr00)
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

#include "cullTraverser.h"
#include "directRenderTransition.h"

#include <colorTransition.h>
#include <textureTransition.h>
#include <billboardTransition.h>
#include <namedNode.h>
#include <pt_NamedNode.h>
#include <geomNode.h>
#include <dftraverser.h>
#include <traverserVisitor.h>
#include <allTransitionsWrapper.h>
#include <pointerTo.h>
#include <nullLevelState.h>


class PrintNodes : public TraverserVisitor<AllTransitionsWrapper, NullLevelState> {
public:
  PrintNodes() {
    _indent_level = 0;
  }
  bool reached_node(Node *node, TransitionWrapper &state, NullLevelState &) {
    indent(nout, _indent_level)
      << *node << ", state is:\n";
    state.write(nout, _indent_level);
    return true;
  }
  bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                   TransitionWrapper &pre, TransitionWrapper &post,
                   NullLevelState &) {
    _indent_level += 2;
    return true;
  }
  void backward_arc(NodeRelation *arc, TransitionWrapper &trans,
                    TransitionWrapper &pre, TransitionWrapper &post,
                    const NullLevelState &) {
    _indent_level -= 2;
  }
  int _indent_level;
};


int
main(int argc, char *argv[]) {
  PT_NamedNode r = new NamedNode("r");

  PT_NamedNode a = new NamedNode("a");
  PT_NamedNode b = new NamedNode("b");

  PT(GeomNode) aa = new GeomNode("aa");
  PT(GeomNode) aaa = new GeomNode("aaa");
  PT(GeomNode) ab = new GeomNode("ab");
  PT(GeomNode) ba = new GeomNode("ba");
  PT(GeomNode) bb = new GeomNode("bb");

  PT_NamedNode m = new GeomNode("m");
  PT(GeomNode) ma = new GeomNode("ma");
  PT(GeomNode) mb = new GeomNode("mb");

  NodeRelation *r_a =
    new NodeRelation(r, a, 0, NodeRelation::get_class_type());
  NodeRelation *r_b =
    new NodeRelation(r, b, 0, NodeRelation::get_class_type());

  NodeRelation *a_aa =
    new NodeRelation(a, aa, 0, NodeRelation::get_class_type());
  NodeRelation *a_ab =
    new NodeRelation(a, ab, 0, NodeRelation::get_class_type());
  NodeRelation *b_ba =
    new NodeRelation(b, ba, 0, NodeRelation::get_class_type());
  NodeRelation *b_bb =
    new NodeRelation(b, bb, 0, NodeRelation::get_class_type());

  NodeRelation *aa_aaa =
    new NodeRelation(aa, aaa, 0, NodeRelation::get_class_type());

  NodeRelation *a_m =
    new NodeRelation(a, m, 0, NodeRelation::get_class_type());

  NodeRelation *b_m =
    new NodeRelation(b, m, 0, NodeRelation::get_class_type());

  NodeRelation *m_ma =
    new NodeRelation(m, ma, 0, NodeRelation::get_class_type());
  NodeRelation *m_mb =
    new NodeRelation(m, mb, 0, NodeRelation::get_class_type());


  PT(Texture) alpha = new Texture;
  alpha->set_name("alpha");
  PT(Texture) beta = new Texture;
  beta->set_name("beta");

  r_a->set_transition(new ColorTransition(1.0, 1.0, 1.0, 1.0));
  r_a->set_transition(new TextureTransition(alpha));
  a_aa->set_transition(new ColorTransition(0.5, 1.0, 0.5, 0.2));
  a_ab->set_transition(new TextureTransition(beta));

  a_m->set_transition(new BillboardTransition);
  b_m->set_transition(new ColorTransition(0.0, 0.0, 1.0, 1.0));
  m_ma->set_transition(new TextureTransition(alpha));

  //  r_a->set_transition(new DirectRenderTransition);

  nout << "\nr to a has:\n";
  r_a->write_transitions(nout, 2);
  nout << "\nr to b has:\n";
  r_b->write_transitions(nout, 2);
  nout << "\na to aa has:\n";
  a_aa->write_transitions(nout, 2);
  nout << "\na to ab has:\n";
  a_ab->write_transitions(nout, 2);
  nout << "\nb to ba has:\n";
  b_ba->write_transitions(nout, 2);
  nout << "\n";

  nout << "\n";
  PrintNodes pn;
  df_traverse(r, pn,
              AllTransitionsWrapper(), NullLevelState(),
              NodeRelation::get_class_type());
  nout << "\n";

  CullTraverser ct(NULL, NodeRelation::get_class_type());

  ct.traverse(r, AllTransitionsWrapper(), AllTransitionsWrapper());
  ct.write(nout, 0);

  nout << "\nframe 2:\n";
  ct.traverse(r, AllTransitionsWrapper(), AllTransitionsWrapper());
  ct.write(nout, 0);

  return (0);
}

