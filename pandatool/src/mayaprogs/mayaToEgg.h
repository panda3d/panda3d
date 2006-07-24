// Filename: mayaToEgg.h
// Created by:  drose (15Feb00)
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

#ifndef MAYATOEGG_H
#define MAYATOEGG_H

#include "pandatoolbase.h"
#include "somethingToEgg.h"
#include "mayaToEggConverter.h"

////////////////////////////////////////////////////////////////////
//       Class : MayaToEgg
// Description :
////////////////////////////////////////////////////////////////////
class MayaToEgg : public SomethingToEgg {
public:
  MayaToEgg();

  void run();

protected:
  static bool dispatch_transform_type(const string &opt, const string &arg, void *var);

  int _verbose;
  bool _polygon_output;
  double _polygon_tolerance;
  bool _respect_maya_double_sided;
  bool _suppress_vertex_color;
  MayaToEggConverter::TransformType _transform_type;
  vector_string _subroots; 
  vector_string _subsets;
  vector_string _ignore_sliders;
  vector_string _force_joints;
};

#endif
