// Filename: mayaFile.h
// Created by:  drose (10Nov99)
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
