// Filename: mayaCopy.cxx
// Created by:  drose (10May02)
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

#include "mayaCopy.h"
#include "config_maya.h"
#include "cvsSourceDirectory.h"
#include "mayaShader.h"
#include "dcast.h"

#include "pre_maya_include.h"
#include <maya/MStringArray.h>
#include <maya/MFileIO.h>
#include <maya/MItDag.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnMesh.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaCopy::
MayaCopy() {
  set_program_description
    ("mayacopy copies one or more Maya .mb files into a "
     "CVS source hierarchy.  "
     "Rather than copying the named files immediately into the current "
     "directory, it first scans the entire source hierarchy, identifying all "
     "the already-existing files.  If the named file to copy matches the "
     "name of an already-existing file in the current directory or elsewhere "
     "in the hierarchy, that file is overwritten.  Other .mb files, as "
     "well as texture files, that are externally referenced by the "
     "named .mb file(s) are similarly copied.");

  clear_runlines();
  add_runline("[opts] file.mb [file.mb ... ]");

  add_option
    ("keepver", "", 0,
     "Don't attempt to strip the Maya version number from the tail of the "
     "source filename before it is copied into the tree.",
     &CVSCopy::dispatch_none, &_keep_ver);

  add_path_replace_options();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MayaCopy::
run() {
  _maya = MayaApi::open_api(_program_name);
  if (!_maya->is_valid()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  SourceFiles::iterator fi;
  for (fi = _source_files.begin(); fi != _source_files.end(); ++fi) {
    ExtraData ed;
    ed._type = FT_maya;

    CVSSourceDirectory *dest = import(*fi, &ed, _model_dir);
    if (dest == (CVSSourceDirectory *)NULL) {
      exit(1);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_file
//       Access: Protected, Virtual
//  Description: Called by import() if verify_file() indicates that a
//               file needs to be copied.  This does the actual copy
//               of a file from source to destination.  If new_file is
//               true, then dest does not already exist.
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_file(const Filename &source, const Filename &dest,
          CVSSourceDirectory *dir, void *extra_data, bool new_file) {
  ExtraData *ed = (ExtraData *)extra_data;
  switch (ed->_type) {
  case FT_maya:
    return copy_maya_file(source, dest, dir);

  case FT_texture:
    return copy_texture(source, dest, dir);
  }

  nout << "Internal error: invalid type " << (int)ed->_type << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::filter_filename
//       Access: Protected, Virtual
//  Description: Given a source filename (including the basename only,
//               without a dirname), return the appropriate
//               corresponding filename within the source directory.
//               This may be used by derived classes to, for instance,
//               strip a version number from the filename.
////////////////////////////////////////////////////////////////////
string MayaCopy::
filter_filename(const string &source) {
  if (_keep_ver) {
    return source;
  }

  size_t dot = source.rfind('.');
  size_t underscore = source.rfind("_v", dot);

  if (underscore == string::npos) {
    // No version number appears to be present.
    return source;
  }

  return source.substr(0, underscore) + source.substr(dot);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_maya_file
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_maya_file(const Filename &source, const Filename &dest,
               CVSSourceDirectory *dir) {
  if (!_maya->read(source)) {
    maya_cat.error()
      << "Unable to read " << source << "\n";
    return false;
  }

  // Get all the shaders so we can determine the set of textures.
  _shaders.clear();
  collect_shaders();
  int num_shaders = _shaders.get_num_shaders();
  for (int i = 0; i < num_shaders; i++) {
    MayaShader *shader = _shaders.get_shader(i);
    if (shader->_has_texture) {
      Filename texture_filename = 
        _path_replace->convert_path(shader->_texture);
      if (!texture_filename.exists()) {
        nout << "*** Warning: texture " << texture_filename
             << " does not exist.\n";
      } else {
        ExtraData ed;
        ed._type = FT_texture;

        CVSSourceDirectory *texture_dir =
          import(texture_filename, &ed, _map_dir);
        if (texture_dir == (CVSSourceDirectory *)NULL) {
          return false;
        }

        // Update the texture reference to point to the new texture
        // filename, relative to the flt file.  Not sure how to do
        // this right now.
        Filename new_filename = dir->get_rel_to(texture_dir) + "/" +
          texture_filename.get_basename();
        shader->reset_maya_texture(new_filename);
      }
    }
  }

  // Get the set of externally referenced Maya files.
  MStringArray refs;
  MStatus status = MFileIO::getReferences(refs);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // Now write out the Maya file.
  if (!_maya->write(dest)) {
    maya_cat.error()
      << "Cannot write " << dest << "\n";
    return false;
  }

  // Finally, copy in any referenced Maya files.  This is untested code.
  unsigned int num_refs = refs.length();
  if (num_refs != 0) {
    maya_cat.warning()
      << "External references are not yet properly supported by mayacopy!\n";
  }
  for (unsigned int ref_index = 0; ref_index < num_refs; ref_index++) {
    Filename filename = 
      _path_replace->convert_path(refs[ref_index].asChar());
    maya_cat.warning()
      << "External ref: " << filename << "\n";
    /*
    ExtraData ed;
    ed._type = FT_maya;

    CVSSourceDirectory *dest = import(filename, &ed, _model_dir);
    if (dest == (CVSSourceDirectory *)NULL) {
      exit(1);
    }
    */
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_texture
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_texture(const Filename &source, const Filename &dest,
             CVSSourceDirectory *dir) {
  if (!copy_binary_file(source, dest)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::collect_shaders
//       Access: Private
//  Description: Recursively walks through the maya scene graph
//               hierarchy, looking for shaders.
////////////////////////////////////////////////////////////////////
bool MayaCopy::
collect_shaders() {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // This while loop walks through the entire Maya hierarchy, one node
  // at a time.  Maya's MItDag object automatically performs a
  // depth-first traversal of its scene graph.
  bool all_ok = true;
  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
    } else {
      if (!collect_shader_for_node(dag_path)) {
        all_ok = false;
      }
    }

    dag_iterator.next();
  }

  if (!all_ok) {
    nout << "Errors encountered in traversal.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::collect_shader_for_node
//       Access: Private
//  Description: Gets the relevant shader on the current node, if it
//               has one.
////////////////////////////////////////////////////////////////////
bool MayaCopy::
collect_shader_for_node(const MDagPath &dag_path) {
  MStatus status;
  MFnDagNode dag_node(dag_path, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return false;
  }

  if (dag_path.hasFn(MFn::kNurbsSurface)) {
    MFnNurbsSurface surface(dag_path, &status);
    if (status) {
      _shaders.find_shader_for_node(surface.object());
    }

  } else if (dag_path.hasFn(MFn::kMesh)) {
    MFnMesh mesh(dag_path, &status);
    if (status) {
      // Meshes may have multiple different shaders.
      MObjectArray shaders;
      MIntArray poly_shader_indices;

      status = mesh.getConnectedShaders(dag_path.instanceNumber(),
                                        shaders, poly_shader_indices);
      if (status) {
        unsigned int num_shaders = shaders.length();
        for (unsigned int shader_index = 0;
             shader_index < num_shaders; 
             shader_index++) {
          MObject engine = shaders[shader_index];
          _shaders.find_shader_for_shading_engine(engine);
        }
      }
    }

  } else {
    // Ignoring other kinds of node.
  }

  return true;
}


int main(int argc, char *argv[]) {
  MayaCopy prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
