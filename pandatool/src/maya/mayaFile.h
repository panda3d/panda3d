// Filename: mayaFile.h
// Created by:  drose (10Nov99)
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

#ifndef MAYAFILE_H
#define MAYAFILE_H

#include <pandatoolbase.h>

#include "mayaShaders.h"

#include <eggTextureCollection.h>

class EggData;
class EggGroup;
class EggVertexPool;
class EggNurbsCurve;

class MDagPath;
class MFnNurbsSurface;
class MFnNurbsCurve;
class MFnMesh;
class MPointArray;

class MayaFile {
public:
  MayaFile();
  ~MayaFile();

  bool init(const string &program);
  bool read(const string &filename);
  void make_egg(EggData &data);

private:
  bool traverse(EggData &data);
  bool process_node(const MDagPath &dag_path, EggData &data);
  void get_transform(const MDagPath &dag_path, EggGroup *egg_group);
  void make_nurbs_surface(const MDagPath &dag_path, MFnNurbsSurface surface,
                          EggGroup *group);
  EggNurbsCurve *make_trim_curve(MFnNurbsCurve curve,
                                 const string &nurbs_name,
                                 EggGroupNode *egg_group,
                                 int trim_curve_index);
  void make_nurbs_curve(const MDagPath &dag_path, MFnNurbsCurve curve,
                        EggGroup *group);
  void make_polyset(const MDagPath &dag_path, MFnMesh mesh,
                    EggGroup *egg_group,
                    MayaShader *default_shader = NULL);

  EggGroup *get_egg_group(const string &name, EggData &data);

  typedef map<string, EggGroup *> Groups;
  Groups _groups;

public:
  double _scale_units;
  MayaShaders _shaders;
  EggTextureCollection _textures;
};


#endif
