// Filename: bamInfo.cxx
// Created by:  drose (02Jul00)
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

#include "bamInfo.h"

#include <bamFile.h>
#include <node.h>
#include <renderRelation.h>
#include <geomNode.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//     Function: BamInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamInfo::
BamInfo() {
  set_program_description
    ("This program scans one or more Bam files--Panda's Binary Animation "
     "and Models native binary format--and describes their contents.");

  clear_runlines();
  add_runline("[opts] input.bam [input.bam ... ]");

  add_option
    ("ls", "", 0,
     "List the scene graph hierarchy in the bam file.",
     &BamInfo::dispatch_none, &_ls);

  add_option
    ("t", "", 0,
     "List explicitly each transition in the hierarchy.",
     &BamInfo::dispatch_none, &_verbose_transitions);

  add_option
    ("g", "", 0,
     "Output verbose information about the each Geom in the Bam file.",
     &BamInfo::dispatch_none, &_verbose_geoms);

  _num_scene_graphs = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: BamInfo::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BamInfo::
run() {
  bool okflag = true;

  Filenames::const_iterator fi;
  for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
    if (!get_info(*fi)) {
      okflag = false;
    }
  }

  if (_num_scene_graphs > 0) {
    nout << "\nScene graph statistics:\n";
    _analyzer.write(nout, 2);
  }
  nout << "\n";

  if (!okflag) {
    // Exit with an error if any of the files was unreadable.
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: BamInfo::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool BamInfo::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the Bam file(s) to read on the command line.\n";
    return false;
  }

  ProgramBase::Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    _filenames.push_back(*ai);
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: BamInfo::get_info
//       Access: Private
//  Description: Reads a single Bam file and displays its contents.
//               Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool BamInfo::
get_info(const Filename &filename) {
  BamFile bam_file;

  if (!bam_file.open_read(filename)) {
    nout << "Unable to read.\n";
    return false;
  }

  nout << filename << " : Bam version " << bam_file.get_file_major_ver()
       << "." << bam_file.get_file_minor_ver() << "\n";

  typedef vector<TypedWritable *> Objects;
  Objects objects;
  TypedWritable *object = bam_file.read_object();
  while (object != (TypedWritable *)NULL || !bam_file.is_eof()) {
    if (object != (TypedWritable *)NULL) {
      objects.push_back(object);
    }
    object = bam_file.read_object();
  }
  bam_file.resolve();
  bam_file.close();

  if (objects.size() == 1 && objects[0]->is_of_type(Node::get_class_type())) {
    describe_scene_graph(DCAST(Node, objects[0]));

  } else {
    for (int i = 0; i < (int)objects.size(); i++) {
      describe_general_object(objects[i]);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: BamInfo::describe_scene_graph
//       Access: Private
//  Description: Called for Bam files that contain a single scene
//               graph and no other objects.  This should describe
//               that scene graph in some meaningful way.
////////////////////////////////////////////////////////////////////
void BamInfo::
describe_scene_graph(Node *node) {
  // Parent the node to our own scene graph root, so we can (a)
  // guarantee it won't accidentally be deleted before we're done, (b)
  // easily determine the bounding volume of the scene, and (c) report
  // statistics on all the bam file's scene graphs together when we've
  // finished.

  PT_Node root = new Node;
  NodeRelation *arc = new RenderRelation(root, node);
  _num_scene_graphs++;

  int num_nodes = _analyzer._num_nodes;
  _analyzer.add_node(node);
  num_nodes = _analyzer._num_nodes - num_nodes;

  nout << "  " << num_nodes << " nodes, bounding volume is "
       << arc->get_bound() << "\n";

  if (_ls || _verbose_geoms || _verbose_transitions) {
    list_hierarchy(node, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamInfo::describe_general_object
//       Access: Private
//  Description: Called for Bam files that contain multiple objects
//               which may or may not be scene graph nodes.  This
//               should describe each object in some meaningful way.
////////////////////////////////////////////////////////////////////
void BamInfo::
describe_general_object(TypedWritable *object) {
  nout << "  " << object->get_type() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: BamInfo::list_hierarchy
//       Access: Private
//  Description: Outputs the hierarchy and all of the verbose GeomNode
//               information.
////////////////////////////////////////////////////////////////////
void BamInfo::
list_hierarchy(Node *node, int indent_level) {
  indent(nout, indent_level) << *node << "\n";

  if (_verbose_geoms && node->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geom_node;
    DCAST_INTO_V(geom_node, node);
    geom_node->write_verbose(nout, indent_level);
  }

  int num_children = node->get_num_children(RenderRelation::get_class_type());
  for (int i = 0; i < num_children; i++) {
    int next_indent = indent_level + 2;

    NodeRelation *arc = node->get_child(RenderRelation::get_class_type(), i);
    if (_verbose_transitions) {
      nout << "\n";
      indent(nout, next_indent) << *arc << "\n";

      arc->write_transitions(nout, next_indent);
      nout << "\n";

      next_indent += 2;
    }

    list_hierarchy(arc->get_child(), next_indent);
  }
}

int main(int argc, char *argv[]) {
  BamInfo prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
