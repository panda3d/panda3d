// Filename: test_eggWrite.cxx
// Created by:  jason (16Jun00)
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

#include <renderRelation.h>
#include <transformTransition.h>
#include <namedNode.h>
#include <geom.h>
#include <geomNode.h>
#include <pt_NamedNode.h>
#include <nodeTransitionWrapper.h>
#include <nodeAttributeWrapper.h>
#include <allTransitionsWrapper.h>
#include <allAttributesWrapper.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>
#include <traverserVisitor.h>
#include <dftraverser.h>
#include <nullLevelState.h>
#include "filename.h"
#include <character.h>

#include <indent.h>
#include <ipc_file.h>
#include <bamWriter.h>
#include <bam.h>


template<class TW>
class PrintNodes : public TraverserVisitor<TW, NullLevelState> {
public:
  PrintNodes() {
    _indent_level = 0;
  }
  bool reached_node(Node *node, AttributeWrapper &state, NullLevelState &)
  {
    indent(nout, _indent_level) << "Reached " << *node << ", state is " << state << "\n";
    //  if (node->is_of_type(GeomNode::get_class_type()))
//      {
//        GeomNode *geomNode = (GeomNode *)node;
//        int num_geoms = geomNode->get_num_geoms();
//        for (int i = 0; i < num_geoms; i++)
//        {
//      dDrawable *draw = geomNode->get_geom(i);
//      Geom *geom = DCAST(Geom, draw);
//      indent(nout, _indent_level+1) << *geom << ":\n";
//      geom->output_verbose(nout);
//        }
//      }
    return true;
  }
  bool forward_arc(NodeRelation *, TransitionWrapper &, AttributeWrapper &, AttributeWrapper &, NullLevelState &)
  {
    _indent_level += 2;
    return true;
  }
  void backward_arc(NodeRelation *, TransitionWrapper &, AttributeWrapper &, AttributeWrapper &, const NullLevelState &)
  {
    _indent_level -= 2;
  }
  int _indent_level;
};



int main(int argc, char* argv[]) {
  if (argc < 2)
  {
    nout << "Need an egg file" << endl;
    exit(0);
  }
  Filename file(argv[1]);
  if (file.get_extension().compare("egg") != 0)
  {
    nout << "Need an egg file" << endl;
    exit(0);
  }
  datagram_file stream(file.get_basename_wo_extension()+string(".bam"));
  BamWriter manager(&stream);

  PT_NamedNode smiley = loader.load_sync(file);

  nout << "\n";
  PrintNodes<NodeTransitionWrapper> pn;
  df_traverse(smiley, pn,
              NodeAttributeWrapper(TransformTransition::get_class_type()),
              NullLevelState(),
              RenderRelation::get_class_type());
  nout << "\n";

  stream.open(file::FILE_WRITE, _bam_header);
  manager.init();
  manager.write_object(smiley);
  stream.close();

  return 0;
}

