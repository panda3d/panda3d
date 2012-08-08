// Filename: daeToEggConverter.h
// Created by:  pro-rsoft (08May08)
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

#ifndef DAETOEGGCONVERTER_H
#define DAETOEGGCONVERTER_H

#include "pandatoolbase.h"
#include "somethingToEggConverter.h"
#include "eggGroup.h"
#include "eggMaterial.h"
#include "eggTexture.h"
#include "eggTable.h"
#include "eggNurbsCurve.h"

#include "pre_fcollada_include.h"
#include "FCollada.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDTransform.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometrySpline.h"
#include "FCDocument/FCDMaterial.h"
#include "FMath/FMMatrix44.h"

#include "daeMaterials.h"
#include "pvector.h" // Include last

////////////////////////////////////////////////////////////////////
//       Class : DAEToEggConverter
// Description : This class supervises the construction of an
//               EggData structure from a DAE file.
////////////////////////////////////////////////////////////////////
class DAEToEggConverter : public SomethingToEggConverter {
public:
  DAEToEggConverter();
  DAEToEggConverter(const DAEToEggConverter &copy);
  ~DAEToEggConverter();
  
  virtual SomethingToEggConverter *make_copy();
  
  virtual string get_name() const;
  virtual string get_extension() const;
  
  virtual bool convert_file(const Filename &filename);

  bool _invert_transparency;

private:
  PT(EggTable) _table;
  FCDocument* _document;
  FUErrorSimpleHandler* _error_handler;
  pmap<const string, PT(EggGroup)> _joints;
  pmap<const string, PT(EggVertexPool)> _vertex_pools;
  pvector<string> _skeletons;
  int _frame_rate;
  
  void process_asset();
  void preprocess(const FCDSceneNode* node = NULL);
  void process_node(PT(EggGroupNode) parent, const FCDSceneNode* node, bool forced = false);
  void process_instance(PT(EggGroup) parent, const FCDEntityInstance* instance);
  void process_mesh(PT(EggGroup) parent, const FCDGeometryMesh* mesh, PT(DaeMaterials) materials);
  void process_spline(PT(EggGroup) parent, const string group_name, FCDGeometrySpline* geometry_spline);
  void process_spline(PT(EggGroup) parent, const FCDSpline* spline);
  void process_controller(PT(EggGroup) parent, const FCDControllerInstance* instance);
  //void process_table_joint(PT(EggTable) parent, FCDSceneNode* node);
  void process_extra(PT(EggGroup) group, const FCDExtra* extra);
  
  static LMatrix4d convert_matrix(const FMMatrix44& matrix);
  void apply_transform(const PT(EggGroup) to, const FCDTransform* from);
  
  friend class DaeCharacter;
};

#endif
