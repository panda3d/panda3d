// Filename: mayaBlendDesc.h
// Created by:  drose (10Feb04)
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

#ifndef MAYABLENDDESC_H
#define MAYABLENDDESC_H

#include "pandatoolbase.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include "pre_maya_include.h"
#include <maya/MFnBlendShapeDeformer.h>
#include "post_maya_include.h"

class EggTable;
class EggSAnimData;

////////////////////////////////////////////////////////////////////
//       Class : MayaBlendDesc
// Description : A handle to a Maya blend shape description.  This is
//               just one target of a Maya BlendShape object, and
//               thus corresponds more or less one-to-one with a
//               single Egg morph target.  (We don't attempt to
//               support Maya's chained target shapes here; should we
//               need to later, it would mean breaking each of those
//               target shapes on the one continuous Maya slider into
//               a separate MayaBlendDesc object, and synthesizing the
//               egg slider values appropriately.)
////////////////////////////////////////////////////////////////////
class MayaBlendDesc : public ReferenceCount, public Namable {
public:
  MayaBlendDesc(MFnBlendShapeDeformer deformer, int weight_index);
  ~MayaBlendDesc();

  void set_slider(float value);
  float get_slider() const;

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
