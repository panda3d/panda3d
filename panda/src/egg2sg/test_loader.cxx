// Filename: test_loader.cxx
// Created by:  drose (21Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "load_egg_file.h"

#include <animControl.h>
#include <auto_bind.h>
#include <builder.h>
#include <character.h>
#include <dftraverser.h>
#include <eggData.h>
#include <geom.h>
#include <geomNode.h>
#include <indent.h>
#include <lmatrix.h>
#include <namedNode.h>
#include <partBundle.h>
#include <pointerTo.h>
#include <renderRelation.h>
#include <traverserVisitor.h>
#include <allTransitionsWrapper.h>
#include <allAttributesWrapper.h>
#include <nullLevelState.h>
#include <typedObject.h>
#include <notify.h>

#include <stdlib.h>

class PrintNodes : 
  public TraverserVisitor<AllTransitionsWrapper, NullLevelState> {
public:
  PrintNodes() {
    _indent_level = 0;
  }
  bool reached_node(Node *node, AttributeWrapper &state, NullLevelState &) {
    if (node->is_of_type(GeomNode::get_class_type())) {
      GeomNode *geomNode = (GeomNode *)node;
      indent(nout, _indent_level) 
        << *geomNode << ", " << geomNode->get_num_geoms()
        << " geoms:\n";
      int num_geoms = geomNode->get_num_geoms();
      for (int i = 0; i < num_geoms; i++) {
        dDrawable *draw = geomNode->get_geom(i);
        if (draw->is_of_type(Geom::get_class_type())) {
          Geom *geom = DCAST(Geom, draw);
          //      geom->output_verbose(nout);
          indent(nout, _indent_level + 2) << *geom << "\n";
        } else {
          indent(nout, _indent_level + 2) << draw->get_type() << "\n";
        }
      }
    }
    return true;
  }
  bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                   AttributeWrapper &pre, AttributeWrapper &post,
                   NullLevelState &) {
    indent(nout, _indent_level) << *arc << " has:\n";
    trans.write(nout, _indent_level + 4);
    _indent_level += 2;
    return true;
  }
  void backward_arc(NodeRelation *arc, TransitionWrapper &trans,
                    AttributeWrapper &pre, AttributeWrapper &post,
                    const NullLevelState &) {
    _indent_level -= 2;
  }
  int _indent_level;
};

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    nout << "Specify an egg file to load.\n";
    exit(1);
  }

  PT_NamedNode root = new NamedNode("root");

  for (int i = 1; i < argc; i++) {
    const char *egg_filename = argv[i];

    PT_NamedNode egg_root = load_egg_file(egg_filename);
    if (egg_root == NULL) {
      nout << "Unable to load " << egg_filename << ".\n";
      exit(1);
    }

    new RenderRelation(root, egg_root);
  }
  
  PrintNodes pn;
  df_traverse(root, pn, AllAttributesWrapper(), NullLevelState(),
              RenderRelation::get_class_type());
  
  return(0);
}
