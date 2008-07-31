// Filename: attribSlots.h
// Created by:  jyelon (01Sep05)
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

#ifndef ATTRIBSLOTS_H
#define ATTRIBSLOTS_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"

#include "renderAttrib.h"
#include "alphaTestAttrib.h"
#include "antialiasAttrib.h"
#include "audioVolumeAttrib.h"
#include "auxBitplaneAttrib.h"
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
#include "lightRampAttrib.h"
#include "materialAttrib.h"
#include "renderModeAttrib.h"
#include "rescaleNormalAttrib.h"
#include "scissorAttrib.h"
#include "shadeModelAttrib.h"
#include "shaderAttrib.h"
#include "stencilAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : AttribSlots
// Description : This is a very simple class: an object full of
//               render attributes, one per attrib type.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA_PGRAPH AttribSlots
{
 public:
  CPT(AlphaTestAttrib)       _alpha_test;
  CPT(AntialiasAttrib)       _antialias;
  CPT(AudioVolumeAttrib)     _audio_volume;
  CPT(AuxBitplaneAttrib)     _aux_bitplane;
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
  CPT(LightRampAttrib)       _light_ramp;
  CPT(MaterialAttrib)        _material;
  CPT(RenderModeAttrib)      _render_mode;
  CPT(RescaleNormalAttrib)   _rescale_normal;
  CPT(ShadeModelAttrib)      _shade_model;
  CPT(ShaderAttrib)          _shader;
  CPT(StencilAttrib)         _stencil;
  CPT(TexGenAttrib)          _tex_gen;
  CPT(TexMatrixAttrib)       _tex_matrix;
  CPT(TextureAttrib)         _texture;
  CPT(TransparencyAttrib)    _transparency;
  CPT(ScissorAttrib)         _scissor;

 public:
  AttribSlots();
  AttribSlots(const AttribSlots &copy);
  INLINE void clear_to_zero();
  INLINE void clear_to_defaults();
  void operator =(const AttribSlots &src);
  INLINE static const AttribSlots &get_defaults();

 public:
  enum { slot_count=28 };
  const RenderAttrib *get_slot(int n) const;

 private:
  static AttribSlots _defvals;
  static void initialize_defvals();
};

#include "attribSlots.I"

#endif /* ATTRIBSLOTS_H */
