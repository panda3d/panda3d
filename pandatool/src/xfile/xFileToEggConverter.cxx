// Filename: xFileToEggConverter.cxx
// Created by:  drose (21Jun01)
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

#include "xFileToEggConverter.h"
#include "xFileMesh.h"
#include "xFileMaterial.h"
#include "xFileTemplates.h"
#include "config_xfile.h"

#include "eggData.h"
#include "eggGroup.h"
#include "datagram.h"
#include "eggMaterialCollection.h"
#include "eggTextureCollection.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter() {
  _dx_file = NULL;
  _dx_file_enum = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter(const XFileToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
  _dx_file = NULL;
  _dx_file_enum = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
~XFileToEggConverter() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *XFileToEggConverter::
make_copy() {
  return new XFileToEggConverter(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string XFileToEggConverter::
get_name() const {
  return "DirectX";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string XFileToEggConverter::
get_extension() const {
  return "x";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
//
//               This is designed to be as generic as possible,
//               generally in support of run-time loading.
//               Command-line converters may choose to use
//               convert_flt() instead, as it provides more control.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_file(const Filename &filename) {
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
  hr = _dx_file->CreateEnumObject
    ((void *)os_file.c_str(), DXFILELOAD_FROMFILE, &_dx_file_enum);
  if (hr != DXFILE_OK) {
    nout << "Unable to open X file: " << os_file << "\n";
    return false;
  }

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_left);
  }

  return get_toplevel();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::close
//       Access: Public
//  Description: Finalizes and closes the file previously opened via
//               convert_file().
////////////////////////////////////////////////////////////////////
void XFileToEggConverter::
close() {
  if (_dx_file != NULL) {
    if (_dx_file_enum != NULL) {
      _dx_file_enum->Release();
      _dx_file_enum = NULL;
    }
    _dx_file->Release();
    _dx_file = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_unique_texture
//       Access: Public
//  Description: Returns an EggTexture pointer whose properties match
//               that of the the given EggTexture, except for the tref
//               name.
////////////////////////////////////////////////////////////////////
EggTexture *XFileToEggConverter::
create_unique_texture(const EggTexture &copy) {
  return _textures.create_unique_texture(copy, ~EggTexture::E_tref_name);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_unique_material
//       Access: Public
//  Description: Returns an EggMaterial pointer whose properties match
//               that of the the given EggMaterial, except for the mref
//               name.
////////////////////////////////////////////////////////////////////
EggMaterial *XFileToEggConverter::
create_unique_material(const EggMaterial &copy) {
  return _materials.create_unique_material(copy, ~EggMaterial::E_mref_name);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_toplevel
//       Access: Private
//  Description: Pulls off all of the top-level objects in the .x file
//               and converts them, and their descendents, to the
//               appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
get_toplevel() {
  HRESULT hr;
  LPDIRECTXFILEDATA obj;

  hr = _dx_file_enum->GetNextDataObject(&obj);
  while (hr == DXFILE_OK) {
    if (!convert_data_object(obj, _egg_data)) {
      return false;
    }
    hr = _dx_file_enum->GetNextDataObject(&obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting top-level objects.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_object
//       Access: Private
//  Description: Converts the indicated object to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_object(LPDIRECTXFILEOBJECT obj, EggGroupNode *egg_parent) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_data_object(data_obj, egg_parent);
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_data_object
//       Access: Private
//  Description: Converts the indicated object to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_data_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == mydef_TID_D3DRMHeader) {
    // Quietly ignore headers.

  } else if (*type == TID_D3DRMFrame) {
    if (!convert_frame(obj, egg_parent)) {
      return false;
    }

  } else if (*type == TID_D3DRMFrameTransformMatrix) {
    if (!convert_transform(obj, egg_parent)) {
      return false;
    }

  } else if (*type == TID_D3DRMMesh) {
    if (!convert_mesh(obj, egg_parent)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_frame
//       Access: Private
//  Description: Converts the indicated frame to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_frame(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent) {
  HRESULT hr;

  string name = get_object_name(obj);
  EggGroup *group = new EggGroup(name);
  egg_parent->add_child(group);

  // Now walk through the children of the frame.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_object(child_obj, group)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of frame " << name << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_transform
//       Access: Private
//  Description: Reads a transform matrix, a child of a given frame,
//               and applies it to the node.  Normally this can only
//               be done if the node in question is an EggGroup, which
//               should be the case if the transform was a child of a
//               frame.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_transform(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  DatagramIterator di(raw_data);
  LMatrix4f mat;
  mat.read_datagram(di);

  if (egg_parent->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_parent);
    egg_group->set_transform(LCAST(double, mat));
    egg_group->set_group_type(EggGroup::GT_instance);
  } else {
    xfile_cat.error()
      << "Transform " << get_object_name(obj)
      << " encountered without frame!\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh
//       Access: Private
//  Description: Converts the indicated mesh to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent) {
  HRESULT hr;

  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  XFileMesh mesh;
  mesh.set_name(get_object_name(obj));
  if (!mesh.read_mesh_data(raw_data)) {
    return false;
  }

  // Now process all the child objects of the mesh.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_mesh_object(child_obj, mesh)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of mesh " << get_object_name(obj)
      << ".\n";
    return false;
  }

  if (!mesh.create_polygons(egg_parent, this)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_object
//       Access: Private
//  Description: Converts the indicated object, a child of a Mesh, to
//               the appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_mesh_data_object(data_obj, mesh);
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring mesh object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_data_object
//       Access: Private
//  Description: Converts the indicated data object, a child of a
//               Mesh, to the appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == TID_D3DRMMeshNormals) {
    if (!convert_mesh_normals(obj, mesh)) {
      return false;
    }

  } else if (*type == TID_D3DRMMeshVertexColors) {
    if (!convert_mesh_colors(obj, mesh)) {
      return false;
    }

  } else if (*type == TID_D3DRMMeshTextureCoords) {
    if (!convert_mesh_uvs(obj, mesh)) {
      return false;
    }

  } else if (*type == TID_D3DRMMeshMaterialList) {
    if (!convert_mesh_material_list(obj, mesh)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring mesh data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_normals
//       Access: Private
//  Description: Converts the indicated MeshNormals template
//               object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_normals(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!mesh.read_normal_data(raw_data)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_colors
//       Access: Private
//  Description: Converts the indicated MeshVertexColors template
//               object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_colors(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!mesh.read_color_data(raw_data)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_uvs
//       Access: Private
//  Description: Converts the indicated MeshTextureCoords template
//               object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_uvs(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!mesh.read_uv_data(raw_data)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh_material_list
//       Access: Private
//  Description: Converts the indicated MeshMaterialList template
//               object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh_material_list(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  HRESULT hr;

  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!mesh.read_material_list_data(raw_data)) {
    return false;
  }

  // Now we need to process the children of the material list.  These
  // will be the materials.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_material_list_object(child_obj, mesh)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of MeshMaterialList "
      << get_object_name(obj) << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_material_list_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               MeshMaterialList.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_material_list_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_material_list_data_object(data_obj, mesh);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_material_list_data_object(data_obj, mesh);
    }
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring material list object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_material_list_data_object
//       Access: Private
//  Description: Converts the indicated data object, a child of a
//               MeshMaterialList.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_material_list_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == TID_D3DRMMaterial) {
    if (!convert_material(obj, mesh)) {
      return false;
    }
  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring material list data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_material
//       Access: Private
//  Description: Converts the indicated Material template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_material(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  HRESULT hr;

  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  XFileMaterial *material = new XFileMaterial;

  if (!material->read_material_data(raw_data)) {
    delete material;
    return false;
  }

  mesh.add_material(material);

  // Now we need to process the children of the material.  There
  // should only be one, and it will be a texture.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_material_object(child_obj, *material)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of Material " 
      << get_object_name(obj) << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_material_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               Material.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_material_object(LPDIRECTXFILEOBJECT obj, XFileMaterial &material) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_material_data_object(data_obj, material);
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring material object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_material_data_object
//       Access: Private
//  Description: Converts the indicated data object, a child of a
//               Material.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_material_data_object(LPDIRECTXFILEDATA obj, XFileMaterial &material) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == TID_D3DRMTextureFilename) {
    if (!convert_texture(obj, material)) {
      return false;
    }
  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring material data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_texture
//       Access: Private
//  Description: Converts the indicated TextureFilename template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_texture(LPDIRECTXFILEDATA obj, XFileMaterial &material) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!material.read_texture_data(raw_data)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_object_name
//       Access: Private
//  Description: Returns the name of the indicated object.
////////////////////////////////////////////////////////////////////
string XFileToEggConverter::
get_object_name(LPDIRECTXFILEOBJECT obj) {
  HRESULT hr;

  DWORD length = 0;
  obj->GetName(NULL, &length);
  
  if (length == 0) {
    return string();
  }

  char *buffer = new char[length];
  hr = obj->GetName(buffer, &length);

  string result;
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get object name.\n";
  } else {
    result = buffer;
  }

  delete[] buffer;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_data
//       Access: Private
//  Description: Extracts out the raw data for the indicated object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
get_data(LPDIRECTXFILEDATA obj, Datagram &raw_data) {
  HRESULT hr;
  DWORD length;
  void *data;
  hr = obj->GetData(NULL, &length, &data);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get data for " << get_object_name(obj) << "\n";
    return false;
  }

  raw_data.clear();
  raw_data.append_data(data, length);

  return true;
}
