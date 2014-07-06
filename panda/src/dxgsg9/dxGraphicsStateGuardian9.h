// Filename: dxGraphicsStateGuardian9.h
// Created by:  mike (02Feb99)
// Updated by: fperazzi, PandaSE (05May10) (added get_supports_cg_profile)
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

#ifndef DXGRAPHICSSTATEGUARDIAN9_H
#define DXGRAPHICSSTATEGUARDIAN9_H

#include "dxgsg9base.h"
#include "dxTextureContext9.h"
#include "config_dxgsg9.h"

#include "graphicsStateGuardian.h"
#include "texture.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "cullFaceAttrib.h"
#include "renderModeAttrib.h"
#include "colorBlendAttrib.h"
#include "fog.h"
#include "pointerToArray.h"

#include "lru.h"

#include "vertexElementArray.h"
#include "dxShaderContext9.h"


enum GsgPageType
{
  GPT_Texture,
  GPT_VertexBuffer,
  GPT_IndexBuffer,

  GPT_TotalPageTypes
};

class Light;

class DXTextureContext9;
class DXVertexBufferContext9;
class DXIndexBufferContext9;

////////////////////////////////////////////////////////////////////
//       Class : DXGraphicsStateGuardian9
// Description : A GraphicsStateGuardian for rendering into DirectX9
//               contexts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsStateGuardian9 : public GraphicsStateGuardian {
public:
  DXGraphicsStateGuardian9(GraphicsEngine *engine, GraphicsPipe *pipe);
  ~DXGraphicsStateGuardian9();

  FrameBufferProperties
    calc_fb_properties(DWORD cformat, DWORD dformat,
                       DWORD multisampletype, DWORD multisamplequality);

  virtual TextureContext *prepare_texture(Texture *tex, int view);
  void apply_texture(int i, TextureContext *tc);
  virtual bool update_texture(TextureContext *tc, bool force);
  bool upload_texture(DXTextureContext9 *dtc, bool force);
  virtual void release_texture(TextureContext *tc);
  virtual bool extract_texture_data(Texture *tex);

  ShaderContext *prepare_shader(Shader *se);
  void release_shader(ShaderContext *sc);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  bool apply_vertex_buffer(VertexBufferContext *vbc,
                           const GeomVertexArrayDataHandle *reader,
                           bool force);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  bool setup_array_data(CLP(VertexBufferContext)*& vbc,
                        const GeomVertexArrayDataHandle* data,
                        bool force);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  bool apply_index_buffer(IndexBufferContext *ibc,
                          const GeomPrimitivePipelineReader *reader, bool force);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual void begin_occlusion_query();
  virtual PT(OcclusionQueryContext) end_occlusion_query();

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void clear(DrawableRegion *clearable);

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr);
  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomMunger *munger,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  virtual bool framebuffer_copy_to_texture(Texture *tex, int view, int z,
                                           const DisplayRegion *dr,
                                           const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram(Texture *tex, int view, int z,
                                       const DisplayRegion *dr,
                                       const RenderBuffer &rb);
  bool do_framebuffer_copy_to_ram(Texture *tex, int view, int z,
                                  const DisplayRegion *dr,
                                  const RenderBuffer &rb,
                                  bool inverted);

  void reset_render_states (void);
  virtual void reset();

  virtual void apply_fog(Fog *fog);

  virtual void bind_light(PointLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

  static D3DFORMAT get_index_type(Geom::NumericType numeric_type);
  INLINE static DWORD LColor_to_D3DCOLOR(const LColor &cLColor);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  bool check_dx_allocation (HRESULT result, int allocation_size, int attempts);

  INLINE HRESULT set_render_state (D3DRENDERSTATETYPE state, DWORD value);
  INLINE HRESULT set_texture_stage_state (DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
  INLINE HRESULT set_sampler_state (DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

  INLINE bool get_supports_render_texture() const;

  static bool get_gamma_table(void);
  static bool static_set_gamma(bool restore, PN_stdfloat gamma);
  bool set_gamma(PN_stdfloat gamma);
  void restore_gamma();
  static void atexit_function(void);

  static void set_cg_device(LPDIRECT3DDEVICE9 cg_device);
  virtual bool get_supports_cg_profile(const string &name) const;


protected:
  void do_issue_transform();
  void do_issue_alpha_test();
  void do_issue_shader();
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
  void do_issue_stencil();
  void do_issue_scissor();

  virtual void reissue_transforms();

  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const LColor &color);
  virtual void enable_light(int light_id, bool enable);

  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);

  virtual void close_gsg();
  void free_nondx_resources();
  void free_d3d_device();

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  void do_auto_rescale_normal();

  void disable_standard_vertex_arrays();
  bool update_standard_vertex_arrays(bool force);
  void disable_standard_texture_bindings();
  void update_standard_texture_bindings();

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
  HRESULT reset_d3d_device(D3DPRESENT_PARAMETERS *p_presentation_params,
                           DXScreenData **screen = NULL);

  bool check_cooperative_level();

  void show_frame();

  bool create_swap_chain (DXScreenData *new_context);
  bool release_swap_chain (DXScreenData *new_context);
  void copy_pres_reset(DXScreenData *new_context);

  static D3DTEXTUREFILTERTYPE get_d3d_min_type(Texture::FilterType filter_type);
  static D3DTEXTUREFILTERTYPE get_d3d_mip_type(Texture::FilterType filter_type);
  static D3DTEXTUREOP get_texture_operation(TextureStage::CombineMode mode, int scale);
  DWORD get_texture_argument(TextureStage::CombineSource source,
           TextureStage::CombineOperand operand) const;
  static DWORD get_texture_argument_modifier(TextureStage::CombineOperand operand);

  void draw_primitive_up(D3DPRIMITIVETYPE primitive_type,
       unsigned int primitive_count,
       unsigned int first_vertex,
       unsigned int num_vertices,
       const unsigned char *buffer, size_t stride);
  void draw_indexed_primitive_up(D3DPRIMITIVETYPE primitive_type,
         unsigned int min_index, unsigned int max_index,
         unsigned int num_primitives,
         const unsigned char *index_data,
         D3DFORMAT index_type,
         const unsigned char *buffer, size_t stride);

  INLINE static unsigned char *get_safe_buffer_start();

public:
  DXScreenData *_screen;
  
protected:
  LPDIRECT3DDEVICE9 _d3d_device;  // same as _screen->_d3d_device, cached for spd
  IDirect3DSwapChain9 *_swap_chain;
  D3DPRESENT_PARAMETERS _presentation_reset;  // This is built during reset device

  bool _dx_is_ready;
  HRESULT _last_testcooplevel_result;

  bool _vertex_blending_enabled;
  bool _supports_render_texture;

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation
  bool _auto_rescale_normal;

  PN_stdfloat _material_ambient;
  PN_stdfloat _material_diffuse;
  PN_stdfloat _material_specular;
  PN_stdfloat _material_shininess;
  PN_stdfloat _material_emission;

  enum DxgsgFogType {
    None,
    PerVertexFog=D3DRS_FOGVERTEXMODE,
    PerPixelFog=D3DRS_FOGTABLEMODE
  };
  DxgsgFogType _do_fog_type;

  D3DVIEWPORT9 _current_viewport;
  bool _supports_depth_bias;

  DWORD _clip_plane_bits;
  CullFaceAttrib::Mode _cull_face_mode;
  RenderModeAttrib::Mode _current_fill_mode;  //point/wireframe/solid

  PT(Shader)  _current_shader;
  CLP(ShaderContext)  *_current_shader_context;
  PT(Shader)  _vertex_array_shader;
  CLP(ShaderContext)  *_vertex_array_shader_context;
  PT(Shader)  _texture_binding_shader;
  CLP(ShaderContext)  *_texture_binding_shader_context;

  const DXIndexBufferContext9 *_active_ibuffer;

  bool _overlay_windows_supported;
  bool _tex_stats_retrieval_impossible;
  bool _supports_texture_constant_color;
  DWORD _constant_color_operand;

  static D3DMATRIX _d3d_ident_mat;

  static unsigned char *_temp_buffer;
  static unsigned char *_safe_buffer_start;

  int _gsg_managed_textures;
  int _gsg_managed_vertex_buffers;
  int _gsg_managed_index_buffers;
  UINT _available_texture_memory;

  DWORD _last_fvf;
  int _num_bound_streams;

  // Cache the data necessary to bind each particular light each
  // frame, so if we bind a given light multiple times, we only have
  // to compute its data once.
  typedef pmap<NodePath, D3DLIGHT9> DirectionalLights;
  DirectionalLights _dlights;

  #define MAXIMUM_TEXTURES 16


  // from D3DRENDERSTATETYPE + pad
  #define MAXIMUM_RENDER_STATES 256

  // from D3DTEXTURESTAGESTATETYPE + pad
  #define MAXIMUM_TEXTURE_STAGE_STATES 40
  typedef struct {
    DWORD state_array [MAXIMUM_TEXTURE_STAGE_STATES];
  }
  TextureStageStates;

  // from D3DSAMPLERSTATETYPE + pad
  #define MAXIMUM_TEXTURE_RENDER_STATES 16
  typedef struct {
    DWORD state_array [MAXIMUM_TEXTURE_RENDER_STATES];
  }
  TextureRenderStates;

  // from D3DRENDERSTATETYPE
  DWORD _render_state_array [MAXIMUM_RENDER_STATES];
  TextureStageStates _texture_stage_states_array [D3D_MAXTEXTURESTAGES];
  TextureRenderStates _texture_render_states_array [MAXIMUM_TEXTURES];

  int _num_active_texture_stages;

  int _vertex_shader_version_major;
  int _vertex_shader_version_minor;
  int _pixel_shader_version_major;
  int _pixel_shader_version_minor;

  char *_vertex_shader_profile;
  char *_pixel_shader_profile;

  int _vertex_shader_maximum_constants;

  bool _supports_stream_offset;

  list <wdxGraphicsBuffer9 **> _graphics_buffer_list;

  int _supports_gamma_calibration;  

  static LPDIRECT3DDEVICE9 _cg_device;

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
    register_type(_type_handle, "DXGraphicsStateGuardian9",
                  GraphicsStateGuardian::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class wdxGraphicsWindow9;
  friend class wdxGraphicsPipe9;
  friend class wdxGraphicsWindowGroup9;
  friend class DXTextureContext9;
  friend class wdxGraphicsBuffer9;
  friend class DXVertexBufferContext9;
  friend class DXIndexBufferContext9;
  friend class DXShaderContext9;
};

#include "dxGraphicsStateGuardian9.I"

#endif
