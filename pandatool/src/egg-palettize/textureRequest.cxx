// Filename: textureRequest.cxx
// Created by:  drose (30Nov00)
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
  _minfilter = EggTexture::FT_unspecified;
  _magfilter = EggTexture::FT_unspecified;
  _anisotropic_degree = 0;
  _alpha_mode = EggRenderMode::AM_unspecified;
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
