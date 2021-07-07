/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureRequest.h
 * @author drose
 * @date 2000-11-29
 */

#ifndef TEXTUREREQUEST_H
#define TEXTUREREQUEST_H

#include "pandatoolbase.h"

#include "textureProperties.h"

#include "eggTexture.h"
#include "eggRenderMode.h"

/**
 * These are the things that a user might explicitly request to adjust on a
 * texture via a line in the .txa file.
 */
class TextureRequest {
public:
  TextureRequest();
  void pre_txa_file();

  TextureProperties _properties;

  bool _got_size;
  bool _got_num_channels;
  int _x_size;
  int _y_size;
  int _num_channels;
  EggTexture::Format _format;
  bool _force_format;
  bool _generic_format;
  bool _keep_format;
  bool _srgb;
  EggTexture::FilterType _minfilter;
  EggTexture::FilterType _magfilter;
  int _anisotropic_degree;
  EggRenderMode::AlphaMode _alpha_mode;
  EggTexture::WrapMode _wrap_u, _wrap_v;
  bool _omit;
  int _margin;
  double _coverage_threshold;
};

#endif
