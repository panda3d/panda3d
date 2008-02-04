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

#include "attribSlots.h"
#include "renderAttrib.h"

AttribSlots AttribSlots::_defvals;

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::clear_to_defaults
//       Access: Private
//  Description: clears all values to their initial defaults.
////////////////////////////////////////////////////////////////////
void AttribSlots::
initialize_defvals() {

  // Step one. Fill slots with any object of right type.
  // the actual value is totally irrelevant.

  _defvals._alpha_test     = DCAST(AlphaTestAttrib,AlphaTestAttrib::make(AlphaTestAttrib::M_none, 0.0));
  _defvals._antialias      = DCAST(AntialiasAttrib,AntialiasAttrib::make(AntialiasAttrib::M_none));
  _defvals._audio_volume   = DCAST(AudioVolumeAttrib,AudioVolumeAttrib::make_identity());
  _defvals._clip_plane     = DCAST(ClipPlaneAttrib,ClipPlaneAttrib::make_all_off());
  _defvals._color          = DCAST(ColorAttrib,ColorAttrib::make_off());
  _defvals._color_blend    = DCAST(ColorBlendAttrib,ColorBlendAttrib::make_off());
  _defvals._color_scale    = DCAST(ColorScaleAttrib,ColorScaleAttrib::make_off());
  _defvals._color_write    = DCAST(ColorWriteAttrib,ColorWriteAttrib::make(ColorWriteAttrib::C_all));
  _defvals._cull_bin       = DCAST(CullBinAttrib,CullBinAttrib::make("",0));
  _defvals._cull_face      = DCAST(CullFaceAttrib,CullFaceAttrib::make(CullFaceAttrib::M_cull_counter_clockwise));
  _defvals._depth_offset   = DCAST(DepthOffsetAttrib,DepthOffsetAttrib::make(0));
  _defvals._depth_test     = DCAST(DepthTestAttrib,DepthTestAttrib::make(DepthTestAttrib::M_none));
  _defvals._depth_write    = DCAST(DepthWriteAttrib,DepthWriteAttrib::make(DepthWriteAttrib::M_on));
  _defvals._fog            = DCAST(FogAttrib,FogAttrib::make_off());
  _defvals._light          = DCAST(LightAttrib,LightAttrib::make_all_off());
  _defvals._light_ramp     = DCAST(LightRampAttrib,LightRampAttrib::make_identity());
  _defvals._material       = DCAST(MaterialAttrib,MaterialAttrib::make_off());
  _defvals._render_mode    = DCAST(RenderModeAttrib,RenderModeAttrib::make(RenderModeAttrib::M_unchanged));
  _defvals._rescale_normal = DCAST(RescaleNormalAttrib,RescaleNormalAttrib::make_default());
  _defvals._shade_model    = DCAST(ShadeModelAttrib,ShadeModelAttrib::make(ShadeModelAttrib::M_smooth));
  _defvals._shader         = DCAST(ShaderAttrib,ShaderAttrib::make_off());
  _defvals._stencil        = DCAST(StencilAttrib,StencilAttrib::make_off());
  _defvals._tex_gen        = DCAST(TexGenAttrib,TexGenAttrib::make());
  _defvals._tex_matrix     = DCAST(TexMatrixAttrib,TexMatrixAttrib::make());
  _defvals._texture        = DCAST(TextureAttrib,TextureAttrib::make_all_off());
  _defvals._transparency   = DCAST(TransparencyAttrib,TransparencyAttrib::make(TransparencyAttrib::M_none));

  // Step two. Replace each with make_default_impl.

  _defvals._alpha_test     = DCAST(AlphaTestAttrib,_defvals._alpha_test->make_default());
  _defvals._antialias      = DCAST(AntialiasAttrib,_defvals._antialias->make_default());
  _defvals._audio_volume   = DCAST(AudioVolumeAttrib,_defvals._audio_volume->make_default());
  _defvals._clip_plane     = DCAST(ClipPlaneAttrib,_defvals._clip_plane->make_default());
  _defvals._color          = DCAST(ColorAttrib,_defvals._color->make_default());
  _defvals._color_blend    = DCAST(ColorBlendAttrib,_defvals._color_blend->make_default());
  _defvals._color_scale    = DCAST(ColorScaleAttrib,_defvals._color_scale->make_default());
  _defvals._color_write    = DCAST(ColorWriteAttrib,_defvals._color_write->make_default());
  _defvals._cull_bin       = DCAST(CullBinAttrib,_defvals._cull_bin->make_default());
  _defvals._cull_face      = DCAST(CullFaceAttrib,_defvals._cull_face->make_default());
  _defvals._depth_offset   = DCAST(DepthOffsetAttrib,_defvals._depth_offset->make_default());
  _defvals._depth_test     = DCAST(DepthTestAttrib,_defvals._depth_test->make_default());
  _defvals._depth_write    = DCAST(DepthWriteAttrib,_defvals._depth_write->make_default());
  _defvals._fog            = DCAST(FogAttrib,_defvals._fog->make_default());
  _defvals._light          = DCAST(LightAttrib,_defvals._light->make_default());
  _defvals._light_ramp     = DCAST(LightRampAttrib,_defvals._light_ramp->make_default());
  _defvals._material       = DCAST(MaterialAttrib,_defvals._material->make_default());
  _defvals._render_mode    = DCAST(RenderModeAttrib,_defvals._render_mode->make_default());
  _defvals._rescale_normal = DCAST(RescaleNormalAttrib,_defvals._rescale_normal->make_default());
  _defvals._shade_model    = DCAST(ShadeModelAttrib,_defvals._shade_model->make_default());
  _defvals._shader         = DCAST(ShaderAttrib,_defvals._shader->make_default());
  _defvals._stencil        = DCAST(StencilAttrib,_defvals._stencil->make_default());
  _defvals._tex_gen        = DCAST(TexGenAttrib,_defvals._tex_gen->make_default());
  _defvals._tex_matrix     = DCAST(TexMatrixAttrib,_defvals._tex_matrix->make_default());
  _defvals._texture        = DCAST(TextureAttrib,_defvals._texture->make_default());
  _defvals._transparency   = DCAST(TransparencyAttrib,_defvals._transparency->make_default());
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AttribSlots::
AttribSlots() {
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AttribSlots::
AttribSlots(const AttribSlots &copy) :
  _alpha_test(copy._alpha_test),
  _antialias(copy._antialias),
  _audio_volume(copy._audio_volume),
  _clip_plane(copy._clip_plane),
  _color(copy._color),
  _color_blend(copy._color_blend),
  _color_scale(copy._color_scale),
  _color_write(copy._color_write),
  _cull_bin(copy._cull_bin),
  _cull_face(copy._cull_face),
  _depth_offset(copy._depth_offset),
  _depth_test(copy._depth_test),
  _depth_write(copy._depth_write),
  _fog(copy._fog),
  _light(copy._light),
  _light_ramp(copy._light_ramp),
  _material(copy._material),
  _render_mode(copy._render_mode),
  _rescale_normal(copy._rescale_normal),
  _shade_model(copy._shade_model),
  _shader(copy._shader),
  _stencil(copy._stencil),
  _tex_gen(copy._tex_gen),
  _tex_matrix(copy._tex_matrix),
  _texture(copy._texture),
  _transparency(copy._transparency)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::Copy Assignment Operator
//       Access: Public
//  Description: copy all the slots from source.
////////////////////////////////////////////////////////////////////
void AttribSlots::
operator =(const AttribSlots &src) {
  _alpha_test     = src._alpha_test;
  _antialias      = src._antialias;
  _audio_volume   = src._audio_volume;
  _clip_plane     = src._clip_plane;
  _color          = src._color;
  _color_blend    = src._color_blend;
  _color_scale    = src._color_scale;
  _color_write    = src._color_write;
  _cull_bin       = src._cull_bin;
  _cull_face      = src._cull_face;
  _depth_offset   = src._depth_offset;
  _depth_test     = src._depth_test;
  _depth_write    = src._depth_write;
  _fog            = src._fog;
  _light          = src._light;
  _light_ramp     = src._light_ramp;
  _material       = src._material;
  _render_mode    = src._render_mode;
  _rescale_normal = src._rescale_normal;
  _shade_model    = src._shade_model;
  _shader         = src._shader;
  _stencil        = src._stencil;
  _tex_gen        = src._tex_gen;
  _tex_matrix     = src._tex_matrix;
  _texture        = src._texture;
  _transparency   = src._transparency;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::get_slot
//       Access: Public
//  Description: fetch the contents of the nth slot.
////////////////////////////////////////////////////////////////////
const RenderAttrib *AttribSlots::
get_slot(int n) const {
  switch(n) {
  case  0: return DCAST(RenderAttrib, _alpha_test);
  case  1: return DCAST(RenderAttrib, _antialias);
  case  2: return DCAST(RenderAttrib, _audio_volume);
  case  3: return DCAST(RenderAttrib, _clip_plane);
  case  4: return DCAST(RenderAttrib, _color);
  case  5: return DCAST(RenderAttrib, _color_blend);
  case  6: return DCAST(RenderAttrib, _color_scale);
  case  7: return DCAST(RenderAttrib, _color_write);
  case  8: return DCAST(RenderAttrib, _cull_bin);
  case  9: return DCAST(RenderAttrib, _cull_face);
  case 10: return DCAST(RenderAttrib, _depth_offset);
  case 11: return DCAST(RenderAttrib, _depth_test);
  case 12: return DCAST(RenderAttrib, _depth_write);
  case 13: return DCAST(RenderAttrib, _fog);
  case 14: return DCAST(RenderAttrib, _light);
  case 15: return DCAST(RenderAttrib, _light_ramp);
  case 16: return DCAST(RenderAttrib, _material);
  case 17: return DCAST(RenderAttrib, _render_mode);
  case 18: return DCAST(RenderAttrib, _rescale_normal);
  case 19: return DCAST(RenderAttrib, _shade_model);
  case 20: return DCAST(RenderAttrib, _shader);
  case 21: return DCAST(RenderAttrib, _stencil);
  case 22: return DCAST(RenderAttrib, _tex_gen);
  case 23: return DCAST(RenderAttrib, _tex_matrix);
  case 24: return DCAST(RenderAttrib, _texture);
  case 25: return DCAST(RenderAttrib, _transparency);
  default:
    nassertr(false, NULL);
    return NULL;
  }
}

