// Filename: cLwoSurfaceBlock.h
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOSURFACEBLOCK_H
#define CLWOSURFACEBLOCK_H

#include <pandatoolbase.h>

#include <lwoSurfaceBlock.h>
#include <lwoSurfaceBlockOpacity.h>
#include <lwoSurfaceBlockProjection.h>
#include <lwoSurfaceBlockAxis.h>
#include <lwoSurfaceBlockWrap.h>

#include <luse.h>

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoSurfaceBlock
// Description : This class is a wrapper around LwoSurfaceBlock and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoSurfaceBlock {
public:
  CLwoSurfaceBlock(LwoToEggConverter *converter, const LwoSurfaceBlock *block);

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
};

#include "cLwoSurfaceBlock.I"

#endif


