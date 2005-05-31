// Filename: dxGraphicsStateGuardian8.h
// Created by:  mike (02Feb99)
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

#ifndef DXGRAPHICSSTATEGUARDIAN8_H
#define DXGRAPHICSSTATEGUARDIAN8_H

#include "dxgsg8base.h"
#include "dxTextureContext8.h"
#include "config_dxgsg8.h"

#include "graphicsStateGuardian.h"
#include "geomprimitives.h"
#include "texture.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "cullFaceAttrib.h"
#include "renderModeAttrib.h"
#include "fog.h"
#include "pointerToArray.h"

class Light;

class DXVertexBufferContext8;
class DXIndexBufferContext8;

////////////////////////////////////////////////////////////////////
//       Class : DXGraphicsStateGuardian8
// Description : A GraphicsStateGuardian for rendering into DirectX8
//               contexts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsStateGuardian8 : public GraphicsStateGuardian {
public:
  DXGraphicsStateGuardian8(const FrameBufferProperties &properties);
  ~DXGraphicsStateGuardian8();

  virtual TextureContext *prepare_texture(Texture *tex);
  void apply_texture(int i, TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual VertexBufferContext *prepare_vertex_buffer(qpGeomVertexArrayData *data);
  void apply_vertex_buffer(VertexBufferContext *vbc);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(qpGeomPrimitive *data);
  void apply_index_buffer(IndexBufferContext *ibc);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual PT(qpGeomMunger) make_geom_munger(const RenderState *state);

  virtual void set_color_clear_value(const Colorf &value);

  virtual void do_clear(const RenderBuffer &buffer);

  virtual void prepare_display_region();
  virtual bool prepare_lens();

  virtual bool begin_frame();
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame();

  virtual bool begin_draw_primitives(const qpGeom *geom, 
                                     const qpGeomMunger *munger,
                                     const qpGeomVertexData *vertex_data);
  virtual void draw_triangles(const qpGeomTriangles *primitive);
  virtual void draw_tristrips(const qpGeomTristrips *primitive);
  virtual void draw_trifans(const qpGeomTrifans *primitive);
  virtual void draw_lines(const qpGeomLines *primitive);
  virtual void draw_linestrips(const qpGeomLinestrips *primitive);
  virtual void draw_points(const qpGeomPoints *primitive);
  virtual void end_draw_primitives();

  virtual void framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr,
                                           const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr,
                                       const RenderBuffer &rb);

  virtual void reset();

  virtual void apply_fog(Fog *fog);

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_alpha_test(const AlphaTestAttrib *attrib);
  virtual void issue_render_mode(const RenderModeAttrib *attrib);
  virtual void issue_rescale_normal(const RescaleNormalAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_depth_test(const DepthTestAttrib *attrib);
  virtual void issue_depth_write(const DepthWriteAttrib *attrib);
  virtual void issue_cull_face(const CullFaceAttrib *attrib);
  virtual void issue_fog(const FogAttrib *attrib);
  virtual void issue_depth_offset(const DepthOffsetAttrib *attrib);
  virtual void issue_tex_gen(const TexGenAttrib *attrib);
  virtual void issue_shade_model(const ShadeModelAttrib *attrib);

  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light, 
                          int light_id);

  static D3DFORMAT get_index_type(qpGeom::NumericType numeric_type);
  INLINE static DWORD Colorf_to_D3DCOLOR(const Colorf &cColorf);

protected:
  virtual void do_issue_material();
  virtual void do_issue_texture();

  virtual bool slot_new_light(int light_id);
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);

  virtual bool slot_new_clip_plane(int plane_id);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);

  virtual void set_blend_mode();

  void free_nondx_resources();            // free local internal buffers
  void free_d3d_device(void);

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  static CPT(RenderState) get_smooth_state();
  static CPT(RenderState) get_flat_state();

  void do_auto_rescale_normal();

