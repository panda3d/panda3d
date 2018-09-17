/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMultiBase.cxx
 * @author drose
 * @date 2000-11-02
 */

#include "eggMultiBase.h"
#include "eggBase.h"
#include "eggData.h"
#include "eggComment.h"
#include "filename.h"
#include "dSearchPath.h"

/**
 *
 */
EggMultiBase::
EggMultiBase() {
  add_option
    ("f", "", 80,
     "Force complete loading: load up the egg file along with all of its "
     "external references.",
     &EggMultiBase::dispatch_none, &_force_complete);

  add_option
    ("noabs", "", 0,
     "Don't allow any of the named egg files to have absolute pathnames.  "
     "If any do, abort with an error.  This option is designed to help "
     "detect errors when populating or building a standalone model tree, "
     "which should be self-contained and include only relative pathnames.",
     &EggMultiBase::dispatch_none, &_noabs);
}

/**
 * Performs any processing of the egg file(s) that is appropriate before
 * writing them out.  This includes any normal adjustments the user requested
 * via -np, etc.
 *
 * Normally, you should not need to call this function directly;
 * write_egg_files() calls it for you.  You should call this only if you do
 * not use write_egg_files() to write out the resulting egg files.
 */
void EggMultiBase::
post_process_egg_files() {
  if (_eggs.empty()) {
    return;
  }

  Eggs::iterator ei;
  if (_got_transform) {
    nout << "Applying transform matrix:\n";
    _transform.write(nout, 2);
    LVecBase3d scale, hpr, translate;
    if (decompose_matrix(_transform, scale, hpr, translate,
                         _eggs[0]->get_coordinate_system())) {
      nout << "(scale " << scale << ", hpr " << hpr << ", translate "
           << translate << ")\n";
    }
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      (*ei)->transform(_transform);
    }
  }

  if (_make_points) {
    nout << "Making points\n";
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      (*ei)->make_point_primitives();
    }
  }

  switch (_normals_mode) {
  case NM_strip:
    nout << "Stripping normals.\n";
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      (*ei)->strip_normals();
      (*ei)->remove_unused_vertices(true);
    }
    break;

  case NM_polygon:
    nout << "Recomputing polygon normals.\n";
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      (*ei)->recompute_polygon_normals();
      (*ei)->remove_unused_vertices(true);
    }
    break;

  case NM_vertex:
    nout << "Recomputing vertex normals.\n";
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      (*ei)->recompute_vertex_normals(_normals_threshold);
      (*ei)->remove_unused_vertices(true);
    }
    break;

  case NM_preserve:
    // Do nothing.
    break;
  }

  if (_got_tbnall) {
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      if ((*ei)->recompute_tangent_binormal(GlobPattern("*"))) {
        (*ei)->remove_unused_vertices(true);
      }
    }
  } else {
    if (_got_tbnauto) {
      for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
        if ((*ei)->recompute_tangent_binormal_auto()) {
          (*ei)->remove_unused_vertices(true);
        }
      }
    }

    for (vector_string::const_iterator si = _tbn_names.begin();
         si != _tbn_names.end();
         ++si) {
      GlobPattern uv_name(*si);
      nout << "Computing tangent and binormal for \"" << uv_name << "\"\n";
      for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
        (*ei)->recompute_tangent_binormal(uv_name);
        (*ei)->remove_unused_vertices(true);
      }
    }
  }
}


/**
 * Allocates and returns a new EggData structure that represents the indicated
 * egg file.  If the egg file cannot be read for some reason, returns NULL.
 *
 * This can be overridden by derived classes to control how the egg files are
 * read, or to extend the information stored with each egg structure, by
 * deriving from EggData.
 */
PT(EggData) EggMultiBase::
read_egg(const Filename &filename) {
  PT(EggData) data = new EggData;

  if (!data->read(filename)) {
    // Failure reading.
    return nullptr;
  }

  if (_noabs && data->original_had_absolute_pathnames()) {
    nout << filename.get_basename()
         << " includes absolute pathnames!\n";
    return nullptr;
  }

  DSearchPath file_path;
  file_path.append_directory(filename.get_dirname());

  // We always resolve filenames first based on the source egg filename, since
  // egg files almost always store relative paths.  This is a temporary kludge
  // around integrating the path_replace system with the EggData better.
  // Update: I believe this kludge is obsolete.  Commenting out.  - Josh.
  // data->resolve_filenames(file_path);

  if (_force_complete) {
    if (!data->load_externals()) {
      return nullptr;
    }
  }

  // Now resolve the filenames again according to the user's specified
  // _path_replace.
  EggBase::convert_paths(data, _path_replace, file_path);

  if (_got_coordinate_system) {
    data->set_coordinate_system(_coordinate_system);
  } else {
    _coordinate_system = data->get_coordinate_system();
    _got_coordinate_system = true;
  }

  return data;
}
