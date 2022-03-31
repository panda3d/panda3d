/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEggConverter.h
 * @author drose
 * @date 1999-11-10
 */

#ifndef MAYATOEGGCONVERTER_H
#define MAYATOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"
#include "mayaNodeTree.h"

#include "mayaApi.h"
#include "mayaShaders.h"
#include "mayaShaderColorDef.h"
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

/**
 * This class supervises the construction of an EggData structure from a
 * single Maya file, or from the data already in the global Maya model space.
 *
 * Note that since the Maya API presents just one global model space, it is
 * not possible to simultaneously load two distinct Maya files.
 */
class MayaToEggConverter : public SomethingToEggConverter {
public:
  MayaToEggConverter(const std::string &program_name = "");
  MayaToEggConverter(const MayaToEggConverter &copy);
  virtual ~MayaToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual std::string get_additional_extensions() const;

  virtual bool convert_file(const Filename &filename);
  virtual DistanceUnit get_input_units();

  void clear_subroots();
  void add_subroot(const GlobPattern &glob);

  void clear_subsets();
  void add_subset(const GlobPattern &glob);

  void clear_excludes();
  void add_exclude(const GlobPattern &glob);

  void clear_ignore_sliders();
  void add_ignore_slider(const GlobPattern &glob);
  bool ignore_slider(const std::string &name) const;

  void clear_force_joints();
  void add_force_joint(const GlobPattern &glob);
  bool force_joint(const std::string &name) const;

  void set_from_selection(bool from_selection);

  bool convert_maya();

  void clear();

  bool open_api(bool revert_directory=true);
  void close_api();

private:
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

  // I ran into core dumps trying to pass around a MFnMesh object by value.
  // From now on, all MFn* objects will be passed around by reference.  void
  // make_tex_names(const MFnMesh &mesh, const MObject &mesh_object);

  void make_nurbs_surface(MayaNodeDesc *node_desc,
                          const MDagPath &dag_path,
                          MFnNurbsSurface &surface,
                          EggGroup *group);
  EggNurbsCurve *make_trim_curve(const MFnNurbsCurve &curve,
                                 const std::string &nurbs_name,
                                 EggGroupNode *egg_group,
                                 int trim_curve_index);
  void make_nurbs_curve(const MDagPath &dag_path,
                        const MFnNurbsCurve &curve,
                        EggGroup *group);
  void make_polyset(MayaNodeDesc *node_desc, const MDagPath &dag_path,
                    const MFnMesh &mesh, EggGroup *egg_group,
                    MayaShader *default_shader = nullptr);
  void make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  void make_camera_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  void make_light_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  bool get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                          pvector<EggGroup *> &joints, MFloatArray &weights);
  bool get_vertex_weights(const MDagPath &dag_path, const MFnNurbsSurface &surface,
                          pvector<EggGroup *> &joints, MFloatArray &weights);
  void apply_texture_uvprops(EggTexture &tex,
                             const MayaShaderColorDef &color_def);
  void apply_texture_blendtype(EggTexture &tex,
                               const MayaShaderColorDef &color_def);
  void apply_texture_filename(EggTexture &tex,
                              const MayaShaderColorDef &color_def);
  void apply_texture_alpha_filename(EggTexture &tex,
                                    const MayaShaderColorDef &color_def);
  bool compare_texture_uvprops(EggTexture &tex,
                               const MayaShaderColorDef &color_def);
  bool reparent_decals(EggGroupNode *egg_parent);
  void set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader,
                             bool mesh = false);
  void set_shader_modern(EggPrimitive &primitive, const MayaShader &shader,
                         bool mesh);
  void set_shader_legacy(EggPrimitive &primitive, const MayaShader &shader,
                         bool mesh);
  void set_vertex_color(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color);

  void set_vertex_color_legacy(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color);

  void set_vertex_color_modern(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color);

  int round(double value);

  std::string _program_name;

  bool _from_selection;
  std::string _subroot;
  typedef pvector<GlobPattern> Globs;
  Globs _subsets;
  Globs _subroots;
  Globs _excludes;
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
  bool _keep_all_uvsets;
  bool _convert_cameras;
  bool _convert_lights;
  bool _round_uvs;
  bool _legacy_shader;


  enum TransformType {
    TT_invalid,
    TT_all,
    TT_model,
    TT_dcs,
    TT_none,
  };
  TransformType _transform_type;

  static TransformType string_transform_type(const std::string &arg);
};


#endif
