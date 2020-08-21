/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaShader.h
 * @author drose
 * @date 2000-02-01
 */

#ifndef MAYASHADER_H
#define MAYASHADER_H

#include "pandatoolbase.h"
#include "mayaShaderColorDef.h"

#include "luse.h"
#include "lmatrix.h"
#include "namable.h"


/**
 * Corresponds to a single "shader" in Maya.  This extracts out all the
 * parameters of a Maya shader that we might care about.  There are many more
 * parameters that we don't care about or don't know enough to extract.
 */
class MayaShader : public Namable {
public:
  MayaShader(MObject engine, bool legacy_shader);
  ~MayaShader();

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

private:
  bool find_textures_modern(MObject shader);
  bool find_textures_legacy(MObject shader);

public:
  void collect_maps();
  bool _legacy_mode;

  MayaShaderColorList _all_maps;

public: // relevant only to modern mode.

  LColord _flat_color;

  MayaShaderColorList _color_maps;
  MayaShaderColorList _trans_maps;
  MayaShaderColorList _normal_maps;
  MayaShaderColorList _glow_maps;
  MayaShaderColorList _gloss_maps;
  MayaShaderColorList _height_maps;

  void bind_uvsets(MayaFileToUVSetMap &map);

private:
  void calculate_pairings();
  bool try_pair(MayaShaderColorDef *map1,
                MayaShaderColorDef *map2,
                bool perfect);
  std::string get_file_prefix(const std::string &fn);
  bool _legacy_shader;
public: // relevant only to legacy mode.
  MayaShaderColorList _color;
  MayaShaderColorDef  _transparency;
  LColor get_rgba(size_t idx=0) const;
  MayaShaderColorDef *get_color_def(size_t idx=0) const;
};

INLINE std::ostream &operator << (std::ostream &out, const MayaShader &shader) {
  shader.output(out);
  return out;
}

#endif
