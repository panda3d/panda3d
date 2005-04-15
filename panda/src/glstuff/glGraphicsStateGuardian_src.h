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
#include "qpgeomVertexColumn.h"
#include "texture.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "textureAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "textureStage.h"
#include "textureApplyAttrib.h"
#include "antialiasAttrib.h"
#include "renderModeAttrib.h"
#include "pointerToArray.h"
#include "fog.h"
#include "graphicsWindow.h"
#include "pset.h"
#include "pmap.h"
#include "qpgeomVertexArrayData.h"
#include "pmutex.h"
#ifdef HAVE_CGGL
#include "cgShader.h"
#endif

class PlaneNode;
class Light;

// These typedefs are declared in glext.h, but we must repeat them
// here, mainly because they will not be included from glext.h if the
// system GL version matches or exceeds the GL version in which these
// functions are defined, and the system gl.h sometimes doesn't
// declare these typedefs.
typedef void (APIENTRYP PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

class CLP(GeomContext);

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

  INLINE bool draw_display_list(GeomContext *gc);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);
  void record_deleted_display_list(GLuint index);

  virtual VertexBufferContext *prepare_vertex_buffer(qpGeomVertexArrayData *data);
  void apply_vertex_buffer(VertexBufferContext *vbc);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);
  const unsigned char *setup_array_data(const qpGeomVertexArrayData *data);

  virtual IndexBufferContext *prepare_index_buffer(qpGeomPrimitive *data);
  void apply_index_buffer(IndexBufferContext *ibc);
  virtual void release_index_buffer(IndexBufferContext *ibc);
  const unsigned char *setup_primitive(const qpGeomPrimitive *data);

  virtual CPT(qpGeomMunger) get_geom_munger(const RenderState *state);

  virtual void framebuffer_copy_to_texture
    (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram
    (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);

  virtual void apply_material(const Material *material);
  void apply_fog(Fog *fog);

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_tex_matrix(const TexMatrixAttrib *attrib);
  virtual void issue_material(const MaterialAttrib *attrib);
  virtual void issue_render_mode(const RenderModeAttrib *attrib);
  virtual void issue_antialias(const AntialiasAttrib *);
  virtual void issue_rescale_normal(const RescaleNormalAttrib *attrib);
  virtual void issue_texture_apply(const TextureApplyAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_depth_test(const DepthTestAttrib *attrib);
  virtual void issue_alpha_test(const AlphaTestAttrib *attrib);
  virtual void issue_depth_write(const DepthWriteAttrib *attrib);
  virtual void issue_cull_face(const CullFaceAttrib *attrib);
  virtual void issue_fog(const FogAttrib *attrib);
  virtual void issue_depth_offset(const DepthOffsetAttrib *attrib);
  virtual void issue_tex_gen(const TexGenAttrib *attrib);
  virtual void issue_shade_model(const ShadeModelAttrib *attrib);
#ifdef HAVE_CGGL
  virtual void issue_cg_shader_bind(const CgShaderAttrib *attrib);
#endif

  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light, 
                          int light_id);

  virtual bool wants_texcoords(void) const;

  virtual float compute_distance_to(const LPoint3f &point) const;

  void print_gfx_visual();

  //For those interested in what the guardian thinks is the current
  //enabled/disable GL State compared to what GL says it is
  void dump_state(void);

  void issue_scaled_color(const Colorf &color) const;

  INLINE static bool report_errors(int line, const char *source_file);
  INLINE void report_my_errors(int line, const char *source_file);

protected:
  static bool report_errors_loop(int line, const char *source_file, 
                                 GLenum error_code, int &error_count);
  void show_gl_string(const string &name, GLenum id);
  virtual void get_gl_version();
  void save_extensions(const char *extensions);
  virtual void get_extra_extensions();
  void report_extensions() const;
  bool has_extension(const string &extension) const;
  bool is_at_least_version(int major_version, int minor_version, int release_version = 0) const;
  virtual void *get_extension_func(const char *prefix, const char *name);

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

  virtual void set_blend_mode();

  virtual void finish_modify_state();

  virtual void free_pointers();

  INLINE void enable_multisample_antialias(bool val);
  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_line_smooth(bool val);
  INLINE void enable_point_smooth(bool val);
  INLINE void enable_polygon_smooth(bool val);
  INLINE void setup_antialias_line();
  INLINE void setup_antialias_point();
  INLINE void setup_antialias_polygon();

  INLINE void enable_scissor(bool val);
  INLINE void enable_stencil_test(bool val);
  INLINE void enable_blend(bool val);
  INLINE void enable_depth_test(bool val);
  INLINE void enable_fog(bool val);
  INLINE void enable_alpha_test(bool val);
  INLINE void enable_polygon_offset(bool val);

  INLINE GLenum get_light_id(int index) const;
  INLINE GLenum get_clip_plane_id(int index) const;

  INLINE void issue_scene_graph_color();
  INLINE void issue_flat_shading(Geom *geom);

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  void bind_texture(TextureContext *tc);
  void specify_texture(Texture *tex);
  bool apply_texture_immediate(CLP(TextureContext) *gtc, Texture *tex);
  bool upload_texture_image(CLP(TextureContext) *gtc, bool uses_mipmaps, 
                            GLenum target, GLint internal_format, 
                            int width, int height, int depth,
                            GLint external_format, GLenum component_type, 
                            const unsigned char *image);

  static GLenum get_numeric_type(qpGeom::NumericType numeric_type);
  GLenum get_texture_target(Texture::TextureType texture_type) const;
  GLenum get_texture_wrap_mode(Texture::WrapMode wm);
  static GLenum get_texture_filter_type(Texture::FilterType ft, bool ignore_mipmaps);
  static GLenum get_component_type(Texture::ComponentType component_type);
  GLint get_external_image_format(Texture::Format format) const;
  static GLint get_internal_image_format(Texture::Format format);
  static GLint get_texture_apply_mode_type(TextureStage::Mode am);
  static GLint get_texture_combine_type(TextureStage::CombineMode cm);
  static GLint get_texture_src_type(TextureStage::CombineSource cs);
  static GLint get_texture_operand_type(TextureStage::CombineOperand co);
  static GLenum get_fog_mode_type(Fog::Mode m);
  static GLenum get_blend_equation_type(ColorBlendAttrib::Mode mode);
  static GLenum get_blend_func(ColorBlendAttrib::Operand operand);
  static GLenum get_usage(qpGeom::UsageHint usage_hint);

  static CPT(RenderState) get_untextured_state();
  static CPT(RenderState) get_smooth_state();
  static CPT(RenderState) get_flat_state();

  void do_auto_rescale_normal();
  void do_issue_texture();
  void do_point_size();

#ifndef NDEBUG
  void build_phony_mipmaps(Texture *tex);
  void build_phony_mipmap_level(int level, int x_size, int y_size);
  void save_mipmap_images(Texture *tex);
#endif

  enum AutoAntialiasMode {
    AA_poly,
    AA_line,
    AA_point,
  };

  enum MultisampleMode {
    MM_antialias  = 0x0001,
    MM_alpha_one  = 0x0002,
    MM_alpha_mask = 0x0004,
  };

  int _multisample_mode;
  bool _line_smooth_enabled;
  bool _point_smooth_enabled;
  bool _polygon_smooth_enabled;
  bool _scissor_enabled;
  bool _stencil_test_enabled;
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _fog_enabled;
  bool _alpha_test_enabled;
  bool _polygon_offset_enabled;
  bool _flat_shade_model;
  int _decal_level;

  bool _dithering_enabled;
  bool _texgen_forced_normal;

  int _max_lights;
  int _max_clip_planes;

  LMatrix4f _projection_mat;
  int _viewport_width;
  int _viewport_height;
  CPT(TextureAttrib) _current_texture;
  CPT(TexMatrixAttrib) _current_tex_mat;
  bool _needs_tex_mat;
  CPT(TexGenAttrib) _current_tex_gen;
  bool _needs_tex_gen;
  bool _tex_gen_modifies_mat;
  bool _auto_antialias_mode;
  RenderModeAttrib::Mode _render_mode;
  float _point_size;
  bool _point_perspective;

  bool _transform_stale;
  bool _vertex_blending_enabled;

  CPT(DisplayRegion) _actual_display_region;
#ifdef HAVE_CGGL
  PT(CgShader) _cg_shader; // The current CgShader object
  typedef pmap< PT(CgShader), PT(CLP(CgShaderContext)) > CGSHADERCONTEXTS;
  CGSHADERCONTEXTS _gl_cg_shader_contexts;// Associate CgShader with GLCgShaderContext
#endif

  int _pass_number;
  bool _auto_rescale_normal;
  GLuint _geom_display_list;
  int _last_max_stage_index;
  
  int _error_count;

  int _gl_version_major, _gl_version_minor, _gl_version_release;
  pset<string> _extensions;

public:
  bool _supports_point_parameters;
  PFNGLPOINTPARAMETERFVPROC _glPointParameterfv;

  bool _supports_point_sprite;

  bool _supports_vertex_blend;
  PFNGLWEIGHTPOINTERARBPROC _glWeightPointerARB;
  PFNGLVERTEXBLENDARBPROC _glVertexBlendARB;

  bool _supports_matrix_palette;
  PFNGLCURRENTPALETTEMATRIXARBPROC _glCurrentPaletteMatrixARB;
  PFNGLMATRIXINDEXPOINTERARBPROC _glMatrixIndexPointerARB;

  bool _supports_draw_range_elements;
  PFNGLDRAWRANGEELEMENTSPROC _glDrawRangeElements;

  bool _supports_3d_texture;
  PFNGLTEXIMAGE3DPROC _glTexImage3D;
  PFNGLTEXSUBIMAGE3DPROC _glTexSubImage3D;

  bool _supports_cube_map;

  bool _supports_bgr;
  bool _supports_rescale_normal;

  bool _supports_multitexture;
  PFNGLACTIVETEXTUREPROC _glActiveTexture;
  PFNGLCLIENTACTIVETEXTUREPROC _glClientActiveTexture;
  PFNGLMULTITEXCOORD2FVPROC _glMultiTexCoord2fv;

  bool _supports_buffers;
  PFNGLGENBUFFERSPROC _glGenBuffers;
  PFNGLBINDBUFFERPROC _glBindBuffer;
  PFNGLBUFFERDATAPROC _glBufferData;
  PFNGLBUFFERSUBDATAPROC _glBufferSubData;
  PFNGLDELETEBUFFERSPROC _glDeleteBuffers;

  PFNGLBLENDEQUATIONPROC _glBlendEquation;
  PFNGLBLENDCOLORPROC _glBlendColor;

  GLenum _edge_clamp;
  GLenum _border_clamp;
  GLenum _mirror_repeat;
  GLenum _mirror_clamp;
  GLenum _mirror_edge_clamp;
  GLenum _mirror_border_clamp;

  Mutex _lock;
  typedef pvector<GLuint> DeletedDisplayLists;
  DeletedDisplayLists _deleted_display_lists;

  static PStatCollector _vertices_display_list_pcollector;

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
    register_type(_type_handle, CLASSPREFIX_QUOTED "GraphicsStateGuardian",
                  GraphicsStateGuardian::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "glGraphicsStateGuardian_src.I"


