// Filename: textureRequest.cxx
// Created by:  drose (30Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "textureRequest.h"
#include "palettizer.h"

////////////////////////////////////////////////////////////////////
//     Function: TextureRequest::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextureRequest::
TextureRequest() {
  _got_size = false;
  _got_num_channels = false;
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _format = EggTexture::F_unspecified;
  _force_format = false;
  _generic_format = false;
  _keep_format = false;
  _minfilter = EggTexture::FT_unspecified;
  _magfilter = EggTexture::FT_unspecified;
  _anisotropic_degree = 0;
  _alpha_mode = EggRenderMode::AM_unspecified;
  _wrap_u = EggTexture::WM_unspecified;
  _wrap_v = EggTexture::WM_unspecified;
  _omit = false;
  _margin = 0;
  _coverage_threshold = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureRequest::pre_txa_file
//       Access: Public
//  Description: Sets some state up that must be set prior to reading
//               the .txa file.
////////////////////////////////////////////////////////////////////
void TextureRequest::
pre_txa_file() {
  _margin = pal->_margin;
  _coverage_threshold = pal->_coverage_threshold;
}
