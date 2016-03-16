/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaBlendDesc.h
 * @author drose
 * @date 2004-02-10
 */

#ifndef MAYABLENDDESC_H
#define MAYABLENDDESC_H

#include "pandatoolbase.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include "pre_maya_include.h"
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MObject.h>
#include "post_maya_include.h"

class EggTable;
class EggSAnimData;

/**
 * A handle to a Maya blend shape description.  This is just one target of a
 * Maya BlendShape object, and thus corresponds more or less one-to-one with a
 * single Egg morph target.  (We don't attempt to support Maya's chained
 * target shapes here; should we need to later, it would mean breaking each of
 * those target shapes on the one continuous Maya slider into a separate
 * MayaBlendDesc object, and synthesizing the egg slider values
 * appropriately.)
 */
class MayaBlendDesc : public ReferenceCount, public Namable {
public:
  MayaBlendDesc(MFnBlendShapeDeformer &deformer, int weight_index);
  ~MayaBlendDesc();

  void set_slider(PN_stdfloat value);
  PN_stdfloat get_slider() const;

private:
  void clear_egg();

  MFnBlendShapeDeformer _deformer;
  int _weight_index;

  EggSAnimData *_anim;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MayaBlendDesc",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class MayaNodeTree;
};

#endif
