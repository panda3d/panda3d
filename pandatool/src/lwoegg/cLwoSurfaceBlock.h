// Filename: cLwoSurfaceBlock.h
// Created by:  drose (26Apr01)
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

#ifndef CLWOSURFACEBLOCK_H
#define CLWOSURFACEBLOCK_H

#include "pandatoolbase.h"

#include "lwoSurfaceBlock.h"
#include "lwoSurfaceBlockOpacity.h"
#include "lwoSurfaceBlockProjection.h"
#include "lwoSurfaceBlockAxis.h"
#include "lwoSurfaceBlockWrap.h"

#include "luse.h"

class LwoToEggConverter;
class CLwoSurfaceBlockTMap;

////////////////////////////////////////////////////////////////////
//       Class : CLwoSurfaceBlock
// Description : This class is a wrapper around LwoSurfaceBlock and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoSurfaceBlock {
public:
  CLwoSurfaceBlock(LwoToEggConverter *converter, const LwoSurfaceBlock *block);
  ~CLwoSurfaceBlock();

  IffId _block_type;
  IffId _channel_id;
  string _ordinal;
  bool _enabled;

  LwoSurfaceBlockOpacity::Type _opacity_type;
  float _opacity;

  LMatrix4d _transform;
  LMatrix4d _inv_transform;
  LwoSurfaceBlockProjection::Mode _projection_mode;
  LwoSurfaceBlockAxis::Axis _axis;

  int _clip_index;
  LwoSurfaceBlockWrap::Mode _w_wrap;
  LwoSurfaceBlockWrap::Mode _h_wrap;
  float _w_repeat;
  float _h_repeat;
  string _uv_name;

  LwoToEggConverter *_converter;
  CPT(LwoSurfaceBlock) _block;
  CLwoSurfaceBlockTMap *_tmap;
};

#include "cLwoSurfaceBlock.I"

#endif


