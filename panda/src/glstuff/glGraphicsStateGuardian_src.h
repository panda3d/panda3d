// Filename: glGraphicsStateGuardian_src.h
// Created by:  drose (02Feb99)
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

#include "pandabase.h"

#include "graphicsStateGuardian.h"
#include "geomprimitives.h"
#include "texture.h"
#include "pixelBuffer.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "textureApplyAttrib.h"
#include "pointerToArray.h"
#include "fog.h"
#include "graphicsWindow.h"
#include "pset.h"

class PlaneNode;
class Light;

////////////////////////////////////////////////////////////////////
//       Class : GLGraphicsStateGuardian
// Description : A GraphicsStateGuardian specialized for rendering
//               into OpenGL contexts.  There should be no GL calls
//               outside of this object.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GraphicsStateGuardian) : public GraphicsStateGuardian {
public:
  CLP(GraphicsStateGuardian)(const FrameBufferProperties &properties);
  virtual ~CLP(GraphicsStateGuardian)();

  virtual void reset();

  virtual void do_clear(const RenderBuffer &buffer);

  virtual void prepare_display_region();
  virtual bool prepare_lens();

  virtual bool begin_frame();
  virtual void end_frame();

  virtual void draw_point(GeomPoint *geom, GeomContext *gc);
  virtual void draw_line(GeomLine *geom, GeomContext *gc);
  virtual void draw_linestrip(GeomLinestrip *geom, GeomContext *gc);
  virtual void draw_sprite(GeomSprite *geom, GeomContext *gc);
  virtual void draw_polygon(GeomPolygon *geom, GeomContext *gc);
  virtual void draw_quad(GeomQuad *geom, GeomContext *gc);
  virtual void draw_tri(GeomTri *geom, GeomContext *gc);
  virtual void draw_tristrip(GeomTristrip *geom, GeomContext *gc);
  virtual void draw_trifan(GeomTrifan *geom, GeomContext *gc);
  virtual void draw_sphere(GeomSphere *geom, GeomContext *gc);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual void copy_texture(Texture *tex, const DisplayRegion *dr);
  virtual void copy_texture(Texture *tex, const DisplayRegion *dr,
                            const RenderBuffer &rb);

  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb);
  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                                       const DisplayRegion *dr);

  virtual bool copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr);
  virtual bool copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb);

  virtual void apply_material(const Material *material);
  void apply_fog(Fog *fog);

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_tex_matrix(const TexMatrixAttrib *attrib);
  virtual void issue_texture(const TextureAttrib *attrib);
  virtual void issue_material(const MaterialAttrib *attrib);
  virtual void issue_render_mode(const RenderModeAttrib *attrib);
  virtual void issue_texture_apply(const TextureApplyAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_depth_test(const DepthTestAttrib *attrib);
  virtual void issue_alpha_test(const AlphaTestAttrib *attrib);
  virtual void issue_depth_write(const DepthWriteAttrib *attrib);
  virtual void issue_cull_face(const CullFaceAttrib *attrib);
  virtual void issue_fog(const FogAttrib *attrib);
  virtual void issue_depth_offset(const DepthOffsetAttrib *attrib);
  //  virtual void issue_tex_gen(const TexGenAttrib *attrib);
  //  virtual void issue_stencil(const StencilAttrib *attrib);

  virtual void bind_light(PointLight *light, int light_id);
  virtual void bind_light(DirectionalLight *light, int light_id);
  virtual void bind_light(Spotlight *light, int light_id);

  virtual bool wants_normals(void) const;
  virtual bool wants_texcoords(void) const;

  virtual bool depth_offset_decals();

  virtual CoordinateSystem get_internal_coordinate_system() const;
  virtual float compute_distance_to(const LPoint3f &point) const;

  void print_gfx_visual();

  //For those interested in what the guardian thinks is the current
  //enabled/disable GL State compared to what GL says it is
  void dump_state(void);

  void issue_transformed_color(const Colorf &color) const;

  INLINE static bool report_errors(int line, const char *source_file);
  INLINE void report_my_errors(int line, const char *source_file);

protected:
  static bool report_errors_loop(int line, const char *source_file, 
                                 GLenum error_code, int &error_count);
  void show_gl_string(const string &name, GLenum id);
  void get_gl_version();
  void save_extensions(const char *extensions);
  virtual void get_extra_extensions();
  void report_extensions() const;
  bool has_extension(const string &extension) const;
  bool is_at_least_version(int major_version, int minor_version, int release_version = 0) const;

  virtual bool slot_new_light(int light_id);
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  virtual bool slot_new_clip_plane(int plane_id);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(PlaneNode *plane, int plane_id);
  virtual void end_bind_clip_planes();

  virtual void set_blend_mode(ColorWriteAttrib::Mode color_write_mode,
                              ColorBlendAttrib::Mode color_blend_mode,
                              TransparencyAttrib::Mode transparency_mode);

  virtual void free_pointers();
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                                                 CPT(DisplayRegion) dr);
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer);

  INLINE void call_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                                GLclampf alpha);
  INLINE void call_glClearDepth(GLclampd depth);
  INLINE void call_glClearStencil(GLint s);
  INLINE void call_glClearAccum(GLclampf red, GLclampf green, GLclampf blue,
                                GLclampf alpha);
  INLINE void call_glShadeModel(GLenum mode);
  INLINE void call_glBlendFunc(GLenum sfunc, GLenum dfunc);
  INLINE void call_glCullFace(GLenum mode);
  INLINE void call_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
  INLINE void call_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
  INLINE void call_glLightModelLocal(GLboolean local);
  INLINE void call_glLightModelTwoSide(GLboolean twoside);
  INLINE void call_glStencilFunc(GLenum func,GLint refval,GLuint mask);
  INLINE void call_glStencilOp(GLenum fail,GLenum zfail,GLenum pass);
  INLINE void call_glClipPlane(GLenum plane, const double equation[4]);
  INLINE void call_glLineWidth(GLfloat width);
  INLINE void call_glPointSize(GLfloat size);
  INLINE void call_glFogMode(GLint mode);
  INLINE void call_glFogStart(GLfloat start);
  INLINE void call_glFogEnd(GLfloat end);
  INLINE void call_glFogDensity(GLfloat density);
  INLINE void call_glFogColor(const Colorf &color);
  INLINE void call_glAlphaFunc(GLenum func, GLclampf ref);
  INLINE void call_glPolygonMode(GLenum mode);

  INLINE void set_pack_alignment(GLint alignment);
  INLINE void set_unpack_alignment(GLint alignment);

  INLINE void enable_multisample(bool val);
  INLINE void enable_line_smooth(bool val);
  INLINE void enable_point_smooth(bool val);
  INLINE void enable_texturing(bool val);
  INLINE void enable_scissor(bool val);
  INLINE void enable_stencil_test(bool val);
  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_blend(bool val);
  INLINE void enable_depth_test(bool val);
  INLINE void enable_fog(bool val);
  INLINE void enable_alpha_test(bool val);
  INLINE void enable_polygon_offset(bool val);

  INLINE GLenum get_light_id(int index) const;
  INLINE GLenum get_clip_plane_id(int index) const;

  INLINE void issue_scene_graph_color();

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  void bind_texture(TextureContext *tc);
  void specify_texture(Texture *tex);
  bool apply_texture_immediate(Texture *tex);

  void draw_texture(TextureContext *tc, const DisplayRegion *dr);
  void draw_texture(TextureContext *tc, const DisplayRegion *dr, 
                    const RenderBuffer &rb);
  void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr);
  void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                         const RenderBuffer &rb);

  GLenum get_texture_wrap_mode(Texture::WrapMode wm);
  GLenum get_texture_filter_type(Texture::FilterType ft);
  GLenum get_image_type(PixelBuffer::Type type);
  GLenum get_external_image_format(PixelBuffer::Format format);
  GLenum get_internal_image_format(PixelBuffer::Format format);
  GLint get_texture_apply_mode_type(TextureApplyAttrib::Mode am) const;
  GLenum get_fog_mode_type(Fog::Mode m) const;

  static CPT(RenderState) get_untextured_state();

