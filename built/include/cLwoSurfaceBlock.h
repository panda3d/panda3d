/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoSurfaceBlock.h
 * @author drose
 * @date 2001-04-26
 */

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

/**
 * This class is a wrapper around LwoSurfaceBlock and stores additional
 * information useful during the conversion-to-egg process.
 */
class CLwoSurfaceBlock {
public:
  CLwoSurfaceBlock(LwoToEggConverter *converter, const LwoSurfaceBlock *block);
  ~CLwoSurfaceBlock();

  IffId _block_type;
  IffId _channel_id;
  std::string _ordinal;
  bool _enabled;

  LwoSurfaceBlockOpacity::Type _opacity_type;
  PN_stdfloat _opacity;

  LMatrix4d _transform;
  LMatrix4d _inv_transform;
  LwoSurfaceBlockProjection::Mode _projection_mode;
  LwoSurfaceBlockAxis::Axis _axis;

  int _clip_index;
  LwoSurfaceBlockWrap::Mode _w_wrap;
  LwoSurfaceBlockWrap::Mode _h_wrap;
  PN_stdfloat _w_repeat;
  PN_stdfloat _h_repeat;
  std::string _uv_name;

  LwoToEggConverter *_converter;
  CPT(LwoSurfaceBlock) _block;
  CLwoSurfaceBlockTMap *_tmap;
};

#include "cLwoSurfaceBlock.I"

#endif
