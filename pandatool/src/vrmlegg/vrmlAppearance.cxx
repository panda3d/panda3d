// Filename: vrmlAppearance.cxx
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

#include "vrmlAppearance.h"
#include "vrmlNode.h"

VRMLAppearance::
VRMLAppearance(const VrmlNode *appearance) {
  _has_material = false;
  _transparency = 0.0;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);

  if (appearance != NULL) {
    const VrmlNode *material = appearance->get_value("material")._sfnode._p;
    if (material != NULL) {
      _has_material = true;
      const double *c = material->get_value("diffuseColor")._sfvec;
      _transparency = material->get_value("transparency")._sffloat;
      _color.set(c[0], c[1], c[2], 1.0 - _transparency);
    }

    const VrmlNode *texture = appearance->get_value("texture")._sfnode._p;
    if (texture != NULL) {
      if (strcmp(texture->_type->getName(), "ImageTexture") == 0) {
	MFArray *url = texture->get_value("url")._mf;
	if (!url->empty()) {
	  const char *filename = (*url->begin())._sfstring;
	  _tex = new EggTexture("tref", filename);
	}
      }
    }
  }
}

  
