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

#ifndef __MaxToEggConverter__H
#define __MaxToEggConverter__H

#pragma conform(forScope, off)

#include "pandatoolbase.h"

/* 3ds Max Includes, with bonus(!) memory protection
 */
#ifdef MAX5
#include "pre_max_include.h"
#endif
#include "Max.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "istdplug.h"
#include "iskin.h"
#include "resource.h"
#include "stdmat.h"
#include "phyexp.h"
#ifdef MAX5
#include "post_max_include.h"
#endif

/* Panda Includes
 */
#include "eggCoordinateSystem.h"
#include "eggGroup.h"
#include "eggPolygon.h"
#include "eggTextureCollection.h"
#include "eggTexture.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "pandatoolbase.h"
#include "somethingToEgg.h"
#include "somethingToEggConverter.h"
#include "eggXfmSAnim.h"

/* Local Includes
 */
#include "maxNodeTree.h"

/* Error-Reporting Includes
 */
#include "Logger.h"
#define MTEC Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM4

/* Helpful Defintions and Casts
 */
#define null 0
#define PHYSIQUE_CLASSID Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)

/* External Helper Functions for UI
 */
// *** Figure out why this is causing link errors
//DWORD WINAPI ProgressBarFunction(LPVOID arg);

////////////////////////////////////////////////////////////////////
//       Class : MaxToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a Max model
////////////////////////////////////////////////////////////////////
class MaxToEggConverter : public SomethingToEggConverter {
 public:
  MaxToEggConverter(const string &program_name = "");
  MaxToEggConverter(const MaxToEggConverter &copy);
  virtual ~MaxToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  bool convert_max(bool from_selection);

  //Sets the interface to 3dsMax.
  void setMaxInterface(Interface *pInterface);

 private:
  double _current_frame;

  bool convert_flip(double start_frame, double end_frame, 
                    double frame_inc, double output_frame_rate);

  bool convert_char_model();
  bool convert_char_chan(double start_frame, double end_frame, 
                         double frame_inc, double output_frame_rate);
  bool convert_hierarchy(EggGroupNode *egg_root);
  bool process_model_node(MaxNodeDesc *node_desc);

  void get_transform(INode *max_node, EggGroup *egg_group);
  LMatrix4d get_object_transform(INode *max_node);
  void get_joint_transform(INode *max_node, EggGroup *egg_group);
  void get_joint_transform(INode *max_node, INode *parent_node, 
			   EggGroup *egg_group);

  // *** Leaving out these functions til there use/support in Max is determined
  /*
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
  void make_polyset(INode *max_node,
                    Mesh *mesh,
                    EggGroup *egg_group,
                    Shader *default_shader = NULL);
  /*
  void make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
                    EggGroup *egg_group);
  */

  //Gets the vertex normal for a given face and vertex. Go figure.
  Point3 get_max_vertex_normal(Mesh *mesh, int faceNo, int vertNo);
  
  void get_vertex_weights(INode *max_node, EggVertexPool *vpool);
  /*
  void set_shader_attributes(EggPrimitive &primitive,
                             const MayaShader &shader);
  */
  void set_material_attributes(EggPrimitive &primitive, INode *max_node);

  void set_material_attributes(EggPrimitive &primitive, Mtl *maxMaterial, Face *face);

  void apply_texture_properties(EggTexture &tex, 
                                StdMat *maxMaterial);
  /*
  bool compare_texture_properties(EggTexture &tex, 
                                  const MayaShaderColorDef &color_def);
  */

  bool reparent_decals(EggGroupNode *egg_parent);

  string _program_name;
  bool _from_selection;

  MaxNodeTree _tree;
  
  int _cur_tref;

 public:
  //MayaShaders _shaders;
  EggTextureCollection _textures;
  Interface *maxInterface;
  
  bool _polygon_output;
  double _polygon_tolerance;

  enum TransformType {
    TT_invalid,
    TT_all,
    TT_model,
    TT_dcs,
    TT_none,
  };
  TransformType _transform_type;

  static TransformType string_transform_type(const string &arg);

  Modifier* FindSkinModifier (INode* node, const Class_ID &type);
};


#endif
