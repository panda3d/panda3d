// Filename: load_egg_file.cxx
// Created by:  drose (26Feb02)
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

#include "load_egg_file.h"
#include "eggLoader.h"
#include "config_egg2pg.h"
#include "sceneGraphReducer.h"
#include "virtualFileSystem.h"
#include "config_util.h"
#include "config_express.h"

static PT(PandaNode)
load_from_loader(EggLoader &loader) {
  loader._data.load_externals();

  loader.build_graph();

  if (loader._error && !egg_accept_errors) {
    egg2pg_cat.error()
      << "Errors in egg file.\n";
    return NULL;
  }

  if (loader._root != (PandaNode *)NULL && egg_flatten) {
    SceneGraphReducer gr;
    int num_reduced = gr.flatten(loader._root, egg_flatten_siblings);
    egg2pg_cat.info() << "Flattened " << num_reduced << " nodes.\n";
  }

  return loader._root;
}

////////////////////////////////////////////////////////////////////
//     Function: load_egg_file
//  Description: A convenience function.  Loads up the indicated egg
//               file, and returns the root of a scene graph.  Returns
//               NULL if the file cannot be read for some reason.
//               Does not search along the egg path for the filename
//               first; use EggData::resolve_egg_filename() if this is
//               required.
////////////////////////////////////////////////////////////////////
PT(PandaNode)
load_egg_file(const string &filename, CoordinateSystem cs) {
  Filename egg_filename = Filename::text_filename(filename);
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    if (!vfs->exists(egg_filename)) {
      egg2pg_cat.error()
        << "Could not find " << egg_filename << "\n";
      return NULL;
    }

  } else {
    if (!egg_filename.exists()) {
      egg2pg_cat.error()
        << "Could not find " << egg_filename << "\n";
      return NULL;
    }
  }

  egg2pg_cat.info()
    << "Reading " << egg_filename << "\n";


  EggLoader loader;
  loader._data.set_egg_filename(egg_filename);
  loader._data.set_auto_resolve_externals(true);
  if (cs != CS_default) {
    loader._data.set_coordinate_system(cs);
  }
  bool okflag;
  
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    istream *istr = vfs->open_read_file(egg_filename);
    if (istr == (istream *)NULL) {
      egg2pg_cat.error()
        << "Could not open " << egg_filename << " for reading.\n";
      return NULL;
    }
    okflag = loader._data.read(*istr);
    delete istr;

  } else {
    ifstream file;

    if (!egg_filename.open_read(file)) {
      egg2pg_cat.error()
        << "Could not open " << egg_filename << " for reading.\n";
      return NULL;
    } 
    okflag = loader._data.read(file);
  }

  if (!okflag) {
    egg2pg_cat.error()
      << "Error reading " << egg_filename << "\n";
    return NULL;
  }

  return load_from_loader(loader);
}

////////////////////////////////////////////////////////////////////
//     Function: load_egg_data
//  Description: Another convenience function; works like
//               load_egg_file() but starts from an already-filled
//               EggData structure.  The structure is destroyed in the
//               loading.
////////////////////////////////////////////////////////////////////
PT(PandaNode)
load_egg_data(EggData &data) {
  // We temporarily shuttle the children to a holding node so we can
  // copy them into the EggLoader's structure without it complaining.
  EggGroupNode children_holder;
  children_holder.steal_children(data);

  EggLoader loader(data);
  loader._data.steal_children(children_holder);

  return load_from_loader(loader);
}
