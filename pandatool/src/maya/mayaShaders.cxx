// Filename: mayaShaders.cxx
// Created by:  drose (11Feb00)
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

#include "mayaShaders.h"
#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_maya.h"

#include "pre_maya_include.h"
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFn.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaShaders::
MayaShaders() {
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaShaders::
~MayaShaders() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::find_shader_for_node
//       Access: Public
//  Description: Extracts the shader assigned to the indicated node.
////////////////////////////////////////////////////////////////////
MayaShader *MayaShaders::
find_shader_for_node(MObject node, bool legacy_shader) {
  MStatus status;
  MFnDependencyNode node_fn(node);
  // Look on the instObjGroups attribute for shading engines.
  MObject iog_attr = node_fn.attribute("instObjGroups", &status);
  if (!status) {
    // The node is not renderable.  What are you thinking?
    maya_cat.error()
      << node_fn.name().asChar() << " : not a renderable object.\n";
    return (MayaShader *)NULL;
  }

  // instObjGroups is a multi attribute, whatever that means.  For
  // now, we'll just get the first connection, since that's what the
  // example code did.  Is there any reason to search deeper?

  MPlug iog_plug(node, iog_attr);
  MPlugArray iog_pa;
  iog_plug.elementByLogicalIndex(0).connectedTo(iog_pa, false, true, &status);
  if (!status) {
    // No shading group defined for this object.
    maya_cat.error()
      << node_fn.name().asChar() << " : no shading group defined.\n";
    return (MayaShader *)NULL;
  }

  // Now we have a number of ShadingEngines defined, one for each of
  // these connections we just turned up.  Usually there will only be
  // one.  In fact, we'll just take the first one we find.

  size_t i;
  for (i = 0; i < iog_pa.length(); i++) {
    MObject engine = iog_pa[i].node();
    if (engine.hasFn(MFn::kShadingEngine)) {
      return find_shader_for_shading_engine(engine, legacy_shader);
    }
  }

  // Well, we didn't find a ShadingEngine after all.  Huh.
  maya_cat.debug()
    << node_fn.name().asChar() << " : no shading engine found.\n";
  return (MayaShader *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::bind_uvsets
//       Access: Public
//  Description: Causes all shaders in the set to use the given
//               mesh as a file-to-uvset map.
////////////////////////////////////////////////////////////////////
void MayaShaders::
bind_uvsets(MObject mesh) {
  _uvset_names.clear();
  _file_to_uvset.clear();
  
  if (mesh.hasFn(MFn::kMesh)) {
    MFnMesh mesh_fn(mesh);
    MStatus status;
    MStringArray maya_uvset_names;
    status = mesh_fn.getUVSetNames(maya_uvset_names);
    for (size_t i=0; i<maya_uvset_names.length(); ++i) {
      MObjectArray moa;
      string uvset_name = maya_uvset_names[i].asChar();
      _uvset_names.push_back(uvset_name);
      mesh_fn.getAssociatedUVSetTextures(maya_uvset_names[i], moa);
      for (size_t j=0; j<moa.length(); ++j){
        MFnDependencyNode dt(moa[j]);
        string tex_name = dt.name().asChar();
        _file_to_uvset[tex_name] = uvset_name;
      }
    }
  }
  
  Shaders::iterator sha;
  for (sha=_shaders.begin(); sha!=_shaders.end(); sha++) {
    (*sha).second->bind_uvsets(_file_to_uvset);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::find_shader_for_shading_engine
//       Access: Public
//  Description: Returns the MayaShader object associated with the
//               indicated "shading engine".  This will create a new
//               MayaShader object if this is the first time we have
//               encountered the indicated engine.
////////////////////////////////////////////////////////////////////
MayaShader *MayaShaders::
find_shader_for_shading_engine(MObject engine, bool legacy_shader) {
  MFnDependencyNode engine_fn(engine);
  // See if we have already decoded this engine.
  string engine_name = engine_fn.name().asChar();
  Shaders::const_iterator si = _shaders.find(engine_name);
  if (si != _shaders.end()) {
    return (*si).second;
  }

  // All right, this is a newly encountered shading engine.  Create a
  // new MayaShader object to represent it.
  MayaShader *shader = new MayaShader(engine, legacy_shader);
  shader->bind_uvsets(_file_to_uvset);
  
  // Record this for the future.
  _shaders.insert(Shaders::value_type(engine_name, shader));
  _shaders_in_order.push_back(shader);
  return shader;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::find_uv_link
//       Access: Public
//  Description: Returns the current mapping from file to uvset
//               for the given file texture name.  
////////////////////////////////////////////////////////////////////
string MayaShaders::
find_uv_link(const string &match) {
  MayaFileToUVSetMap::iterator it = _file_to_uvset.find(match);
  if (it == _file_to_uvset.end()) {
    return "not found";
  } else {
    return (*it).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::get_num_shaders
//       Access: Public
//  Description: Returns the number of unique MayaShaders that have
//               been discovered so far.
////////////////////////////////////////////////////////////////////
int MayaShaders::
get_num_shaders() const {
  return _shaders_in_order.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::get_shader
//       Access: Public
//  Description: Returns the nth MayaShader that has been discovered
//               so far.
////////////////////////////////////////////////////////////////////
MayaShader *MayaShaders::
get_shader(int n) const {
  nassertr(n >= 0 && n < (int)_shaders_in_order.size(), NULL);
  return _shaders_in_order[n];
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaders::clear
//       Access: Public
//  Description: Frees all of the previously-defined MayaShader
//               objects associated with this set.
////////////////////////////////////////////////////////////////////
void MayaShaders::
clear() {
  ShadersInOrder::iterator si;
  for (si = _shaders_in_order.begin(); si != _shaders_in_order.end(); ++si) {
    delete (*si);
  }

  _shaders.clear();
  _shaders_in_order.clear();
  _file_to_uvset.clear();
}
