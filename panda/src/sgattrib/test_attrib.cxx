// Filename: test_attrib.cxx
// Created by:  drose (18Jan99)
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

#include "colorTransition.h"
#include "textureTransition.h"

#include <namedNode.h>
#include <pt_NamedNode.h>
#include <dftraverser.h>
#include <traverserVisitor.h>
#include <wrt.h>
#include <allTransitionsWrapper.h>
#include <nullLevelState.h>


class PrintNodes :
  public TraverserVisitor<AllTransitionsWrapper, NullLevelState> {
public:
  PrintNodes() {
    _indent_level = 0;
  }
  bool reached_node(Node *node, TransitionWrapper &state,
                    NullLevelState &) {
    indent(cerr, _indent_level)
      << "\nReached " << *node << ", state is:\n";
    state.write(cerr, _indent_level);
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

  PT_NamedNode aa = new NamedNode("aa");
  PT_NamedNode ab = new NamedNode("ab");
  PT_NamedNode ba = new NamedNode("ba");

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

  PT(Texture) alpha = new Texture;
  alpha->set_name("alpha");
  PT(Texture) beta = new Texture;
  beta->set_name("beta");

  r_a->set_transition(new ColorTransition(1.0f, 1.0f, 1.0f, 1.0f));
  r_a->set_transition(new TextureTransition(alpha));
  a_aa->set_transition(new ColorTransition(0.5f, 1.0f, 0.5f, 0.2f));
  a_ab->set_transition(new TextureTransition(beta));

  cerr << "\nr to a has:\n";
  r_a->write_transitions(cerr, 2);
  cerr << "\nr to b has:\n";
  r_b->write_transitions(cerr, 2);
  cerr << "\na to aa has:\n";
  a_aa->write_transitions(cerr, 2);
  cerr << "\na to ab has:\n";
  a_ab->write_transitions(cerr, 2);
  cerr << "\nb to ba has:\n";
  b_ba->write_transitions(cerr, 2);
  cerr << "\n";

  {
    cerr << "\n";
    PrintNodes pn;
    df_traverse(r, pn,
                AllTransitionsWrapper(),
                NullLevelState(),
                NodeRelation::get_class_type());
    cerr << "\n";

    AllTransitionsWrapper result;
    wrt(r, aa, result, NodeRelation::get_class_type());
    cerr << "\nwrt of r to aa is:\n";
    result.write(cerr, 2);

    wrt(aa, r, result, NodeRelation::get_class_type());
    cerr << "\nwrt of aa to r is:\n";
    result.write(cerr, 2);

    wrt(r, ab, result, NodeRelation::get_class_type());
    cerr << "\nwrt of r to ab is:\n";
    result.write(cerr, 2);

    wrt(ab, r, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ab to r is:\n";
    result.write(cerr, 2);

    wrt(ab, aa, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ab to aa is:\n";
    result.write(cerr, 2);

    wrt(aa, ab, result, NodeRelation::get_class_type());
    cerr << "\nwrt of aa to ab is:\n";
    result.write(cerr, 2);

    wrt(r, ba, result, NodeRelation::get_class_type());
    cerr << "\nwrt of r to ba is:\n";
    result.write(cerr, 2);

    wrt(ba, r, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ba to r is:\n";
    result.write(cerr, 2);

    wrt(aa, ba, result, NodeRelation::get_class_type());
    cerr << "\nwrt of aa to ba is:\n";
    result.write(cerr, 2);

    wrt(ba, aa, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ba to aa is:\n";
    result.write(cerr, 2);

    wrt(ab, ba, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ab to ba is:\n";
    result.write(cerr, 2);

    wrt(ba, ab, result, NodeRelation::get_class_type());
    cerr << "\nwrt of ba to ab is:\n";
    result.write(cerr, 2);
  }

  return (0);
}

