// Filename: dxGraphicsStateGuardian7.h
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

#ifndef DXGRAPHICSSTATEGUARDIAN7_H
#define DXGRAPHICSSTATEGUARDIAN7_H

#include "dxgsg7base.h"
#include "dxTextureContext7.h"
#include "config_dxgsg7.h"

#include "graphicsStateGuardian.h"
#include "texture.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "cullFaceAttrib.h"
#include "renderModeAttrib.h"
#include "fog.h"
#include "pointerToArray.h"

class Light;

class DXVertexBufferContext7;
class DXIndexBufferContext7;

////////////////////////////////////////////////////////////////////
//       Class : DXGraphicsStateGuardian7
// Description : A GraphicsStateGuardian for rendering into DirectX7
//               contexts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsStateGuardian7 : public GraphicsStateGuardian {
public:
  DXGraphicsStateGuardian7(const FrameBufferProperties &properties);
  ~DXGraphicsStateGuardian7();

  virtual TextureContext *prepare_texture(Texture *tex);
  void apply_texture(int i, TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state);

  virtual void set_color_clear_value(const Colorf &value);

  virtual void do_clear(const RenderBuffer &buffer);

  virtual void prepare_display_region();
  virtual bool prepare_lens();

  virtual bool begin_frame();
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame();

  virtual bool begin_draw_primitives(const Geom *geom, 
                                     const GeomMunger *munger,
                                     const GeomVertexData *vertex_data);
  void determine_fvf();
  virtual void draw_triangles(const GeomTriangles *primitive);
  virtual void draw_lines(const GeomLines *primitive);
  virtual void draw_points(const GeomPoints *primitive);
  virtual void end_draw_primitives();

  virtual void framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr,
                                           const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr,
                                       const RenderBuffer &rb);

  virtual void reset();

  virtual void apply_fog(Fog *fog);

  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light, 
                          int light_id);

  INLINE static DWORD Colorf_to_D3DCOLOR(const Colorf &cColorf);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

protected:
  void do_issue_transform();
  void do_issue_alpha_test();
  void do_issue_render_mode();
  void do_issue_rescale_normal();
  void do_issue_color_write();
  void do_issue_depth_test();
  void do_issue_depth_write();
  void do_issue_cull_face();
  void do_issue_fog();
  void do_issue_depth_offset();
  void do_issue_tex_gen();
  void do_issue_shade_model();
  void do_issue_material();
  void do_issue_texture();
  void do_issue_blending();
  
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);

  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);

  void free_nondx_resources();
  void free_d3d_device();

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  void do_auto_rescale_normal();

protected:
  INLINE static D3DTEXTUREADDRESS get_texture_wrap_mode(Texture::WrapMode wm);
  INLINE static D3DFOGMODE get_fog_mode_type(Fog::Mode m);
  const D3DCOLORVALUE &get_light_color(Light *light) const;
  INLINE static D3DTRANSFORMSTATETYPE get_tex_mat_sym(int stage_index);

  static D3DBLEND get_blend_func(ColorBlendAttrib::Operand operand);
  void report_texmgr_stats();

  void set_context(DXScreenData *new_context);
  void set_render_target();

  void set_texture_blend_mode(int i, const TextureStage *stage);

  void dx_cleanup();

  bool check_cooperative_level();

  void show_frame();
  void  show_full_screen_frame();
  void  show_windowed_frame();

  void copy_pres_reset(DXScreenData *new_context);

  static D3DTEXTUREMINFILTER get_d3d_min_type(Texture::FilterType filter_type);
  static D3DTEXTUREMIPFILTER get_d3d_mip_type(Texture::FilterType filter_type);
  static D3DTEXTUREOP get_texture_operation(TextureStage::CombineMode mode, int scale);
  static DWORD get_texture_argument(TextureStage::CombineSource source,
                                    TextureStage::CombineOperand operand);
  static DWORD get_texture_argument_modifier(TextureStage::CombineOperand operand);

  static HRESULT CALLBACK enum_tex_formats_callback(LPDDPIXELFORMAT pddpf, void *param);

protected:
  DXScreenData *_screen;
  LPDIRECT3DDEVICE7 _d3d_device;  // same as _screen->_d3d_device, cached for spd

  bool _dx_is_ready;
  HRESULT _last_testcooplevel_result;

  bool _vertex_blending_enabled;

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation
  bool _auto_rescale_normal;

  D3DCOLOR _d3dcolor_clear_value;
  UINT _color_writemask;

  float _material_ambient;
  float _material_diffuse;
  float _material_specular;
  float _material_shininess;
  float _material_emission;

  enum DxgsgFogType {
    None,
    PerVertexFog=D3DRENDERSTATE_FOGVERTEXMODE,
    PerPixelFog=D3DRENDERSTATE_FOGTABLEMODE
  };
  DxgsgFogType _do_fog_type;

  DWORD _clip_plane_bits;
  CullFaceAttrib::Mode _cull_face_mode;
  RenderModeAttrib::Mode _current_fill_mode;  //point/wireframe/solid

  LMatrix4f _projection_mat;

  CPT(DisplayRegion) _actual_display_region;
  const DXVertexBufferContext7 *_active_vbuffer;
  const DXIndexBufferContext7 *_active_ibuffer;

  bool _tex_stats_retrieval_impossible;

  int _fvf;
  LPDDPIXELFORMAT _tex_formats;
  int _num_tex_formats;

  static D3DMATRIX _d3d_ident_mat;

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
    register_type(_type_handle, "DXGraphicsStateGuardian7",
                  GraphicsStateGuardian::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class wdxGraphicsWindow7;
  friend class wdxGraphicsPipe7;
  friend class wdxGraphicsWindowGroup7;
  friend class DXTextureContext7;
};

#include "dxGraphicsStateGuardian7.I"

#endif

