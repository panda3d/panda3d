/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaShaders.h
 * @author drose
 * @date 2000-02-11
 */

#ifndef MAYASHADERS_H
#define MAYASHADERS_H

#include "pandatoolbase.h"

#include "pmap.h"
#include "pvector.h"
#include "mayaShaderColorDef.h"

class MayaShader;

/**
 * Collects the set of MayaShaders that have been encountered so far.
 */
class MayaShaders {
public:
  MayaShaders();
  ~MayaShaders();
  MayaShader *find_shader_for_node(MObject node, bool legacy_shader);
  MayaShader *find_shader_for_shading_engine(MObject engine, bool legacy_shader);

  int get_num_shaders() const;
  MayaShader *get_shader(int n) const;

  MayaFileToUVSetMap _file_to_uvset;
  pvector<std::string> _uvset_names;
  void clear();
  void bind_uvsets(MObject mesh);
  std::string find_uv_link(const std::string &match);

private:
  typedef pmap<std::string, MayaShader *> Shaders;
  Shaders _shaders;
  typedef pvector<MayaShader *> ShadersInOrder;
  ShadersInOrder _shaders_in_order;
};

#endif
