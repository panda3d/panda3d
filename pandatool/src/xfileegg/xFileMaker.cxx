// Filename: xFileMaker.cxx
// Created by:  drose (19Jun01)
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

#include "xFileMaker.h"
#include "xFileMesh.h"
#include "xFileMaterial.h"
#include "xFileTemplates.h"
#include "config_xfile.h"

#include "notify.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggBin.h"
#include "eggPolysetMaker.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggData.h"
#include "pvector.h"
#include "vector_int.h"
#include "string_utils.h"
#include "datagram.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaker::
XFileMaker() {
  _dx_file = NULL;
  _dx_file_save = NULL;
  _mesh_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaker::
~XFileMaker() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::open
//       Access: Public
//  Description: Opens the indicated filename for writing, and writes
//               the .x header information; returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
open(const Filename &filename) {
  HRESULT hr;

  close();
  hr = DirectXFileCreate(&_dx_file);
  if (hr != DXFILE_OK) {
    nout << "Unable to create X interface.\n";
    return false;
  }

  // Register our templates.
  hr = _dx_file->RegisterTemplates(D3DRM_XTEMPLATES, d3drm_xtemplates_length);
  if (hr != DXFILE_OK) {
    nout << "Unable to register templates.\n";
    return false;
  }

  string os_file = filename.to_os_specific();
  hr = _dx_file->CreateSaveObject(os_file.c_str(), DXFILEFORMAT_TEXT,
                                  &_dx_file_save);
  if (hr != DXFILE_OK) {
    nout << "Unable to open X file: " << os_file << "\n";
    return false;
  }

  // Save the templates we will use.
  static const GUID *temps[] = {
    &mydef_TID_D3DRMHeader,
    &TID_D3DRMCoords2d,
    &TID_D3DRMVector,
    &TID_D3DRMColorRGBA,
    &TID_D3DRMColorRGB,
    &TID_D3DRMIndexedColor,
    &TID_D3DRMTextureFilename,
    &TID_D3DRMMatrix4x4,
    &TID_D3DRMMaterial,
    &TID_D3DRMMeshFace,
    &TID_D3DRMMesh,
    &TID_D3DRMMeshNormals,
    &TID_D3DRMMeshTextureCoords,
    &TID_D3DRMMeshMaterialList,
    &TID_D3DRMFrameTransformMatrix,
    &TID_D3DRMFrame,

    /*
      This is the complete list I extracted out of the Windows header
      files.

    &TID_D3DRMInfo,
    &TID_D3DRMMesh,
    &TID_D3DRMVector,
    &TID_D3DRMMeshFace,
    &TID_D3DRMMaterial,
    &TID_D3DRMMaterialArray,
    &TID_D3DRMFrame,
    &TID_D3DRMFrameTransformMatrix,
    &TID_D3DRMMeshMaterialList,
    &TID_D3DRMMeshTextureCoords,
    &TID_D3DRMMeshNormals,
    &TID_D3DRMCoords2d,
    &TID_D3DRMMatrix4x4,
    &TID_D3DRMAnimation,
    &TID_D3DRMAnimationSet,
    &TID_D3DRMAnimationKey,
    &TID_D3DRMFloatKeys,
    &TID_D3DRMMaterialAmbientColor,
    &TID_D3DRMMaterialDiffuseColor,
    &TID_D3DRMMaterialSpecularColor,
    &TID_D3DRMMaterialEmissiveColor,
    &TID_D3DRMMaterialPower,
    &TID_D3DRMColorRGBA,
    &TID_D3DRMColorRGB,
    &TID_D3DRMGuid,
    &TID_D3DRMTextureFilename,
    &TID_D3DRMTextureReference,
    &TID_D3DRMIndexedColor,
    &TID_D3DRMMeshVertexColors,
    &TID_D3DRMMaterialWrap,
    &TID_D3DRMBoolean,
    &TID_D3DRMMeshFaceWraps,
    &TID_D3DRMBoolean2d,
    &TID_D3DRMTimedFloatKeys,
    &TID_D3DRMAnimationOptions,
    &TID_D3DRMFramePosition,
    &TID_D3DRMFrameVelocity,
    &TID_D3DRMFrameRotation,
    &TID_D3DRMLight,
    &TID_D3DRMCamera,
    &TID_D3DRMAppData,
    &TID_D3DRMLightUmbra,
    &TID_D3DRMLightRange,
    &TID_D3DRMLightPenumbra,
    &TID_D3DRMLightAttenuation,
    &TID_D3DRMInlineData,
    &TID_D3DRMUrl,
    &TID_D3DRMProgressiveMesh,
    &TID_D3DRMExternalVisual,
    &TID_D3DRMStringProperty, 
    &TID_D3DRMPropertyBag, 
    &TID_D3DRMRightHanded, 
    */
  };
  static const int num_temps = sizeof(temps) / sizeof(temps[0]);
  hr = _dx_file_save->SaveTemplates(num_temps, temps);
  if (hr != DXFILE_OK) {
    nout << "Unable to save templates.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::close
//       Access: Public
//  Description: Finalizes and closes the file previously opened via
//               open().
////////////////////////////////////////////////////////////////////
void XFileMaker::
close() {
  if (_dx_file != NULL) {
    if (_dx_file_save != NULL) {
      _dx_file_save->Release();
      _dx_file_save = NULL;
    }
    _dx_file->Release();
    _dx_file = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_tree
//       Access: Public
//  Description: Adds the egg tree rooted at the indicated node to the
//               DX structure.  This may be somewhat destructive of
//               the egg tree.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_tree(EggData &egg_data) {
  _meshes.clear();

  // Now collect all the polygons together into polysets.
  EggPolysetMaker pmaker;
  int num_bins = pmaker.make_bins(&egg_data);

  // And now we're ready to traverse the egg hierarchy.
  if (!recurse_nodes(&egg_data, NULL)) {
    return false;
  }

  // Make sure we finalize any meshes in the root.
  if (!finalize_mesh(NULL)) {
    return false;
  }

  nassertr(_meshes.empty(), false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_node
//       Access: Private
//  Description: Adds the node to the DX structure, in whatever form
//               it is supported.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_node(EggNode *egg_node, LPDIRECTXFILEDATA dx_parent) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    return add_bin(DCAST(EggBin, egg_node), dx_parent);

  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return add_group(DCAST(EggGroup, egg_node), dx_parent);

  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // A grouping node of some kind.
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);
    LPDIRECTXFILEDATA obj;

    if (xfile_one_mesh) {
      // Don't create any additional frames representing the egg
      // hierarchy.
      if (!recurse_nodes(egg_group, dx_parent)) {
        return false;
      }

    } else {
      // Create a frame for each EggGroup.
      if (!create_frame(obj, egg_group->get_name())) {
        return false;
      }
    
      if (!recurse_nodes(egg_group, obj)) {
        obj->Release();
        return false;
      }
      
      if (!attach_and_release(obj, dx_parent)) {
        return false;
      }
    }
    
    return true;
  }

  // Some unsupported node type.  Ignore it.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_group
//       Access: Private
//  Description: Adds a frame for the indicated group node.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_group(EggGroup *egg_group, LPDIRECTXFILEDATA dx_parent) {
  if (xfile_one_mesh) {
    // Don't create any additional frames representing the egg
    // hierarchy.
    if (!recurse_nodes(egg_group, dx_parent)) {
      return false;
    }

  } else {
    // Create a frame for each EggGroup.
    LPDIRECTXFILEDATA obj;
    if (!create_frame(obj, egg_group->get_name())) {
      return false;
    }

    // Set the transform on the frame, if we have one.
    if (egg_group->has_transform()) {
      add_frame_transform(obj, LCAST(float, egg_group->get_transform()));
    }
    
    if (!recurse_nodes(egg_group, obj)) {
      obj->Release();
      return false;
    }
    
    if (!attach_and_release(obj, dx_parent)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_bin
//       Access: Private
//  Description: Determines what kind of object needs to be added for
//               the indicated bin node.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_bin(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent) {
  switch (egg_bin->get_bin_number()) {
  case EggPolysetMaker::BN_polyset:
    return add_polyset(egg_bin, dx_parent);
  }

  xfile_cat.error()
    << "Unexpected bin type " << egg_bin->get_bin_number() << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_polyset
//       Access: Private
//  Description: Adds a mesh object corresponding to the collection of
//               polygons within the indicated bin.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_polyset(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent) {
  // Make sure that all our polygons are reasonable.
  egg_bin->remove_invalid_primitives();

  XFileMesh *mesh = get_mesh(dx_parent);

  EggGroupNode::iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    EggPolygon *poly;
    DCAST_INTO_R(poly, *ci, false);

    mesh->add_polygon(poly);
  }

  return true;
}

  
////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::recurse_nodes
//       Access: Private
//  Description: Adds each child of the indicated Node as a child of
//               the indicated DX object.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
recurse_nodes(EggGroupNode *egg_node, LPDIRECTXFILEDATA dx_parent) {
  EggGroupNode::iterator ci;
  for (ci = egg_node->begin(); ci != egg_node->end(); ++ci) {
    EggNode *node = (*ci);
    if (!add_node(node, dx_parent)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::create_object
//       Access: Private
//  Description: Creates a DX data object.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
create_object(LPDIRECTXFILEDATA &obj, REFGUID template_id,
              const string &name, const Datagram &dg) {
  HRESULT hr;

  string nice_name = make_nice_name(name);

  int data_size = dg.get_length();
  void *data_pointer = (void *)dg.get_data();

  if (data_size == 0) {
    data_pointer = (void *)NULL;
  }

  hr = _dx_file_save->CreateDataObject
    (template_id, nice_name.c_str(), NULL, 
     data_size, data_pointer, &obj);

  if (hr != DXFILE_OK) {
    nout << "Unable to create data object for " << name << "\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::create_frame
//       Access: Private
//  Description: Creates a "frame" object with the indicated name.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
create_frame(LPDIRECTXFILEDATA &obj, const string &name) {
  return create_object(obj, TID_D3DRMFrame, name, Datagram());
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::add_frame_transform
//       Access: Private
//  Description: Adds a transformation matrix to the indicated frame.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
add_frame_transform(LPDIRECTXFILEDATA obj, const LMatrix4f &mat) {
  Datagram raw_data;
  mat.write_datagram(raw_data);

  LPDIRECTXFILEDATA xtransform;
  if (!create_object(xtransform, TID_D3DRMFrameTransformMatrix, 
                     "transform", raw_data)) {
    return false;
  }
  if (!attach_and_release(xtransform, obj)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::attach_and_release
//       Access: Private
//  Description: Assigns the indicated X data object to the indicated
//               parent, and releases the pointer.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
attach_and_release(LPDIRECTXFILEDATA obj, LPDIRECTXFILEDATA dx_parent) {
  HRESULT hr;

  // First, make sure we don't have an outstanding mesh for this
  // object.
  if (!finalize_mesh(obj)) {
    return false;
  }

  if (dx_parent == NULL) {
    // No parent; it's a toplevel object.
    hr = _dx_file_save->SaveData(obj);
    if (hr != DXFILE_OK) {
      nout << "Unable to save data object\n";
      obj->Release();
      return false;
    }
  } else {
    // Got a parent; it's a child of the indicated object.
    hr = dx_parent->AddDataObject(obj);
    if (hr != DXFILE_OK) {
      nout << "Unable to save data object\n";
      obj->Release();
      return false;
    }
  }

  obj->Release();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::make_nice_name
//       Access: Private, Static
//  Description: Transforms the indicated egg name to a name that is
//               acceptable to the DirectX format.
////////////////////////////////////////////////////////////////////
string XFileMaker::
make_nice_name(const string &str) {
  string result;

  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    if (isalnum(*si)) {
      result += *si;
    } else {
      result += "_";
    }
  }

  if (!str.empty() && isdigit(str[0])) {
    // If the name begins with a digit, we must make it begin with
    // something else, like for instance an underscore.
    result = '_' + result;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::get_mesh
//       Access: Private
//  Description: Returns a suitable XFileMesh object for creating
//               meshes within the indicated dx_parent object.
////////////////////////////////////////////////////////////////////
XFileMesh *XFileMaker::
get_mesh(LPDIRECTXFILEDATA dx_parent) {
  Meshes::iterator mi = _meshes.find(dx_parent);
  if (mi != _meshes.end()) {
    // We've already started working on this dx_parent before; use the
    // same mesh object.
    return (*mi).second;
  }

  // We haven't seen this dx_parent before; create a new mesh object.
  XFileMesh *mesh = new XFileMesh;
  _meshes.insert(Meshes::value_type(dx_parent, mesh));
  return mesh;
}


////////////////////////////////////////////////////////////////////
//     Function: XFileMaker::finalize_mesh
//       Access: Private
//  Description: Creates the actual DX mesh object corresponding to
//               the indicated dx_parent object.
////////////////////////////////////////////////////////////////////
bool XFileMaker::
finalize_mesh(LPDIRECTXFILEDATA dx_parent) {
  Meshes::iterator mi = _meshes.find(dx_parent);
  if (mi == _meshes.end()) {
    // We haven't got a mesh for this object; do nothing.
    return true;
  }

  XFileMesh *mesh = (*mi).second;
  _meshes.erase(mi);

  // Get a unique number for each mesh.
  _mesh_index++;
  string mesh_index = format_string(_mesh_index);

  // Finally, create the Mesh object.
  Datagram raw_data;
  mesh->make_mesh_data(raw_data);

  LPDIRECTXFILEDATA xobj;
  if (!create_object(xobj, TID_D3DRMMesh, "mesh" + mesh_index, raw_data)) {
    return false;
  }

  if (mesh->has_normals()) {
    // Tack on normals.
    LPDIRECTXFILEDATA xnormals;
    mesh->make_normal_data(raw_data);
    if (!create_object(xnormals, TID_D3DRMMeshNormals, "norms" + mesh_index,
                       raw_data)) {
      return false;
    }
    if (!attach_and_release(xnormals, xobj)) {
      return false;
    }
  }

  if (mesh->has_colors()) {
    // Tack on colors.
    LPDIRECTXFILEDATA xcolors;
    mesh->make_color_data(raw_data);
    if (!create_object(xcolors, TID_D3DRMMeshVertexColors, 
                       "colors" + mesh_index, raw_data)) {
      return false;
    }
    if (!attach_and_release(xcolors, xobj)) {
      return false;
    }
  }

  if (mesh->has_uvs()) {
    // Tack on texture coordinates.
    LPDIRECTXFILEDATA xuvs;
    mesh->make_uv_data(raw_data);
    if (!create_object(xuvs, TID_D3DRMMeshTextureCoords, 
                       "uvs" + mesh_index, raw_data)) {
      return false;
    }
    if (!attach_and_release(xuvs, xobj)) {
      return false;
    }
  }

  if (mesh->has_materials()) {
    // Tack on material definitions.
    LPDIRECTXFILEDATA xmaterial_list;
    mesh->make_material_list_data(raw_data);
    if (!create_object(xmaterial_list, TID_D3DRMMeshMaterialList, 
                       "materials" + mesh_index, raw_data)) {
      return false;
    }

    // Now we need to iterate through the list of Materials
    // themselves, and add *these* as children of the material list.
    int num_materials = mesh->get_num_materials();
    for (int i = 0; i < num_materials; i++) {
      XFileMaterial *material = mesh->get_material(i);
      LPDIRECTXFILEDATA xmaterial;
      material->make_material_data(raw_data);
      if (!create_object(xmaterial, TID_D3DRMMaterial, 
                         "material" + mesh_index + "_" + format_string(i),
                         raw_data)) {
        return false;
      }

      // Also, if the Material has a texture map, we must add *this*
      // as a child of the material.  What a weird system.
      if (material->has_texture()) {
        LPDIRECTXFILEDATA xtexture;
        material->make_texture_data(raw_data);
        if (!create_object(xtexture, TID_D3DRMTextureFilename, 
                           "texture" + mesh_index + "_" + format_string(i),
                           raw_data)) {
          return false;
        }

        if (!attach_and_release(xtexture, xmaterial)) {
          return false;
        }
      }

      if (!attach_and_release(xmaterial, xmaterial_list)) {
        return false;
      }
    }

    if (!attach_and_release(xmaterial_list, xobj)) {
      return false;
    }
  }

  if (!attach_and_release(xobj, dx_parent)) {
    return false;
  }
  return true;
}
