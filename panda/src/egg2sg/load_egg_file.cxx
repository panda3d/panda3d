// Filename: load_egg_file.cxx
// Created by:  drose (09Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "load_egg_file.h"
#include "eggLoader.h"
#include "config_egg2sg.h"

#include <sceneGraphReducer.h>
#include <renderRelation.h>

static PT_NamedNode
load_from_loader(EggLoader &loader) {
  loader._data.resolve_externals();

  loader.build_graph();

  if (loader._root != (NamedNode *)NULL && egg_flatten) {
    SceneGraphReducer gr(RenderRelation::get_class_type());
    int num_reduced = gr.flatten(loader._root, egg_flatten_siblings);
    egg2sg_cat.info() << "Flattened " << num_reduced << " arcs.\n";
  }

  return loader._root;
}

////////////////////////////////////////////////////////////////////
//     Function: load_egg_file
//  Description: A convenience function.  Loads up the indicated egg
//               file, and returns the root of a scene graph.  Returns
//               NULL if the file cannot be read for some reason.
////////////////////////////////////////////////////////////////////
PT_NamedNode
load_egg_file(const string &filename, CoordinateSystem cs) {
  Filename egg_filename = Filename::text_filename(filename);
  if (!EggData::resolve_egg_filename(egg_filename)) {
    egg2sg_cat.error()
      << "Could not find " << egg_filename << "\n";
    return NULL;
  }

  egg2sg_cat.info()
    << "Reading " << egg_filename << "\n";

  ifstream file;
  if (!egg_filename.open_read(file)) {
    egg2sg_cat.error()
      << "Could not open " << egg_filename << " for reading.\n";
    return NULL;
  }

  EggLoader loader;
  loader._data.set_egg_filename(egg_filename);
  if (cs != CS_default) {
    loader._data.set_coordinate_system(cs);
  }

  if (!loader._data.read(file)) {
    egg2sg_cat.error()
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
PT_NamedNode 
load_egg_data(EggData &data) {
  // We temporarily shuttle the children to a holding node so we can
  // copy them into the EggLoader's structure without it complaining.
  EggGroupNode children_holder;
  children_holder.steal_children(data);

  EggLoader loader(data);
  loader._data.steal_children(children_holder);

  return load_from_loader(loader);
}
