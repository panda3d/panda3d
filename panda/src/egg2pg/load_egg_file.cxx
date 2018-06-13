/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_egg_file.cxx
 * @author drose
 * @date 2002-02-26
 */

#include "load_egg_file.h"
#include "eggLoader.h"
#include "config_egg2pg.h"
#include "sceneGraphReducer.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "bamCacheRecord.h"

static PT(PandaNode)
load_from_loader(EggLoader &loader) {
  loader._data->load_externals(DSearchPath(), loader._record);

  loader.build_graph();

  if (loader._error && !egg_accept_errors) {
    egg2pg_cat.error()
      << "Errors in egg file.\n";
    return nullptr;
  }

  if (loader._root != nullptr && egg_flatten) {
    SceneGraphReducer gr;

    int combine_siblings_bits = 0;
    if (egg_combine_geoms) {
      combine_siblings_bits |= SceneGraphReducer::CS_geom_node;
    }
    if (egg_flatten_radius > 0.0) {
      combine_siblings_bits |= SceneGraphReducer::CS_within_radius;
      gr.set_combine_radius(egg_flatten_radius);
    }

    int num_reduced = gr.flatten(loader._root, combine_siblings_bits);
    egg2pg_cat.info() << "Flattened " << num_reduced << " nodes.\n";

    if (egg_unify) {
      // We want to premunge before unifying, since otherwise we risk
      // needlessly duplicating vertices.
      if (premunge_data) {
        gr.premunge(loader._root, RenderState::make_empty());
      }
      gr.collect_vertex_data(loader._root);
      gr.unify(loader._root, true);
      if (egg2pg_cat.is_debug()) {
        egg2pg_cat.debug() << "Unified.\n";
      }
    }
  }

  return loader._root;
}

/**
 * A convenience function.  Loads up the indicated egg file, and returns the
 * root of a scene graph.  Returns NULL if the file cannot be read for some
 * reason.  Does not search along the egg path for the filename first; use
 * EggData::resolve_egg_filename() if this is required.
 */
PT(PandaNode)
load_egg_file(const Filename &filename, CoordinateSystem cs,
              BamCacheRecord *record) {
  Filename egg_filename = filename;
  egg_filename.set_text();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (record != nullptr) {
    record->add_dependent_file(egg_filename);
  }

  EggLoader loader;
  loader._data->set_egg_filename(egg_filename);
  loader._data->set_auto_resolve_externals(true);
  loader._data->set_coordinate_system(cs);
  loader._record = record;

  PT(VirtualFile) vfile = vfs->get_file(egg_filename);
  if (vfile == nullptr) {
    return nullptr;
  }

  loader._data->set_egg_timestamp(vfile->get_timestamp());

  bool okflag;
  std::istream *istr = vfile->open_read_file(true);
  if (istr == nullptr) {
    egg2pg_cat.error()
      << "Couldn't read " << egg_filename << "\n";
    return nullptr;
  }

  egg2pg_cat.info()
    << "Reading " << egg_filename << "\n";

  okflag = loader._data->read(*istr);
  vfile->close_read_file(istr);

  if (!okflag) {
    egg2pg_cat.error()
      << "Error reading " << egg_filename << "\n";
    return nullptr;
  }

  return load_from_loader(loader);
}

/**
 * Another convenience function; works like load_egg_file() but starts from an
 * already-filled EggData structure.  The structure is destroyed in the
 * loading.
 */
PT(PandaNode)
load_egg_data(EggData *data, CoordinateSystem cs) {
  // We temporarily shuttle the children to a holding node so we can copy them
  // into the EggLoader's structure without it complaining.
  EggGroupNode children_holder;
  children_holder.steal_children(*data);

  EggLoader loader(data);
  loader._data->steal_children(children_holder);

  loader._data->set_auto_resolve_externals(true);
  loader._data->set_coordinate_system(cs);

  return load_from_loader(loader);
}
