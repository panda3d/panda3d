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
#include "somethingToEggConverter.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d.h>
#include <dxfile.h>
#include <rmxfguid.h>
#undef WIN32_LEAN_AND_MEAN

class EggGroupNode;
class Datagram;
class XFileMesh;
class XFileMaterial;
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

  EggTexture *create_unique_texture(const EggTexture &copy);
  EggMaterial *create_unique_material(const EggMaterial &copy);

private:
  bool get_toplevel();
  bool convert_toplevel_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent,
                               EggGroupNode *egg_toplevel, bool &any_frames);
  bool convert_object(LPDIRECTXFILEOBJECT obj, EggGroupNode *egg_parent);
  bool convert_data_object(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_frame(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_transform(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);
  bool convert_mesh(LPDIRECTXFILEDATA obj, EggGroupNode *egg_parent);

  bool convert_mesh_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh);
  bool convert_mesh_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_normals(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_colors(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_uvs(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_mesh_material_list(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material_list_object(LPDIRECTXFILEOBJECT obj, XFileMesh &mesh);
  bool convert_material_list_data_object(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material(LPDIRECTXFILEDATA obj, XFileMesh &mesh);
  bool convert_material_object(LPDIRECTXFILEOBJECT obj, XFileMaterial &material);
  bool convert_material_data_object(LPDIRECTXFILEDATA obj, XFileMaterial &material);
  bool convert_texture(LPDIRECTXFILEDATA obj, XFileMaterial &material);

  string get_object_name(LPDIRECTXFILEOBJECT obj);
  bool get_data(LPDIRECTXFILEDATA obj, Datagram &raw_data);

  LPDIRECTXFILE _dx_file;
  LPDIRECTXFILEENUMOBJECT _dx_file_enum;

  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
