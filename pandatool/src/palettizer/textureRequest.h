// Filename: textureRequest.h
// Created by:  drose (29Nov00)
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

#ifndef TEXTUREREQUEST_H
#define TEXTUREREQUEST_H

#include "pandatoolbase.h"

#include "textureProperties.h"

#include "eggTexture.h"
#include "eggRenderMode.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureRequest
// Description : These are the things that a user might explicitly
//               request to adjust on a texture via a line in the .txa
//               file.
////////////////////////////////////////////////////////////////////
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
  EggTexture::FilterType _minfilter;
  EggTexture::FilterType _magfilter;
  int _anisotropic_degree;
  EggRenderMode::AlphaMode _alpha_mode;
  bool _omit;
  int _margin;
  double _coverage_threshold;
};

#endif

