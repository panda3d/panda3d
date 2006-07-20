// Filename: mayaCopy.cxx
// Created by:  drose (10May02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "mayaCopy.h"
#include "config_maya.h"
#include "cvsSourceDirectory.h"
#include "mayaShader.h"
#include "dcast.h"

#include "pre_maya_include.h"
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
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

  add_option
    ("rp", "replace_prefix", 80,
     "use these prefixes when replacing reference with the recently copied file from the  "
     "source filename before it is copied into the tree.",
     &CVSCopy::dispatch_vector_string, NULL, &_replace_prefix);

  add_option
    ("omittex", "", 0,
     "Character animation files do not need to copy the texures. This option omits the "
     "textures of the models to be re-mayacopied",
     &CVSCopy::dispatch_none, &_omit_tex);

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
    _curr_idx = 0;
    ExtraData ed;
    ed._type = FT_maya;

    CVSSourceTree::FilePath path = import(*fi, &ed, _model_dir);
    if (!path.is_valid()) {
      nout << "\nUnable to copy, aborting!\n\n";
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
    if (_omit_tex) {
      return true;
    }
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
  if ((underscore != string::npos) && !isdigit(source.at(underscore+2)))
    underscore = string::npos;

  string extension = source.substr(dot);
  if (extension == ".ma") {
    // By convention, we always write out Maya binary files (even if
    // we receive a Maya ascii file for input).
    extension = ".mb";
  }

  if (underscore == string::npos) {
    // No version number appears to be present.
    return source.substr(0, dot) + extension;
  } else {
    return source.substr(0, underscore) + extension;
  }
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

  // Get the set of externally referenced Maya files.
  MStringArray refs;
  MStatus status = MFileIO::getReferences(refs);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // Finally, copy in any referenced Maya files.
  unsigned int num_refs = refs.length();

  /*
  if (num_refs != 0) {
    maya_cat.warning()
      << "External references are not yet properly supported by mayacopy!\n";
  }
  */

  if (num_refs != 0) {
    if (_replace_prefix.empty()) {
      // try to put the first word of the file name as the replace prefix
      size_t idx = source.get_basename().find("_",0);
      if (idx != string::npos) {
        string st = source.get_basename().substr(0, idx);
        _replace_prefix.push_back(st);
        maya_cat.info() << "replace_prefix = " << st << endl;
      }
      else {
        maya_cat.error()
          << "External references exist: " 
          << "please make sure to specify a _replace_prefix with -rp option\n";
        exit(1);
      }
    }
  }
  unsigned int ref_index;
  maya_cat.info() << "num_refs = " << num_refs << endl;
  for (ref_index = 0; ref_index < num_refs; ref_index++) {
    // one thing we need to do is rename the path to the base file
    // that it is referencing to. This will guarantee that the
    // refencing will stay in the copied directory. Only way I could
    // make it work properly is through following MEL command. the
    // pear character is used as an example. The maya API calls of
    // removeReference and reference didn't work for the animations
    // file -loadReference "mtpRN" -type "mayaBinary" -options "v=0"
    // "m_t_pear_zero.mb";
    string lookup = refs[ref_index].asChar();
    Filename filename = 
      _path_replace->convert_path(Filename::from_os_specific(refs[ref_index].asChar()));

    CVSSourceTree::FilePath path =
      _tree.choose_directory(filename.get_basename(), dir, _force, _interactive);
    Filename new_filename = path.get_rel_from(dir);
    _exec_string = "file -loadReference \"" + _replace_prefix[_curr_idx++] + "RN\" -type \"mayaBinary\" -options \"v=0\" \"" + new_filename.to_os_generic() + "\";";
    maya_cat.info() << "executing command: " << _exec_string << "\n";
    //MGlobal::executeCommand("file -loadReference \"mtpRN\" -type \"mayaBinary\" -options \"v=0\" \"m_t_pear_zero.mb\";");
    status  = MGlobal::executeCommand(MString(_exec_string.c_str()));
  }

  if (!_omit_tex) {
    // Get all the shaders so we can determine the set of textures.
    _shaders.clear();
    collect_shaders();
    int num_shaders = _shaders.get_num_shaders();
    for (int i = 0; i < num_shaders; i++) {
      MayaShader *shader = _shaders.get_shader(i);
      for (size_t j = 0; j < shader->_color.size(); j++) {
        if (!extract_texture(*shader->get_color_def(j), dir)) {
          return false;
        }
      }
      if (!extract_texture(shader->_transparency, dir)) {
        return false;
      }
    }
  }

  // Now write out the Maya file.
  if (!_maya->write(dest)) {
    maya_cat.error()
      << "Cannot write " << dest << "\n";
    return false;
  }
  
  for (ref_index = 0; ref_index < num_refs; ref_index++) {
    //maya_cat.info() << "refs filename: " << refs[ref_index].asChar() << "\n";
    //maya_cat.info() << "os_specific filename: " << Filename::from_os_specific(refs[ref_index].asChar()) << "\n";

    //maya_cat.info() << "-----------exec_string = " << _exec_string << endl;
    string original("loadReference");
    string replace("unloadReference");
    size_t index = _exec_string.find("loadReference");
    _exec_string.replace(index, original.length(), replace, 0, replace.length());
    maya_cat.info() << "executing command: " << _exec_string << "\n";
    status  = MGlobal::executeCommand(MString(_exec_string.c_str()));

    Filename filename = 
      _path_replace->convert_path(Filename::from_os_specific(refs[ref_index].asChar()));

    maya_cat.info()
      << "External ref: " << filename << "\n";

    // Now import the file
    ExtraData ed;
    ed._type = FT_maya;

    CVSSourceTree::FilePath path = import(filename, &ed, _model_dir);
    if (!path.is_valid()) {
      exit(1);
    }
  }

  /*
  for (unsigned int ref_index = 0; ref_index < num_refs; ref_index++) {
    Filename filename = 
      _path_replace->convert_path(Filename::from_os_specific(refs[ref_index].asChar()));
    status = MFileIO::reference(MString(filename.get_basename().c_str()));
    if (!status) {
      status.perror("MFileIO reference");
    }
    status = MFileIO::removeReference(refs[ref_index].asChar());
    if (!status) {
      status.perror("MFileIO removeReference");
    }
  }
  */

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::extract_texture
//       Access: Private
//  Description: Gets the texture out of the indicated color channel
//               and copies it in, updating the channel with the new
//               texture filename.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool MayaCopy::
extract_texture(MayaShaderColorDef &color_def, CVSSourceDirectory *dir) {
  if (color_def._has_texture) {
    Filename texture_filename = 
      _path_replace->convert_path(color_def._texture_filename);
    if (!texture_filename.exists()) {
      nout << "*** Warning: texture " << texture_filename
           << " does not exist.\n";
    } else if (!texture_filename.is_regular_file()) {
      nout << "*** Warning: texture " << texture_filename
           << " is not a regular file.\n";
    } else {
      ExtraData ed;
      ed._type = FT_texture;
      
      CVSSourceTree::FilePath texture_path =
        import(texture_filename, &ed, _map_dir);

      if (!texture_path.is_valid()) {
        return false;
      }
      
      // Update the texture reference to point to the new texture
      // filename, relative to the maya file.
      Filename new_filename = texture_path.get_rel_from(dir);
      color_def.reset_maya_texture(new_filename);
    }
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
