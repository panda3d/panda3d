/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToBam.h
 * @author drose
 * @date 2000-06-28
 */

#ifndef EGGTOBAM_H
#define EGGTOBAM_H

#include "pandatoolbase.h"

#include "eggToSomething.h"
#include "pset.h"
#include "graphicsPipe.h"

class PandaNode;
class RenderState;
class Texture;
class GraphicsEngine;
class GraphicsStateGuardian;
class GraphicsOutput;

/**
 *
 */
class EggToBam : public EggToSomething {
public:
  EggToBam();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  void collect_textures(PandaNode *node);
  void collect_textures(const RenderState *state);
  void convert_txo(Texture *tex);

  bool make_buffer();

private:
  typedef pset<Texture *> Textures;
  Textures _textures;

  bool _has_egg_flatten;
  int _egg_flatten;
  bool _has_egg_combine_geoms;
  int _egg_combine_geoms;
  bool _egg_suppress_hidden;
  bool _ls;
  bool _has_compression_quality;
  int _compression_quality;
  bool _compression_off;
  bool _tex_rawdata;
  bool _tex_txo;
  bool _tex_txopz;
  bool _tex_ctex;
  bool _tex_mipmap;
  std::string _ctex_quality;
  std::string _load_display;

  // The rest of this is required to support -ctex.
  PT(GraphicsPipe) _pipe;
  GraphicsStateGuardian *_gsg;
  GraphicsEngine *_engine;
  GraphicsOutput *_buffer;
};

#endif
