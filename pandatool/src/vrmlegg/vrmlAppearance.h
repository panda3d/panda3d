// Filename: vrmlAppearance.h
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
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
  Colorf _color;
  double _transparency;
  PT_EggTexture _tex;
};

#endif
