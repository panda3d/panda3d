/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileToEggConverter.h
 * @author drose
 * @date 2001-06-21
 */

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

/**
 *
 */
class XFileToEggConverter : public SomethingToEggConverter {
public:
  XFileToEggConverter();
  XFileToEggConverter(const XFileToEggConverter &copy);
  ~XFileToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool convert_file(const Filename &filename);
  void close();

  EggGroup *get_dart_node() const;

  EggTexture *create_unique_texture(const EggTexture &copy);
  EggMaterial *create_unique_material(const EggMaterial &copy);
  EggGroup *find_joint(const std::string &joint_name);
  void strip_nodes(TypeHandle t);

public:
  bool _make_char;
  std::string _char_name;
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
                                const std::string &joint_name, FrameData &table);
  bool convert_animation_key(XFileDataNode *obj, const std::string &joint_name,
                             FrameData &table);
  bool set_animation_frame(const std::string &joint_name, FrameData &table,
                           int frame, int key_type,
                           const XFileDataObject &values);
  bool convert_mesh(XFileDataNode *obj, EggGroupNode *egg_parent);

  bool create_polygons();
  bool create_hierarchy();

  PT(XFile) _x_file;

  bool _any_frames;
  bool _any_animation;
  int _ticks_per_second;
  int _total_tick_deltas;
  int _num_ticks;

  typedef pvector<XFileMesh *> Meshes;
  Meshes _meshes;

  typedef pvector<XFileAnimationSet *> AnimationSets;
  AnimationSets _animation_sets;

  typedef pmap<std::string, EggGroup *> Joints;
  Joints _joints;

  EggGroup *_dart_node;

  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
