// Filename: mayaShaders.h
// Created by:  drose (11Feb00)
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

#ifndef MAYASHADERS_H
#define MAYASHADERS_H

#include <pandatoolbase.h>

#include "pmap.h"
#include <string>

class MayaShader;
class MObject;

class MayaShaders {
public:
  MayaShader *find_shader_for_node(MObject node);
  MayaShader *find_shader_for_shading_engine(MObject engine);

protected:

  typedef pmap<string, MayaShader *> Shaders;
  Shaders _shaders;
};

#endif

