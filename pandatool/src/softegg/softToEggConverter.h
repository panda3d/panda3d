// Filename: softToEggConverter.h
// Created by:  masad (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
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

#ifndef SOFTTOEGGCONVERTER_H
#define SOFTTOEGGCONVERTER_H

#ifdef _MIN
#undef _MIN
#endif
#ifdef _MAX
#undef _MAX
#endif

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"
#include "softNodeTree.h"

#include <SAA.h>
#include <SI_macros.h>

#include "eggTextureCollection.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"

class EggData;
class EggGroup;
class EggTable;
class EggVertexPool;
class EggNurbsCurve;
class EggPrimitive;
class EggXfmSAnim;
//class MayaShaderColorDef;


////////////////////////////////////////////////////////////////////
//       Class : SoftToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a single Softimage file, or from the data
//               already in th    cout << "egg name = " << eggFilename << endl;e global Softimage model space.
//
////////////////////////////////////////////////////////////////////
class SoftToEggConverter : public SomethingToEggConverter {
public:
  SoftToEggConverter(const string &program_name = "");
  SoftToEggConverter(const SoftToEggConverter &copy);
  virtual ~SoftToEggConverter();

  void Help();
  void Usage();
  void ShowOpts();

  bool HandleGetopts(int &idx, int argc, char **argv);
  bool DoGetopts(int &argc, char **&argv);

  virtual SomethingToEggConverter *make_copy();
  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  bool convert_soft(bool from_selection);
  bool open_api();
  void close_api();

private:
  bool convert_flip(double start_frame, double end_frame, 
                    double frame_inc, double output_frame_rate);

  bool convert_char_model();
  bool convert_char_chan();
  bool convert_hierarchy(EggGroupNode *egg_root);
  bool process_model_node(SoftNodeDesc *node_desc);

  /*
  void get_joint_transform(const MDagPath &dag_path, EggGroup *egg_group);

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
  */
  void make_polyset(SoftNodeDesc *node_Desc, EggGroup *egg_group, SAA_ModelType type);
  void handle_null(SAA_Elem *model, EggGroup *egg_group, SAA_ModelType type, const char *node_name);
  /*
  void make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  bool get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                          pvector<EggGroup *> &joints, MFloatArray &weights);
  */
  void set_shader_attributes(SoftNodeDesc *node_desc, EggPrimitive &primitive, char *texName);
  void apply_texture_properties(EggTexture &tex, int uRepeat, int vRepeat);
  /*
  bool compare_texture_properties(EggTexture &tex, 
                                  const MayaShaderColorDef &color_def);
  */
  bool reparent_decals(EggGroupNode *egg_parent);

  string _program_name;
  bool _from_selection;

  SoftNodeTree _tree;

  SI_Error            result;
  SAA_Scene           scene;
  SAA_Elem            model;
  SAA_Database        database;

public:

  char *_getopts;
  
  // This is argv[0].
  const char *_commandName;

  // This is the entire command line.
  const char *_commandLine;

  char        *rsrc_path;
  char        *database_name;
  char        *scene_name;
  char        *model_name;
  char        *eggFileName;
  char        *animFileName;
  char        *eggGroupName;
  char        *tex_path;
  char        *tex_filename;
  char        *search_prefix;

  int                    nurbs_step;
  int                    anim_start;
  int                    anim_end;
  int                    anim_rate;
  int                    pose_frame;
  int                    verbose;
  int                    flatten;
  int                    shift_textures;
  int                    ignore_tex_offsets;
  int                    use_prefix;
  
  bool                foundRoot;
  bool                geom_as_joint;
  bool                make_anim;
  bool                make_nurbs;
  bool                make_poly;
  bool                make_soft;
  bool                make_morph;
  bool                make_duv;
  bool                make_dart;
  bool                has_morph;
  bool                make_pose;

  
  char *GetTextureName( SAA_Scene *scene, SAA_Elem *texture );

  /*
  MayaShaders _shaders;
  */
  EggTextureCollection _textures;
  /*
  PT(MayaApi) _maya;
  */

  bool _polygon_output;
  double _polygon_tolerance;
  /*
  bool _respect_maya_double_sided;
  bool _always_show_vertex_color;
  */
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