#ifndef NDEBUG
  void build_phony_mipmaps(Texture *tex);
  void build_phony_mipmap_level(int level, int xsize, int ysize);
  void save_mipmap_images(Texture *tex);
#endif

  GLclampf _clear_color_red, _clear_color_green, _clear_color_blue,
    _clear_color_alpha;
  GLclampd _clear_depth;
  GLint _clear_stencil;
  GLclampf _clear_accum_red, _clear_accum_green, _clear_accum_blue,
    _clear_accum_alpha;
  GLenum _shade_model_mode;
  GLint _scissor_x;
  GLint _scissor_y;
  GLsizei _scissor_width;
  GLsizei _scissor_height;
  GLint _viewport_x;
  GLint _viewport_y;
  GLsizei _viewport_width;
  GLsizei _viewport_height;
  GLboolean _lmodel_local;
  GLboolean _lmodel_twoside;
  GLfloat _line_width;
  GLfloat _point_size;
  GLenum _blend_source_func;
  GLenum _blend_dest_func;
  GLboolean _depth_mask;
  GLint _fog_mode;
  GLfloat _fog_start;
  GLfloat _fog_end;
  GLfloat _fog_density;
  Colorf _fog_color;
  GLenum _alpha_func;
  GLclampf _alpha_func_ref;
  GLenum _polygon_mode;

  GLint _pack_alignment;
  GLint _unpack_alignment;

  bool _multisample_enabled;
  bool _line_smooth_enabled;
  bool _point_smooth_enabled;
  bool _scissor_enabled;
  bool _texturing_enabled;
  bool _stencil_test_enabled;
  bool _multisample_alpha_one_enabled;
  bool _multisample_alpha_mask_enabled;
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _fog_enabled;
  bool _alpha_test_enabled;
  bool _polygon_offset_enabled;
  int _decal_level;

  bool _dithering_enabled;

  int _max_lights;
  int _max_clip_planes;

  LMatrix4f _current_projection_mat;
  int _projection_mat_stack_count;

  CPT(DisplayRegion) _actual_display_region;

  int _pass_number;

  int _gl_version_major, _gl_version_minor, _gl_version_release;
  pset<string> _extensions;
  bool _supports_bgr;
  bool _supports_multisample;
  GLenum _edge_clamp;

  int _error_count;

public:
  static GraphicsStateGuardian *
  make_GlGraphicsStateGuardian(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static PStatCollector _vertices_display_list_pcollector;

private:
  static TypeHandle _type_handle;
};

#include "glGraphicsStateGuardian_src.I"
