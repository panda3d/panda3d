// Filename: mayaToEggConverter.h
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

#ifndef MAYATOEGGCONVERTER_H
#define MAYATOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"

#include "mayaApi.h"
#include "mayaShaders.h"
#include "eggTextureCollection.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"

class EggData;
class EggGroup;
class EggVertexPool;
class EggNurbsCurve;

class MDagPath;
class MFnNurbsSurface;
class MFnNurbsCurve;
class MFnMesh;
class MPointArray;

////////////////////////////////////////////////////////////////////
//       Class : MayaToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a single Maya file, or from the data
//               already in the global Maya model space.
//
//               Note that since the Maya API presents just one global
//               model space, it is not possible to simultaneously
//               load two distinct Maya files.
////////////////////////////////////////////////////////////////////
class MayaToEggConverter : public SomethingToEggConverter {
public:
  MayaToEggConverter(const string &program_name = "");
  MayaToEggConverter(const MayaToEggConverter &copy);
  virtual ~MayaToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  bool convert_maya();

private:
  bool process_node(const MDagPath &dag_path, EggData &data);
  void get_transform(const MDagPath &dag_path, EggGroup *egg_group);

  // I ran into core dumps trying to pass around a MFnMesh object by
  // value.  From now on, all MFn* objects will be passed around by
  // reference.
  void make_nurbs_surface(const MDagPath &dag_path, 
                          MFnNurbsSurface &surface,
                          EggGroup *group);
  EggNurbsCurve *make_trim_curve(const MFnNurbsCurve &curve,
                                 const string &nurbs_name,
                                 EggGroupNode *egg_group,
                                 int trim_curve_index);
  void make_nurbs_curve(const MDagPath &dag_path, 
                        const MFnNurbsCurve &curve,
                        EggGroup *group);
  void make_polyset(const MDagPath &dag_path,
                    const MFnMesh &mesh,
                    EggGroup *egg_group,
                    MayaShader *default_shader = NULL);

  EggGroup *get_egg_group(const string &name, EggData &data);

  typedef pmap<string, EggGroup *> Groups;
  Groups _groups;

public:
  MayaShaders _shaders;
  EggTextureCollection _textures;
  PT(MayaApi) _maya;

  bool _polygon_output;
  double _polygon_tolerance;
  bool _ignore_transforms;
};


#endif
