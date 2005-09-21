// Filename: attribSlots.h
// Created by:  jyelon (01Sep05)
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

#ifndef ATTRIBSLOTS_H
#define ATTRIBSLOTS_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"

#include "renderAttrib.h"
#include "alphaTestAttrib.h"
#include "antialiasAttrib.h"
#include "clipPlaneAttrib.h"
#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "cullBinAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "fogAttrib.h"
#include "lightAttrib.h"
#include "materialAttrib.h"
#include "renderModeAttrib.h"
#include "rescaleNormalAttrib.h"
#include "shadeModelAttrib.h"
#include "shaderAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : AttribSlots
// Description : This is a very simple class: an object full of
//               render attributes, one per attrib type.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA AttribSlots
{
 public:
  CPT(AlphaTestAttrib)       _alpha_test;
  CPT(AntialiasAttrib)       _antialias;
  CPT(ClipPlaneAttrib)       _clip_plane;
  CPT(ColorAttrib)           _color;
  CPT(ColorBlendAttrib)      _color_blend;
  CPT(ColorScaleAttrib)      _color_scale;
  CPT(ColorWriteAttrib)      _color_write;
  CPT(CullBinAttrib)         _cull_bin;
  CPT(CullFaceAttrib)        _cull_face;
  CPT(DepthOffsetAttrib)     _depth_offset;
  CPT(DepthTestAttrib)       _depth_test;
  CPT(DepthWriteAttrib)      _depth_write;
  CPT(FogAttrib)             _fog;
  CPT(LightAttrib)           _light;
  CPT(MaterialAttrib)        _material;
  CPT(RenderModeAttrib)      _render_mode;
  CPT(RescaleNormalAttrib)   _rescale_normal;
  CPT(ShadeModelAttrib)      _shade_model;
  CPT(ShaderAttrib)          _shader;
  CPT(TexGenAttrib)          _tex_gen;
  CPT(TexMatrixAttrib)       _tex_matrix;
  CPT(TextureAttrib)         _texture;
  CPT(TransparencyAttrib)    _transparency;
  
 public:
  AttribSlots();
  AttribSlots(const AttribSlots &copy);
  INLINE void clear_to_zero();
  INLINE void clear_to_defaults();
  void operator =(const AttribSlots &src);
  INLINE static const AttribSlots &get_defaults();

 public:
  // Each "array" reference requires a switch and a DCAST, so it's not overly fast.
  enum { slot_count=23 };
  const RenderAttrib *get_slot(int n) const;
  void set_slot(int n, const RenderAttrib *attrib);
  
 private:
  static AttribSlots _defvals;
  static void initialize_defvals();
};

#include "attribSlots.I"

#endif /* ATTRIBSLOTS_H */
