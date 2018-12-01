/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaCopy.h
 * @author drose
 * @date 2002-05-10
 */

#ifndef MAYACOPY_H
#define MAYACOPY_H

#include "pandatoolbase.h"
#include "cvsCopy.h"
#include "mayaApi.h"
#include "dSearchPath.h"
#include "pointerTo.h"

#include "pset.h"

#include "pre_maya_include.h"
#include <maya/MObject.h>
#include "post_maya_include.h"

#include "mayaShaders.h"

class MayaShader;
class MayaShaderColorDef;

/**
 * A program to copy Maya .mb files into the cvs tree.
 */
class MayaCopy : public CVSCopy {
public:
  MayaCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dir, void *extra_data,
                         bool new_file);

  virtual std::string filter_filename(const std::string &source);

private:
  enum FileType {
    FT_maya,
    FT_texture
  };

  class ExtraData {
  public:
    FileType _type;
    MayaShader *_shader;
  };

  bool copy_maya_file(const Filename &source, const Filename &dest,
                     CVSSourceDirectory *dir);
  bool extract_texture(MayaShaderColorDef &color_def, CVSSourceDirectory *dir);
  bool copy_texture(const Filename &source, const Filename &dest,
                    CVSSourceDirectory *dir);

  bool collect_shaders();
  bool collect_shader_for_node(const MDagPath &dag_path);

  bool _keep_ver;
  bool _omit_tex;
  bool _omit_ref;
  int _curr_idx;
  bool _maya_ascii;
  /*
  vector_string _replace_prefix;
  */

  vector_string _exec_string;

  PT(MayaApi) _maya;
  MayaShaders _shaders;
};

#endif
