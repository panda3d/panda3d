// Filename: mayaToEggConverter.h
// Created by:  drose (10Nov99)
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

#ifndef MAYATOEGGCONVERTER_H
#define MAYATOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"
#include "mayaNodeTree.h"

#include "mayaApi.h"
#include "mayaShaders.h"
#include "eggTextureCollection.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"
#include "globPattern.h"
#include "pvector.h"
#include "vector_string.h"

#include "pre_maya_include.h"
#include <maya/MDagPath.h>
#include <maya/MItMeshPolygon.h>
#include "post_maya_include.h"

class EggData;
class EggGroup;
class EggTable;
class EggVertexPool;
class EggNurbsCurve;
class EggPrimitive;
class EggXfmSAnim;
class MayaShaderColorDef;

class MObject;
class MDagPath;
class MFnDagNode;
class MFnNurbsSurface;
class MFnNurbsCurve;
class MFnMesh;
class MPointArray;
class MFloatArray;

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
  virtual string get_additional_extensions() const;

  virtual bool convert_file(const Filename &filename);
  virtual DistanceUnit get_input_units();

  void clear_subsets();
  void add_subset(const GlobPattern &glob);

  void clear_ignore_sliders();
  void add_ignore_slider(const GlobPattern &glob);
  bool ignore_slider(const string &name) const;

  void clear_force_joints();
  void add_force_joint(const GlobPattern &glob);
  bool force_joint(const string &name) const;

  void set_from_selection(bool from_selection);

  bool convert_maya();

  bool open_api();
  void close_api();

private:
  void clear();
  bool convert_flip(double start_frame, double end_frame, 
                    double frame_inc, double output_frame_rate);

  bool convert_char_model();
  bool convert_char_chan(double start_frame, double end_frame, 
                         double frame_inc, double output_frame_rate);
  bool convert_hierarchy(EggGroupNode *egg_root);
  bool process_model_node(MayaNodeDesc *node_desc);

  void get_transform(MayaNodeDesc *node_desc, const MDagPath &dag_path,
                     EggGroup *egg_group);
  void get_joint_transform(const MDagPath &dag_path, EggGroup *egg_group);
  void apply_lod_attributes(EggGroup *egg_group, MFnDagNode &lod_group);

  // I ran into core dumps trying to pass around a MFnMesh object by
  // value.  From now on, all MFn* objects will be passed around by
  // reference.
  void make_nurbs_surface(MayaNodeDesc *node_desc,
                          const MDagPath &dag_path, 
                          MFnNurbsSurface &surface,
                          EggGroup *group);
  EggNurbsCurve *make_trim_curve(const MFnNurbsCurve &curve,
                                 const string &nurbs_name,
                                 EggGroupNode *egg_group,
                                 int trim_curve_index);
  void make_nurbs_curve(const MDagPath &dag_path, 
                        const MFnNurbsCurve &curve,
                        EggGroup *group);
  void make_polyset(MayaNodeDesc *node_desc, const MDagPath &dag_path,
                    const MFnMesh &mesh, EggGroup *egg_group,
                    MayaShader *default_shader = NULL);
  void make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  bool get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                          pvector<EggGroup *> &joints, MFloatArray &weights);
  bool get_vertex_weights(const MDagPath &dag_path, const MFnNurbsSurface &surface,
                          pvector<EggGroup *> &joints, MFloatArray &weights);
  //  void set_shader_attributes(EggPrimitive &primitive,
  //                             const MayaShader &shader);
  void set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader,
                             const MItMeshPolygon *pi = NULL, 
                             const vector_string &uvset_names = vector_string());
  void apply_texture_properties(EggTexture &tex, 
                                const MayaShaderColorDef &color_def);
  bool compare_texture_properties(EggTexture &tex, 
                                  const MayaShaderColorDef &color_def);

  bool reparent_decals(EggGroupNode *egg_parent);

  string _program_name;

  bool _from_selection;
  typedef pvector<GlobPattern> Globs;
  Globs _subsets;
  Globs _ignore_sliders;
  Globs _force_joints;

  MayaNodeTree _tree;

public:
  MayaShaders _shaders;
  EggTextureCollection _textures;
  PT(MayaApi) _maya;

  bool _polygon_output;
  double _polygon_tolerance;
  bool _respect_maya_double_sided;
  bool _always_show_vertex_color;

  enum TransformType {
    TT_invalid,
    TT_all,
    TT_model,
    TT_dcs,
    TT_none,
  };
  TransformType _transform_type;

  static TransformType string_transform_type(const string &arg);
};


#endif
