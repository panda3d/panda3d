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
#include "xFileAnimationSet.h"
#include "config_xfile.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggXfmSAnim.h"
#include "eggGroupUniquifier.h"
#include "datagram.h"
#include "eggMaterialCollection.h"
#include "eggTextureCollection.h"
#include "dcast.h"

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

// These are defined in d3dx9mesh.h, which we may not have available
// (so far, Panda is only dependent on dx8 API's).
#ifndef DXFILEOBJ_XSkinMeshHeader
// {3CF169CE-FF7C-44ab-93C0-F78F62D172E2}
MY_DEFINE_GUID(DXFILEOBJ_XSkinMeshHeader,
0x3cf169ce, 0xff7c, 0x44ab, 0x93, 0xc0, 0xf7, 0x8f, 0x62, 0xd1, 0x72, 0xe2);
#endif

#ifndef DXFILEOBJ_SkinWeights
// {6F0D123B-BAD2-4167-A0D0-80224F25FABB}
MY_DEFINE_GUID(DXFILEOBJ_SkinWeights, 
0x6f0d123b, 0xbad2, 0x4167, 0xa0, 0xd0, 0x80, 0x22, 0x4f, 0x25, 0xfa, 0xbb);
#endif



////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter() {
  _make_char = false;
  _dx_file = NULL;
  _dx_file_enum = NULL;
  _dart_node = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEggConverter::