protected:
  INLINE void enable_color_material(bool val);
  INLINE void enable_fog(bool val);
  INLINE void enable_zwritemask(bool val);
  INLINE void set_color_writemask(UINT color_writemask);
  INLINE void set_vertex_format(DWORD NewFvfType);

  INLINE static D3DTEXTUREADDRESS get_texture_wrap_mode(Texture::WrapMode wm);
  INLINE static D3DFOGMODE get_fog_mode_type(Fog::Mode m);
  const D3DCOLORVALUE &get_light_color(Light *light) const;
  INLINE static D3DTRANSFORMSTATETYPE get_tex_mat_sym(int stage_index);

  INLINE void enable_alpha_test(bool val);
  INLINE void enable_blend(bool val);
  INLINE void call_dxLightModelAmbient(const Colorf &color);
  INLINE void call_dxAlphaFunc(D3DCMPFUNC func, float refval);
  INLINE void call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc);
  static D3DBLEND get_blend_func(ColorBlendAttrib::Operand operand);
  INLINE void enable_dither(bool val);
  void report_texmgr_stats();

  void set_context(DXScreenData *new_context);
  void set_render_target();

  void set_texture_blend_mode(int i, const TextureStage *stage);

  void dx_cleanup();
  HRESULT reset_d3d_device(D3DPRESENT_PARAMETERS *pPresParams, DXScreenData **pScrn=NULL);

  bool check_cooperative_level();

  void show_frame();

  bool create_swap_chain (DXScreenData *new_context);
  bool release_swap_chain (DXScreenData *new_context);
  void copy_pres_reset(DXScreenData *new_context);

  static D3DTEXTUREFILTERTYPE get_d3d_min_type(Texture::FilterType filter_type);
  static D3DTEXTUREFILTERTYPE get_d3d_mip_type(Texture::FilterType filter_type);
  static D3DTEXTUREOP get_tex_color_op1(TextureStage::Mode mode);

protected:
  DXScreenData *_pScrn;
  LPDIRECT3DDEVICE8 _pD3DDevice;  // same as pScrn->_pD3DDevice, cached for spd
  IDirect3DSwapChain8 *_pSwapChain;
  D3DPRESENT_PARAMETERS _PresReset;  // This is built during reset device

  bool _bDXisReady;
  HRESULT _last_testcooplevel_result;

  bool _transform_stale;
  bool _vertex_blending_enabled;

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation
  bool _auto_rescale_normal;

  DWORD _CurFVFType;

  D3DCOLOR _d3dcolor_clear_value;
  UINT _color_writemask;

  Colorf _lmodel_ambient;
  float _material_ambient;
  float _material_diffuse;
  float _material_specular;
  float _material_shininess;
  float _material_emission;

  enum DxgsgFogType {
    None,
    PerVertexFog=D3DRS_FOGVERTEXMODE,
    PerPixelFog=D3DRS_FOGTABLEMODE
  };
  DxgsgFogType _doFogType;
  bool _fog_enabled;

  float _alpha_func_refval;  // d3d stores UINT, panda stores this as float.  we store float
  D3DCMPFUNC _alpha_func;

  D3DBLEND _blend_source_func;
  D3DBLEND _blend_dest_func;

  unsigned int _max_lights;
  unsigned int _max_clip_planes;

  bool _color_material_enabled;
  bool _texturing_enabled;
  bool _dither_enabled;
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _depth_write_enabled;
  bool _alpha_test_enabled;
  DWORD _clip_plane_bits;
  CullFaceAttrib::Mode _cull_face_mode;
  RenderModeAttrib::Mode _current_fill_mode;  //point/wireframe/solid

  LMatrix4f _projection_mat;

  CPT(DisplayRegion) _actual_display_region;
  const DXVertexBufferContext8 *_active_vbuffer;
  const DXIndexBufferContext8 *_active_ibuffer;

  bool _overlay_windows_supported;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian8",
                  GraphicsStateGuardian::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class wdxGraphicsWindow8;
  friend class wdxGraphicsPipe8;
  friend class wdxGraphicsWindowGroup8;
  friend class DXTextureContext8;
};

#include "dxGraphicsStateGuardian8.I"

#endif

