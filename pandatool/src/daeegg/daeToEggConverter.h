/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeToEggConverter.h
 * @author rdb
 * @date 2008-05-08
 */

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
#include <FCollada.h>
#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDTransform.h>
#include <FCDocument/FCDEntityInstance.h>
#include <FCDocument/FCDControllerInstance.h>
#include <FCDocument/FCDGeometryMesh.h>
#include <FCDocument/FCDGeometrySpline.h>
#include <FCDocument/FCDMaterial.h>
#include <FMath/FMMatrix44.h>

#include "daeMaterials.h"
#include "daeCharacter.h"
#include "pvector.h" // Include last

/**
 * This class supervises the construction of an EggData structure from a DAE
 * file.
 */
class DAEToEggConverter : public SomethingToEggConverter {
public:
  DAEToEggConverter();
  DAEToEggConverter(const DAEToEggConverter &copy);
  ~DAEToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;

  virtual bool convert_file(const Filename &filename);
  virtual DistanceUnit get_input_units();

  bool _invert_transparency;

private:
  std::string _unit_name;
  double _unit_meters;
  PT(EggTable) _table;
  FCDocument* _document;
  FUErrorSimpleHandler* _error_handler;
  DaeCharacter::JointMap _joints;

  typedef pvector<PT(DaeCharacter)> Characters;
  Characters _characters;

  void process_asset();
  void process_node(EggGroupNode *parent, const FCDSceneNode* node, bool forced = false);
  void process_instance(EggGroup *parent, const FCDEntityInstance* instance);
  void process_mesh(EggGroup *parent, const FCDGeometryMesh* mesh,
                    DaeMaterials *materials, DaeCharacter *character = nullptr);
  void process_spline(EggGroup *parent, const std::string group_name, FCDGeometrySpline* geometry_spline);
  void process_spline(EggGroup *parent, const FCDSpline* spline);
  void process_controller(EggGroup *parent, const FCDControllerInstance* instance);
  void process_extra(EggGroup *group, const FCDExtra* extra);

  static LMatrix4d convert_matrix(const FMMatrix44& matrix);
  void apply_transform(EggGroup *to, const FCDTransform* from);

  friend class DaeCharacter;
};

#endif
