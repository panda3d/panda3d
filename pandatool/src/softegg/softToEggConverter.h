// Filename: softToEggConverter.h
// Created by:  masad (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTTOEGGCONVERTER_H
#define SOFTTOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"
#include "softNodeTree.h"

#include "eggTextureCollection.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"

#ifdef _MIN
#undef _MIN
#endif
#ifdef _MAX
#undef _MAX
#endif

#include <SAA.h>
#include <SI_macros.h>

class EggData;
class EggGroup;
class EggTable;
class EggVertexPool;
class EggNurbsCurve;
class EggPrimitive;
class EggXfmSAnim;
class EggSAnimData;


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

  SoftNodeDesc *find_node(string name);
  int *FindClosestTriVert( EggVertexPool *vpool, SAA_DVector *vertices, int numVert );

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

  bool make_soft_skin();
  bool cleanup_soft_skin();
  bool convert_char_chan();
  bool convert_char_model();
  bool convert_hierarchy(EggGroupNode *egg_root);
  bool process_model_node(SoftNodeDesc *node_desc);

  void make_polyset(SoftNodeDesc *node_desc, EggGroup *egg_group, SAA_ModelType type);
  void make_nurb_surface(SoftNodeDesc *node_desc, EggGroup *egg_group, SAA_ModelType type);
  void add_knots( vector <double> &eggKnots, double *knots, int numKnots, SAA_Boolean closed, int degree );

  void set_shader_attributes(SoftNodeDesc *node_desc, EggPrimitive &primitive, int idx);
  void apply_texture_properties(EggTexture &tex, int uRepeat, int vRepeat);

  bool reparent_decals(EggGroupNode *egg_parent);

  string _program_name;
  bool _from_selection;

  SI_Error            result;
  SAA_Elem            model;
  SAA_Database        database;

public:

  SoftNodeTree _tree;

  SAA_Scene           scene;

  char *_getopts;
  
  // This is argv[0].
  const char *_commandName;

  // This is the entire command line.
  char _commandLine[4096];

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

  EggTextureCollection _textures;

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

  typedef pvector<EggSAnimData *> MorphTable;
  MorphTable _morph_table;

  EggTable *morph_node;
  EggSAnimData *find_morph_table(char *name);
};

extern const int TEX_PER_MAT;

#endif
