// Filename: bamInfo.cxx
// Created by:  drose (19Jun01)
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

#include "bam2x.h"

#include <bamFile.h>
#include <node.h>
#include <renderRelation.h>
#include <geomNode.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//     Function: Bam2X::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Bam2X::
Bam2X() : WithOutputFile(true, false, true) {
  set_program_description
    ("This program reads a Bam file and outputs an equivalent, "
     "or nearly equivalent, DirectX-style .x file.  Meshes as generated "
     "by the Panda mesher are preserved, but advanced features like "
     "LOD's and characters cannot be supported.");

  clear_runlines();
  add_runline("[opts] input.bam output.x");
  add_runline("[opts] -o output.x input.bam");

  add_option
    ("o", "filename", 50, 
     "Specify the filename to which the resulting .x file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &Bam2X::dispatch_filename, &_got_output_filename, &_output_filename);
}


////////////////////////////////////////////////////////////////////
//     Function: Bam2X::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Bam2X::
run() {
  BamFile bam_file;

  if (!bam_file.open_read(_input_filename)) {
    nout << "Unable to read.\n";
    exit(1);
  }

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
    convert_scene_graph(DCAST(Node, objects[0]));
  } else {
    nout << _input_filename << " does not contain a scene graph.\n";
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Bam2X::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool Bam2X::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  if (args.size() != 1) {
    nout << "You must specify one Bam file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];
  return true;
}

int main(int argc, char *argv[]) {
  Bam2X prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Bam2X::convert_scene_graph
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Bam2X::
convert_scene_graph(Node *root) {
  nout << "Got root: " << *root << "\n";

  if (!_x.open(get_output_filename())) {
    nout << "Unable to open " << get_output_filename() << " for output.\n";
    exit(1);
  }
  if (!_x.add_node(root)) {
    nout << "Unable to add " << *root << " to output.\n";
    exit(1);
  }
  _x.close();
}