XFileToEggConverter(const XFileToEggConverter &copy) :
  SomethingToEggConverter(copy),
  _make_char(copy._make_char)
{
  _dx_file = NULL;
  _dx_file_enum = NULL;
  _dart_node = NULL;
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

  if (!get_toplevel()) {
    return false;
  }

  if (!create_polygons()) {
    return false;
  }

  if (_make_char) {
    // Now make sure that each joint has a unique name.
    EggGroupUniquifier uniquifier;
    uniquifier.uniquify(_dart_node);
  }

  if (!create_hierarchy()) {
    return false;
  }

  return true;
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

  // Clean up all the other stuff.
  Meshes::const_iterator mi;
  for (mi = _meshes.begin(); mi != _meshes.end(); ++mi) {
    delete (*mi);
  }
  _meshes.clear();

  for (mi = _toplevel_meshes.begin(); mi != _toplevel_meshes.end(); ++mi) {
    delete (*mi);
  }
  _toplevel_meshes.clear();
  
  AnimationSets::const_iterator asi;
  for (asi = _animation_sets.begin(); asi != _animation_sets.end(); ++asi) {
    delete (*asi);
  }
  _animation_sets.clear();

  _joints.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::get_dart_node
//       Access: Public
//  Description: Returns the root of the joint hierarchy, if
//               _make_char is true, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggGroup *XFileToEggConverter::
get_dart_node() const {
  return _dart_node;
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
//     Function: XFileToEggConverter::find_joint (one parameter)
//       Access: Public
//  Description: This is called by set_animation_frame, for
//               the purposes of building the frame data for the
//               animation--it needs to know the original rest frame
//               transform.
////////////////////////////////////////////////////////////////////
EggGroup *XFileToEggConverter::
find_joint(const string &joint_name) {
  Joints::iterator ji;
  ji = _joints.find(joint_name);
  if (ji != _joints.end()) {
    JointDef &joint_def = (*ji).second;
    if (joint_def._node == (EggGroup *)NULL) {
      // An invalid joint detected earlier.
      return NULL;
    }

    return joint_def._node;
  }

  // Joint name is unknown.  Issue a warning, then insert NULL into
  // the table so we don't get the same warning again with the next
  // polygon.
  if (_make_char) {
    xfile_cat.warning()
      << "Joint name " << joint_name << " in animation data is undefined.\n";
  }
  _joints[joint_name]._node = NULL;

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::find_joint (two parameters)
//       Access: Public
//  Description: This is called by XFileMesh::create_polygons(), for
//               the purposes of applying skinning to vertices.  It
//               searches for the joint matching the indicated name,
//               and returns it, possibly creating a new joint if the
//               requested matrix_offset demands it.  Returns NULL if
//               the joint name is unknown.
////////////////////////////////////////////////////////////////////
EggGroup *XFileToEggConverter::
find_joint(const string &joint_name, const LMatrix4f &matrix_offset) {
  return find_joint(joint_name);
  /*
  Joints::iterator ji;
  ji = _joints.find(joint_name);
  if (ji != _joints.end()) {
    JointDef &joint_def = (*ji).second;
    if (joint_def._node == (EggGroup *)NULL) {
      // An invalid joint detected earlier.
      return NULL;
    }

    OffsetJoints::iterator oji = joint_def._offsets.find(matrix_offset);
    if (oji != joint_def._offsets.end()) {
      // We've previously created a joint for this matrix, so just
      // reuse it.
      return (*oji).second;
    }

    if (!joint_def._offsets.empty()) {
      const LMatrix4f &mat = (*joint_def._offsets.begin()).first;
    }

    // We need to create a new joint for this matrix.
    EggGroup *new_joint = new EggGroup("synth");
    joint_def._node->add_child(new_joint);

    new_joint->set_group_type(EggGroup::GT_joint);
    new_joint->set_transform(LCAST(double, matrix_offset));
    joint_def._offsets[matrix_offset] = new_joint;

    return new_joint;
  }

  // Joint name is unknown.  Issue a warning, then insert NULL into
  // the table so we don't get the same warning again with the next
  // polygon.
  if (_make_char) {
    xfile_cat.warning()
      << "Joint name " << joint_name << " in animation data is undefined.\n";
  }
  _joints[joint_name]._node = NULL;

  return NULL;
  */
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

  EggGroupNode *egg_parent = _egg_data;
  
  // If we are converting an animatable model, make an extra node to
  // represent the root of the hierarchy.
  if (_make_char) {
    _dart_node = new EggGroup(_char_name);
    egg_parent->add_child(_dart_node);
    _dart_node->set_dart_type(EggGroup::DT_default);
    egg_parent = _dart_node;
  }

  _any_frames = false;

  hr = _dx_file_enum->GetNextDataObject(&obj);
  while (hr == DXFILE_OK) {
    if (!convert_toplevel_object(obj, egg_parent)) {
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
//     Function: XFileToEggConverter::convert_toplevel_object
//       Access: Private
//  Description: Converts the indicated object, encountered outside of
//               any Frames, to the appropriate egg structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_toplevel_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent) {
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

  } else if (*type == TID_D3DRMMaterial) {
    // Quietly ignore toplevel materials.  These will presumably be
    // referenced below.

  } else if (*type == TID_D3DRMFrame) {
    _any_frames = true;
    if (!convert_frame(obj, egg_parent)) {
      return false;
    }

  } else if (*type == TID_D3DRMFrameTransformMatrix) {
    if (!convert_transform(obj, egg_parent)) {
      return false;
    }

  } else if (*type == TID_D3DRMAnimationSet) {
    if (!convert_animation_set(obj)) {
      return false;
    }

  } else if (*type == TID_D3DRMMesh) {
    // Assume a Mesh at the toplevel is just present to define a
    // reference that will be included below.  Convert it into the
    // _toplevel_meshes set, where it will be ignored unless there are
    // no frames at all in the file.
    if (!convert_mesh(obj, egg_parent, true)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring toplevel object of unknown type: "
        << get_object_name(obj) << "\n";
    }
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
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_data_object(data_obj, egg_parent);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_data_object(data_obj, egg_parent);
    }
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
    if (!convert_mesh(obj, egg_parent, false)) {
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

  if (_make_char) {
    group->set_group_type(EggGroup::GT_joint);
    if (name.empty()) {
      // Make up a name for this unnamed joint.
      group->set_name("unnamed");

    } else {
      JointDef joint_def;
      joint_def._node = group;
      bool inserted = _joints.insert(Joints::value_type(name, joint_def)).second;
      if (!inserted) {
        xfile_cat.warning()
          << "Nonunique Frame name " << name
          << " encountered; animation will be ambiguous.\n";
      }
    }
  }

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

  } else {
    xfile_cat.error()
      << "Transform " << get_object_name(obj)
      << " encountered without frame!\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_set
//       Access: Private
//  Description: Begins an AnimationSet.  This is the root of one
//               particular animation (table of frames per joint) to
//               be applied to the model within this file.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_set(LPDIRECTXFILEDATA obj) {
  HRESULT hr;

  XFileAnimationSet *animation_set = new XFileAnimationSet();
  animation_set->set_name(get_object_name(obj));

  // Now walk through the children of the set; each one animates a
  // different joint.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_animation_set_object(child_obj, *animation_set)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of AnimationSet " 
      << get_object_name(obj) << ".\n";
    delete animation_set;
    return false;
  }

  _animation_sets.push_back(animation_set);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_set_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               AnimationSet.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_set_object(LPDIRECTXFILEOBJECT obj, 
                             XFileAnimationSet &animation_set) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_animation_set_data_object(data_obj, animation_set);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_animation_set_data_object(data_obj, animation_set);
    }
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring animation set object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_set_data_object
//       Access: Private
//  Description: Converts the indicated data object, a child of a
//               AnimationSet.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_set_data_object(LPDIRECTXFILEDATA obj, XFileAnimationSet &animation_set) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == TID_D3DRMAnimation) {
    if (!convert_animation(obj, animation_set)) {
      return false;
    }
  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring animation set data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation
//       Access: Private
//  Description: Converts the indicated Animation template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation(LPDIRECTXFILEDATA obj, XFileAnimationSet &animation_set) {
  HRESULT hr;

  // Within an Animation template, we expect to find a reference to a
  // frame, possibly an AnimationOptions object, and one or more
  // AnimationKey objects.
  LPDIRECTXFILEOBJECT child_obj;

  // First, walk through the list of children, to find the one that is
  // the frame reference.  We need to know this up front so we know
  // which table we should be building up.
  string frame_name;
  bool got_frame_name = false;

  pvector<LPDIRECTXFILEOBJECT> children;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    LPDIRECTXFILEDATAREFERENCE ref_obj;
    if (child_obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj) == DD_OK) {
      // Here's a reference!
      LPDIRECTXFILEDATA data_obj;
      if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
        const GUID *type;
        if (data_obj->GetType(&type) == DXFILE_OK) {
          if (*type == TID_D3DRMFrame) {
            // Ok, this one is a reference to a frame.  Save the name.
            frame_name = get_object_name(data_obj);
            got_frame_name = true;
          }
        }
      }
    } else {
      children.push_back(child_obj);
    }

    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of Animation " 
      << get_object_name(obj) << ".\n";
    return false;
  }

  if (!got_frame_name) {
    xfile_cat.error()
      << "Animation " << get_object_name(obj)
      << " includes no reference to a frame.\n";
    return false;
  }

  FrameData &table = animation_set.create_frame_data(frame_name);

  // Now go back again and get the actual data.
  pvector<LPDIRECTXFILEOBJECT>::iterator ci;
  for (ci = children.begin(); ci != children.end(); ++ci) {
    if (!convert_animation_object((*ci), frame_name, table)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_object
//       Access: Private
//  Description: Converts the indicated object, a child of a
//               Animation.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_object(LPDIRECTXFILEOBJECT obj, const string &joint_name,
                         XFileToEggConverter::FrameData &table) {
  HRESULT hr;
  LPDIRECTXFILEDATA data_obj;
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_animation_data_object(data_obj, joint_name, table);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_animation_data_object(data_obj, joint_name, table);
    }
  }

  // It isn't.
  if (xfile_cat.is_debug()) {
    xfile_cat.debug()
      << "Ignoring animation set object of unknown type: "
      << get_object_name(obj) << "\n";
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_data_object
//       Access: Private
//  Description: Converts the indicated data object, a child of a
//               Animation.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_data_object(LPDIRECTXFILEDATA obj, const string &joint_name,
                              XFileToEggConverter::FrameData &table) {
  HRESULT hr;

  // Determine what type of data object we have.
  const GUID *type;
  hr = obj->GetType(&type);
  if (hr != DXFILE_OK) {
    xfile_cat.error()
      << "Unable to get type of template\n";
    return false;
  }

  if (*type == TID_D3DRMAnimationOptions) {
    // Quietly ignore AnimationOptions.

  } else  if (*type == TID_D3DRMAnimationKey) {
    if (!convert_animation_key(obj, joint_name, table)) {
      return false;
    }

  } else {
    if (xfile_cat.is_debug()) {
      xfile_cat.debug()
        << "Ignoring animation set data object of unknown type: "
        << get_object_name(obj) << "\n";
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_animation_key
//       Access: Private
//  Description: Converts the indicated AnimationKey template object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_animation_key(LPDIRECTXFILEDATA obj, const string &joint_name, 
                      XFileToEggConverter::FrameData &table) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  DatagramIterator di(raw_data);
  int key_type = di.get_uint32();
  int nkeys = di.get_uint32();

  int last_time = 0;

  for (int i = 0; i < nkeys; i++) {
    int time = di.get_uint32();

    int nvalues = di.get_uint32();
    pvector<float> values;
    values.reserve(nvalues);
    for (int j = 0; j < nvalues; j++) {
      float value = di.get_float32();
      values.push_back(value);
    }

    while (last_time <= time) {
      if (!set_animation_frame(joint_name, table, last_time, key_type, 
                               &values[0], nvalues)) {
        return false;
      }
      last_time++;
    }
  }

  return true;
}
    
////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::set_animation_frame
//       Access: Private
//  Description: Sets a single frame of the animation data.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
set_animation_frame(const string &joint_name, 
                    XFileToEggConverter::FrameData &table, int frame,
                    int key_type, const float *values, int nvalues) {
  LMatrix4d mat;

  // Pad out the table by duplicating the last row as necessary.
  if ((int)table.size() <= frame) {
    if (table.empty()) {
      // Get the initial transform from the joint's rest transform.
      EggGroup *joint = find_joint(joint_name);
      if (joint != (EggGroup *)NULL) {
        mat = joint->get_transform();
      } else {
        mat = LMatrix4d::ident_mat();
      }
    } else {
      // Get the initial transform from the last frame of animation.
      mat = table.back();
    }
    table.push_back(mat);
    while ((int)table.size() <= frame) {
      table.push_back(mat);
    }

  } else {
    mat = table.back();
  }

  // Now modify the last row in the table.
  switch (key_type) {
    /*
  case 0:
    // Key type 0: rotation
    break;
    */
    
    /*
  case 1:
    // Key type 1: scale
    break;
    */
    
  case 2:
    // Key type 2: position
    if (nvalues != 3) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << nvalues << " for position data.\n";
      return false;
    }
    mat.set_row(3, LVecBase3d(values[0], values[1], values[2]));
    break;

    /*
  case 3:
    // Key type 3: ????
    break;
    */

  case 4:
    // Key type 4: full matrix
    if (nvalues != 16) {
      xfile_cat.error()
        << "Incorrect number of values in animation table: "
        << nvalues << " for matrix data.\n";
      return false;
    }
    mat.set(values[0], values[1], values[2], values[3],
            values[4], values[5], values[6], values[7],
            values[8], values[9], values[10], values[11],
            values[12], values[13], values[14], values[15]);
    break;

  default:
    xfile_cat.error()
      << "Unsupported key type " << key_type << " in animation table.\n";
    return false;
  }

  table.back() = mat;
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::convert_mesh
//       Access: Private
//  Description: Converts the indicated mesh to the appropriate egg
//               structures.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_mesh(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent, 
             bool is_toplevel) {
  HRESULT hr;

  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  XFileMesh *mesh = new XFileMesh(_egg_data->get_coordinate_system());
  mesh->set_name(get_object_name(obj));
  mesh->set_egg_parent(egg_parent);

  if (!mesh->read_mesh_data(raw_data)) {
    delete mesh;
    return false;
  }

  // Now process all the child objects of the mesh.
  LPDIRECTXFILEOBJECT child_obj;

  hr = obj->GetNextObject(&child_obj);
  while (hr == DXFILE_OK) {
    if (!convert_mesh_object(child_obj, *mesh)) {
      return false;
    }
    hr = obj->GetNextObject(&child_obj);
  }

  if (hr != DXFILEERR_NOMOREOBJECTS) {
    xfile_cat.error()
      << "Error extracting children of mesh " << get_object_name(obj)
      << ".\n";
    delete mesh;
    return false;
  }

  if (is_toplevel) {
    _toplevel_meshes.push_back(mesh);
  } else {
    _meshes.push_back(mesh);
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
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_mesh_data_object(data_obj, mesh);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_mesh_data_object(data_obj, mesh);
    }
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

  } else if (*type == DXFILEOBJ_XSkinMeshHeader) {
    // Quietly ignore a skin mesh header.
    
  } else if (*type == DXFILEOBJ_SkinWeights) {
    if (!convert_skin_weights(obj, mesh)) {
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
//     Function: XFileToEggConverter::convert_skin_weights
//       Access: Private
//  Description: Converts the indicated SkinWeights template
//               object.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
convert_skin_weights(LPDIRECTXFILEDATA obj, XFileMesh &mesh) {
  Datagram raw_data;
  if (!get_data(obj, raw_data)) {
    return false;
  }

  if (!mesh.read_skin_weights_data(raw_data)) {
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
  LPDIRECTXFILEDATAREFERENCE ref_obj;

  // See if the object is a data object.
  hr = obj->QueryInterface(IID_IDirectXFileData, (void **)&data_obj);
  if (hr == DD_OK) {
    // It is.
    return convert_material_data_object(data_obj, material);
  }

  // Or maybe it's a reference to a previous object.
  hr = obj->QueryInterface(IID_IDirectXFileDataReference, (void **)&ref_obj);
  if (hr == DD_OK) {
    // It is.
    if (ref_obj->Resolve(&data_obj) == DXFILE_OK) {
      return convert_material_data_object(data_obj, material);
    }
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
//     Function: XFileToEggConverter::create_polygons
//       Access: Private
//  Description: Creates all the polygons associated with
//               previously-saved meshes.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
create_polygons() {
  bool okflag = true;

  Meshes::const_iterator mi;
  for (mi = _meshes.begin(); mi != _meshes.end(); ++mi) {
    if (!(*mi)->create_polygons(this)) {
      okflag = false;
    }
    delete (*mi);
  }
  _meshes.clear();

  for (mi = _toplevel_meshes.begin(); mi != _toplevel_meshes.end(); ++mi) {
    if (!_any_frames) {
      if (!(*mi)->create_polygons(this)) {
        okflag = false;
      }
    }
    delete (*mi);
  }
  _toplevel_meshes.clear();

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEggConverter::create_hierarchy
//       Access: Private
//  Description: Creates the animation table hierarchies for the
//               previously-saved animation sets.
////////////////////////////////////////////////////////////////////
bool XFileToEggConverter::
create_hierarchy() {
  bool okflag = true;

  if (!_make_char && !_animation_sets.empty()) {
    xfile_cat.warning()
      << "Ignoring animation data without -a.\n";
  }

  AnimationSets::const_iterator asi;
  for (asi = _animation_sets.begin(); asi != _animation_sets.end(); ++asi) {
    if (_make_char) {
      if (!(*asi)->create_hierarchy(this)) {
        okflag = false;
      }
    }
    delete (*asi);
  }
  _animation_sets.clear();

  return okflag;
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
