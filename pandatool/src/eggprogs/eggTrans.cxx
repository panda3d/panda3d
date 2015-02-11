// Filename: eggTrans.cxx
// Created by:  drose (14Feb00)
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

#include "eggTrans.h"
#include "eggGroupUniquifier.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: EggTrans::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTrans::
EggTrans() {
  add_path_replace_options();
  add_path_store_options();
  add_normals_options();
  add_transform_options();
  add_texture_options();
  add_delod_options();

  set_program_brief("apply transformations and optimizations to an .egg file");
  set_program_description
    ("egg-trans reads an egg file and writes an essentially equivalent "
     "egg file to the standard output, or to the file specified with -o.  "
     "Some simple operations on the egg file are supported.");

  add_option
    ("F", "", 0,
     "Flatten out transforms.",
     &EggTrans::dispatch_none, &_flatten_transforms);

  add_option
    ("t", "", 0,
     "Apply texture matrices to UV's.",
     &EggTrans::dispatch_none, &_apply_texmats);

  add_option
    ("T", "", 0,
     "Collapse equivalent texture references.",
     &EggTrans::dispatch_none, &_collapse_equivalent_textures);

  add_option
    ("c", "", 0,
     "Clean out degenerate polygons and unused vertices.",
     &EggTrans::dispatch_none, &_remove_invalid_primitives);

  add_option
    ("C", "", 0,
     "Clean out higher-order polygons by subdividing into triangles.",
     &EggTrans::dispatch_none, &_triangulate_polygons);

  add_option
    ("mesh", "", 0,
     "Mesh triangles into triangle strips.  This is mainly useful as a "
     "tool to visualize the work that the mesher will do, since triangles "
     "are automatically meshed whenever an egg file is loaded.  Note that, "
     "unlike the automatic meshing at load time, you are must ensure that "
     "you do not start out with multiple triangles with different attributes "
     "(e.g. texture) together in the same group.",
     &EggTrans::dispatch_none, &_mesh_triangles);

  add_option
    ("N", "", 0,
     "Standardize and uniquify group names.",
     &EggTrans::dispatch_none, &_standardize_names);

}

////////////////////////////////////////////////////////////////////
//     Function: EggTrans::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggTrans::
run() {
  if (_remove_invalid_primitives) {
    nout << "Removing invalid primitives.\n";
    int num_removed = _data->remove_invalid_primitives(true);
    nout << "  (" << num_removed << " removed.)\n";
    _data->remove_unused_vertices(true);
  }

  if (_triangulate_polygons) {
    nout << "Triangulating polygons.\n";
    int num_produced = _data->triangulate_polygons(~0);
    nout << "  (" << num_produced << " triangles produced.)\n";
  }

  if (_mesh_triangles) {
    nout << "Meshing triangles.\n";
    _data->mesh_triangles(~0);
  }

  if (_apply_texmats) {
    nout << "Applying texture matrices.\n";
    _data->apply_texmats();
    _data->remove_unused_vertices(true);
  }

  if (_collapse_equivalent_textures) {
    nout << "Collapsing equivalent textures.\n";
    int num_removed = _data->collapse_equivalent_textures();
    nout << "  (" << num_removed << " removed.)\n";
  }

  if (_flatten_transforms) {
    nout << "Flattening transforms.\n";
    _data->flatten_transforms();
    _data->remove_unused_vertices(true);
  }

  if (_standardize_names) {
    nout << "Standardizing group names.\n";
    EggGroupUniquifier uniquifier;
    uniquifier.uniquify(_data);
  }

  if (!do_reader_options()) {
    exit(1);
  }

  write_egg_file();
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  EggTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
