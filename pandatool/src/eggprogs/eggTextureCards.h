/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTextureCards.h
 * @author drose
 * @date 2001-02-21
 */

#ifndef EGGTEXTURECARDS_H
#define EGGTEXTURECARDS_H

#include "pandatoolbase.h"

#include "eggWriter.h"
#include "eggTexture.h"
#include "luse.h"
#include "vector_string.h"

class EggVertexPool;
class EggVertex;

/**
 * Generates an egg file featuring a number of polygons, one for each named
 * texture.  This is a support program for getting textures through egg-
 * palettize.
 */
class EggTextureCards : public EggWriter {
public:
  EggTextureCards();

protected:
  virtual bool handle_args(Args &args);

  static bool dispatch_wrap_mode(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_filter_type(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_quality_level(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_format(const std::string &opt, const std::string &arg, void *var);

private:
  bool scan_texture(const Filename &filename, LVecBase4d &geometry,
                    int &num_channels);
  void make_vertices(const LPoint4d &geometry, EggVertexPool *vpool,
                     EggVertex *&v1, EggVertex *&v2, EggVertex *&v3, EggVertex *&v4);

public:
  void run();

  LVecBase4d _polygon_geometry;
  LVecBase2d _pixel_scale;
  bool _got_pixel_scale;
  vector_string _suffixes;
  LColor _polygon_color;
  vector_string _texture_names;
  EggTexture::WrapMode _wrap_mode;
  EggTexture::WrapMode _wrap_u;
  EggTexture::WrapMode _wrap_v;
  EggTexture::FilterType _minfilter;
  EggTexture::FilterType _magfilter;
  bool _got_aniso_degree;
  int _aniso_degree;
  EggTexture::QualityLevel _quality_level;
  EggTexture::Format _format;
  EggTexture::Format _format_1, _format_2, _format_3, _format_4;
  bool _apply_bface;
  double _frame_rate;
  bool _noexist;
};

#endif
