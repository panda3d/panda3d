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
#include "xFile.h"
#include "somethingToEggConverter.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"
#include "pvector.h"
#include "pmap.h"
#include "luse.h"
#include "pointerTo.h"

class Datagram;
class XFileMesh;
class XFileMaterial;
class EggGroup;
class EggGroupNode;
class EggTexture;
class EggMaterial;
class XFileDataObject;

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
  void strip_nodes(TypeHandle t);

public:
  bool _make_char;
  string _char_name;
  double _frame_rate;
  bool _keep_model;
  bool _keep_animation;

private:
  typedef XFileAnimationSet::FrameData FrameData;
  
  bool get_toplevel();
  bool convert_toplevel_object(XFileDataNode *obj, EggGroupNode *egg_parent);
  bool convert_object(XFileDataNode *obj, EggGroupNode *egg_parent);
  bool convert_frame(XFileDataNode *obj, EggGroupNode *egg_parent);
  bool convert_transform(XFileDataNode *obj, EggGroupNode *egg_parent);
  bool convert_animation_set(XFileDataNode *obj);
  bool convert_animation_set_object(XFileDataNode *obj, 
                                    XFileAnimationSet &animation_set);
  bool convert_animation(XFileDataNode *obj, 
                         XFileAnimationSet &animation_set);
  bool convert_animation_object(XFileDataNode *obj, 
                                const string &joint_name, FrameData &table);
  bool convert_animation_key(XFileDataNode *obj, const string &joint_name,
                             FrameData &table);
  bool set_animation_frame(const string &joint_name, FrameData &table, 
                           int frame, int key_type,
                           const XFileDataObject &values);
  bool convert_mesh(XFileDataNode *obj, EggGroupNode *egg_parent);

  bool create_polygons();
  bool create_hierarchy();

  PT(XFile) _x_file;

  bool _any_frames;
  bool _any_animation;

  typedef pvector<XFileMesh *> Meshes;
  Meshes _meshes;

  typedef pvector<XFileAnimationSet *> AnimationSets;
  AnimationSets _animation_sets;
    
  typedef pmap<string, EggGroup *> Joints;
  Joints _joints;

  EggGroup *_dart_node;

  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
