// Filename: vrmlAppearance.h
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
////////////////////////////////////////////////////////////////////

#ifndef VRMLAPPEARANCE_H
#define VRMLAPPEARANCE_H

#include "pandatoolbase.h"
#include "eggTexture.h"
#include "pt_EggTexture.h"

class VrmlNode;

class VRMLAppearance {
public:
  VRMLAppearance(const VrmlNode *vrmlAppearance);

  bool _has_material;
  LColor _color;
  double _transparency;
  PT_EggTexture _tex;

  bool _has_tex_transform;
  LVecBase2d _tex_center;
  double _tex_rotation;
  LVecBase2d _tex_scale;
  LVecBase2d _tex_translation;
};

#endif
