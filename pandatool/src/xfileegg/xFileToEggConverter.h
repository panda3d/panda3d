// Filename: xFileToEggConverter.h
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

#ifndef XFILETOEGGCONVERTER_H
#define XFILETOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "xFileAnimationSet.h"
#include "somethingToEggConverter.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"
#include "pvector.h"
#include "pmap.h"
#include "luse.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d.h>
#include <dxfile.h>
#include <rmxfguid.h>
#undef WIN32_LEAN_AND_MEAN

class Datagram;
class XFileMesh;
class XFileMaterial;
class EggGroup;
class EggGroupNode;
class EggTexture;
class EggMaterial;

////////////////////////////////////////////////////////////////////
//       Class : XFileToEggConverter
// Description : 
////////////////////////////////////////////////////////////////////
class XFileToEggConverter : public SomethingToEggConverter {
public:
  XFileToEggConverter();
  XFileToEggConverter(const XFileToEggConverter &copy);
  ~XFileToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  void close();

  EggGroup *get_dart_node() const;

  EggTexture *create_unique_texture(const EggTexture &copy);
  EggMaterial *create_unique_material(const EggMaterial &copy);
  EggGroup *find_joint(const string &joint_name);
  EggGroup *find_joint(const string &joint_name,
                       const LMatrix4f &matrix_offset);

public:
  bool _make_char;
  string _char_name;

private:
  typedef XFileAnimationSet::FrameData FrameData;
  
  bool get_toplevel();
  bool convert_toplevel_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_object(LPDIRECTXFILEOBJECT obj, EggGroupNode *egg_parent);
  bool convert_data_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_frame(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_transform(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_animation_set(LPDIRECTXFILEDATA obj);
  bool convert_animation_set_object(LPDIRECTXFILEOBJECT obj, 
                                    XFileAnimationSet &animation_set);
  bool convert_animation_set_data_object(LPDIRECTXFILEDATA obj, 
                                         XFileAnimationSet &animation_set);
  bool convert_animation(LPDIRECTXFILEDATA obj, 
                         XFileAnimationSet &animation_set);
  bool convert_animation_object(LPDIRECTXFILEOBJECT obj, 
                                const string &joint_name, FrameData &table);
  bool convert_animation_data_object(LPDIRECTXFILEDATA obj, 
                                     const string &joint_name, 
                                     FrameData &table);
  bool convert_animation_key(LPDIRECTXFILEDATA obj, const string &joint_name,
                             FrameData &table);
  bool set_animation_frame(const string &joint_name, FrameData &table, 
                           int frame, int key_type,
                           const float *values, int nvalues);
  bool convert_mesh(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent,
                    bool is_toplevel);

  bool convert_mesh_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh);
  bool convert_mesh_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_normals(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_colors(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_uvs(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_skin_weights(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_material_list(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material_list_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh);
  bool convert_material_list_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material_object(LPDIRECTXFILEOBJECT obj, XFileMaterial &material);
  bool convert_material_data_object(LPDIRECTXFILEDATA obj, XFileMaterial &material);
  bool convert_texture(LPDIRECTXFILEDATA obj, XFileMaterial &material);

  bool create_polygons();
  bool create_hierarchy();

  string get_object_name(LPDIRECTXFILEOBJECT obj);
  bool get_data(LPDIRECTXFILEDATA obj, Datagram &raw_data);

  LPDIRECTXFILE _dx_file;
  LPDIRECTXFILEENUMOBJECT _dx_file_enum;

  bool _any_frames;

  typedef pvector<XFileMesh *> Meshes;
  Meshes _meshes;
  Meshes _toplevel_meshes;

  typedef pvector<XFileAnimationSet *> AnimationSets;
  AnimationSets _animation_sets;

  typedef pmap<LMatrix4f, EggGroup *> OffsetJoints;

  // A joint definition consists of the pointer to the EggGroup that
  // represents the actual joint, plus a table of synthetic joints
  // that were created for each animation set's offset matrix (we need
  // to create a different joint to apply each unique offset matrix in
  // an animation set).
  class JointDef {
  public:
    EggGroup *_node;
    OffsetJoints _offsets;
  };
    
  typedef pmap<string, JointDef> Joints;
  Joints _joints;

  EggGroup *_dart_node;

  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
