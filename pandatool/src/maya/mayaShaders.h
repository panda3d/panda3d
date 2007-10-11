// Filename: mayaShaders.h
// Created by:  drose (11Feb00)
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

#ifndef MAYASHADERS_H
#define MAYASHADERS_H

#include "pandatoolbase.h"

#include "pmap.h"
#include "pvector.h"
#include "mayaShaderColorDef.h"

class MayaShader;
class MObject;

////////////////////////////////////////////////////////////////////
//       Class : MayaShaders
// Description : Collects the set of MayaShaders that have been
//               encountered so far.
////////////////////////////////////////////////////////////////////
class MayaShaders {
public:
  MayaShaders();
  ~MayaShaders();
  MayaShader *find_shader_for_node(MObject node);
  MayaShader *find_shader_for_shading_engine(MObject engine);
  
  int get_num_shaders() const;
  MayaShader *get_shader(int n) const;
  
  MayaFileToUVSetMap _file_to_uvset;
  pvector<string> _uvset_names;
  void clear();
  void bind_uvsets(MObject mesh);
  string find_uv_link(const string &match);

private:
  typedef pmap<string, MayaShader *> Shaders;
  Shaders _shaders;
  typedef pvector<MayaShader *> ShadersInOrder;
  ShadersInOrder _shaders_in_order;
};

#endif

