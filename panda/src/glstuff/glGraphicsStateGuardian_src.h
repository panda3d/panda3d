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
#include "geomVertexColumn.h"
#include "texture.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "textureAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "shaderAttrib.h"
#include "textureStage.h"
#include "antialiasAttrib.h"
#include "renderModeAttrib.h"
#include "pointerToArray.h"
#include "fog.h"
#include "graphicsWindow.h"
#include "pset.h"
#include "pmap.h"
#include "geomVertexArrayData.h"
#include "pmutex.h"

class PlaneNode;
class Light;

// These typedefs are declared in glext.h, but we must repeat them
// here, mainly because they will not be included from glext.h if the
// system GL version matches or exceeds the GL version in which these
// functions are defined, and the system gl.h sometimes doesn't
// declare these typedefs.
#ifndef __EDG__  // Protect the following from the Tau instrumentor.
typedef void (APIENTRYP PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);
typedef void (APIENTRYP PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void (APIENTRYP PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (APIENTRYP PFNGLENDQUERYPROC) (GLenum target);
typedef void (APIENTRYP PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef void (APIENTRYP PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLMULTITEXCOORD1FPROC) (GLenum target, const GLfloat s);
typedef void (APIENTRYP PFNGLMULTITEXCOORD2FPROC) (GLenum target, const GLfloat s, const GLfloat t);
typedef void (APIENTRYP PFNGLMULTITEXCOORD3FPROC) (GLenum target, const GLfloat s, const GLfloat t, const GLfloat r);
typedef void (APIENTRYP PFNGLMULTITEXCOORD4FPROC) (GLenum target, const GLfloat s, const GLfloat t, const GLfloat r, const GLfloat q);
typedef void (APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLACTIVESTENCILFACEEXTPROC) (GLenum face);
#endif  // __EDG__

////////////////////////////////////////////////////////////////////
//       Class : GLGraphicsStateGuardian
// Description : A GraphicsStateGuardian specialized for rendering
//               into OpenGL contexts.  There should be no GL calls
//               outside of this object.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GraphicsStateGuardian) : public GraphicsStateGuardian {
public:
  CLP(GraphicsStateGuardian)(GraphicsPipe *pipe);
  virtual ~CLP(GraphicsStateGuardian)();

  virtual void reset();

  virtual void do_clear(const RenderBuffer &buffer);

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr,
                                      Lens::StereoChannel stereo_channel);
  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
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

  INLINE bool draw_display_list(GeomContext *gc);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void release_texture(TextureContext *tc);
  virtual bool extract_texture_data(Texture *tex);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(ShaderExpansion *shader);
  virtual void release_shader(ShaderContext *sc);

  void record_deleted_display_list(GLuint index);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  bool apply_vertex_buffer(VertexBufferContext *vbc,
                           const GeomVertexArrayDataHandle *reader,
                           bool force);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  bool setup_array_data(const unsigned char *&client_pointer,
                        const GeomVertexArrayDataHandle *data,
                        bool force);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  bool apply_index_buffer(IndexBufferContext *ibc,
                          const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual void release_index_buffer(IndexBufferContext *ibc);
  bool setup_primitive(const unsigned char *&client_pointer,
                       const GeomPrimitivePipelineReader *reader,
                       bool force);

  virtual void begin_occlusion_query();
  virtual PT(OcclusionQueryContext) end_occlusion_query();

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void framebuffer_copy_to_texture
    (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram
    (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);

  void apply_fog(Fog *fog);

  virtual void bind_light(PointLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

  void print_gfx_visual();

  //For those interested in what the guardian thinks is the current
  //enabled/disable GL State compared to what GL says it is
  void dump_state();

  const float *get_light_color(Light *light) const;

#ifdef SUPPORT_IMMEDIATE_MODE
  void draw_immediate_simple_primitives(const GeomPrimitivePipelineReader *reader, GLenum mode);
  void draw_immediate_composite_primitives(const GeomPrimitivePipelineReader *reader, GLenum mode);
#endif  // SUPPORT_IMMEDIATE_MODE

  INLINE static bool report_errors(int line, const char *source_file);
  INLINE void report_my_errors(int line, const char *source_file);

  INLINE const string &get_gl_vendor() const;
  INLINE const string &get_gl_renderer() const;
  INLINE int get_gl_version_major() const;
  INLINE int get_gl_version_minor() const;
  INLINE int get_gl_version_release() const;

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  void bind_fbo(GLuint fbo);

protected:
  void do_issue_transform();
  void do_issue_render_mode();
  void do_issue_antialias();
  void do_issue_rescale_normal();
  void do_issue_color_write();
  void do_issue_depth_test();
  void do_issue_alpha_test();
  void do_issue_depth_write();
  void do_issue_cull_face();
  void do_issue_fog();
  void do_issue_depth_offset();
  void do_issue_shade_model();
  void do_issue_shader();
  void do_issue_material();
  void do_issue_texture();
  void do_issue_blending();
  void do_issue_tex_gen();
  void do_issue_tex_matrix();
  void do_issue_stencil();

  virtual void gl_flush() const;
  virtual GLenum gl_get_error() const;

  static bool report_errors_loop(int line, const char *source_file,
                                 GLenum error_code, int &error_count);
  static string get_error_string(GLenum error_code);
  string show_gl_string(const string &name, GLenum id);
  virtual void query_gl_version();
  void save_extensions(const char *extensions);
  virtual void get_extra_extensions();
  void report_extensions() const;
  bool has_extension(const string &extension) const;
  bool is_at_least_version(int major_version, int minor_version, int release_version = 0) const;
  virtual void *get_extension_func(const char *prefix, const char *name);

  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);
  virtual void end_bind_clip_planes();

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

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  static GLenum get_numeric_type(Geom::NumericType numeric_type);
  GLenum get_texture_target(Texture::TextureType texture_type) const;
  GLenum get_texture_wrap_mode(Texture::WrapMode wm) const;
  static Texture::WrapMode get_panda_wrap_mode(GLenum wm);
  static GLenum get_texture_filter_type(Texture::FilterType ft,
                             Texture::Format fmt, bool ignore_mipmaps);
  static Texture::FilterType get_panda_filter_type(GLenum ft);
  static GLenum get_component_type(Texture::ComponentType component_type);
  GLint get_external_image_format(Texture *tex) const;
  GLint get_internal_image_format(Texture *tex) const;
  static bool is_mipmap_filter(GLenum min_filter);
  static bool is_compressed_format(GLenum format);
  static GLint get_texture_apply_mode_type(TextureStage::Mode am);
  static GLint get_texture_combine_type(TextureStage::CombineMode cm);
  GLint get_texture_src_type(TextureStage::CombineSource cs,
                             int last_stage, int last_saved_result,
                             int this_stage) const;
  static GLint get_texture_operand_type(TextureStage::CombineOperand co);
  static GLenum get_fog_mode_type(Fog::Mode m);
  static GLenum get_blend_equation_type(ColorBlendAttrib::Mode mode);
  static GLenum get_blend_func(ColorBlendAttrib::Operand operand);
  static GLenum get_usage(Geom::UsageHint usage_hint);

  void disable_standard_vertex_arrays();
  bool update_standard_vertex_arrays(bool force);
  void disable_standard_texture_bindings();
  void update_standard_texture_bindings();

  void do_auto_rescale_normal();
  void specify_texture(Texture *tex);
  void apply_texture(TextureContext *tc);
  bool upload_texture(CLP(TextureContext) *gtc);
  bool upload_texture_image(CLP(TextureContext) *gtc,
                            bool uses_mipmaps, int mipmap_bias,
                            GLenum texture_target, GLenum page_target, 
                            GLint internal_format, GLint external_format, 
                            GLenum component_type,
                            bool one_page_only, int z,
          Texture::CompressionMode image_compression);

  size_t get_texture_memory_size(Texture *tex);
  void check_nonresident_texture(BufferContextChain &chain);
  bool extract_texture_image(PTA_uchar &image, size_t &page_size,
           Texture *tex, GLenum target, GLenum page_target,
           Texture::ComponentType type,
           Texture::CompressionMode compression, int n);

  void do_point_size();

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

  int _viewport_width;
  int _viewport_height;
  bool _auto_antialias_mode;
  RenderModeAttrib::Mode _render_mode;
  float _point_size;
  bool _point_perspective;
  bool _vertex_blending_enabled;

  PT(ShaderExpansion)  _current_shader_expansion;
  CLP(ShaderContext)  *_current_shader_context;
  PT(ShaderExpansion)  _vertex_array_shader_expansion;
  CLP(ShaderContext)  *_vertex_array_shader_context;
  PT(ShaderExpansion)  _texture_binding_shader_expansion;
  CLP(ShaderContext)  *_texture_binding_shader_context;

#ifdef SUPPORT_IMMEDIATE_MODE
  CLP(ImmediateModeSender) _sender;
  bool _use_sender;
#endif  // SUPPORT_IMMEDIATE_MODE

  int _pass_number;
  bool _auto_rescale_normal;
  GLuint _geom_display_list;
  GLuint _current_vbuffer_index;
  GLuint _current_ibuffer_index;
  GLuint _current_fbo;
  int _num_active_texture_stages;

  int _error_count;

  string _gl_vendor;
  string _gl_renderer;
  int _gl_version_major, _gl_version_minor, _gl_version_release;
  pset<string> _extensions;

public:
  bool _supports_point_parameters;
  PFNGLPOINTPARAMETERFVPROC _glPointParameterfv;

  bool _supports_point_sprite;

  bool _supports_vertex_blend;
  PFNGLWEIGHTPOINTERARBPROC _glWeightPointerARB;
  PFNGLVERTEXBLENDARBPROC _glVertexBlendARB;
  PFNGLWEIGHTFVARBPROC _glWeightfvARB;

  bool _supports_matrix_palette;
  PFNGLCURRENTPALETTEMATRIXARBPROC _glCurrentPaletteMatrixARB;
  PFNGLMATRIXINDEXPOINTERARBPROC _glMatrixIndexPointerARB;
  PFNGLMATRIXINDEXUIVARBPROC _glMatrixIndexuivARB;

  bool _supports_draw_range_elements;
  PFNGLDRAWRANGEELEMENTSPROC _glDrawRangeElements;

  PFNGLTEXIMAGE3DPROC _glTexImage3D;
  PFNGLTEXSUBIMAGE3DPROC _glTexSubImage3D;

  PFNGLCOMPRESSEDTEXIMAGE1DPROC _glCompressedTexImage1D;
  PFNGLCOMPRESSEDTEXIMAGE2DPROC _glCompressedTexImage2D;
  PFNGLCOMPRESSEDTEXIMAGE3DPROC _glCompressedTexImage3D;
  PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC _glCompressedTexSubImage1D;
  PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC _glCompressedTexSubImage2D;
  PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC _glCompressedTexSubImage3D;
  PFNGLGETCOMPRESSEDTEXIMAGEPROC _glGetCompressedTexImage;

  bool _supports_bgr;
  bool _supports_rescale_normal;

  bool _supports_multitexture;
  PFNGLACTIVETEXTUREPROC _glActiveTexture;
  PFNGLCLIENTACTIVETEXTUREPROC _glClientActiveTexture;
  PFNGLMULTITEXCOORD1FPROC _glMultiTexCoord1f;
  PFNGLMULTITEXCOORD2FPROC _glMultiTexCoord2f;
  PFNGLMULTITEXCOORD3FPROC _glMultiTexCoord3f;
  PFNGLMULTITEXCOORD4FPROC _glMultiTexCoord4f;

  bool _supports_buffers;
  PFNGLGENBUFFERSPROC _glGenBuffers;
  PFNGLBINDBUFFERPROC _glBindBuffer;
  PFNGLBUFFERDATAPROC _glBufferData;
  PFNGLBUFFERSUBDATAPROC _glBufferSubData;
  PFNGLDELETEBUFFERSPROC _glDeleteBuffers;

  PFNGLBLENDEQUATIONPROC _glBlendEquation;
  PFNGLBLENDCOLORPROC _glBlendColor;

  bool _supports_framebuffer_object;
  PFNGLISRENDERBUFFEREXTPROC _glIsRenderbuffer;
  PFNGLBINDRENDERBUFFEREXTPROC _glBindRenderbuffer;
  PFNGLDELETERENDERBUFFERSEXTPROC _glDeleteRenderbuffers;
  PFNGLGENRENDERBUFFERSEXTPROC _glGenRenderbuffers;
  PFNGLRENDERBUFFERSTORAGEEXTPROC _glRenderbufferStorage;
  PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC _glGetRenderbufferParameteriv;
  PFNGLISFRAMEBUFFEREXTPROC _glIsFramebuffer;
  PFNGLBINDFRAMEBUFFEREXTPROC _glBindFramebuffer;
  PFNGLDELETEFRAMEBUFFERSEXTPROC _glDeleteFramebuffers;
  PFNGLGENFRAMEBUFFERSEXTPROC _glGenFramebuffers;
  PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC _glCheckFramebufferStatus;
  PFNGLFRAMEBUFFERTEXTURE1DEXTPROC _glFramebufferTexture1D;
  PFNGLFRAMEBUFFERTEXTURE2DEXTPROC _glFramebufferTexture2D;
  PFNGLFRAMEBUFFERTEXTURE3DEXTPROC _glFramebufferTexture3D;
  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC _glFramebufferRenderbuffer;
  PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC _glGetFramebufferAttachmentParameteriv;
  PFNGLGENERATEMIPMAPEXTPROC _glGenerateMipmap;
  PFNGLBINDPROGRAMARBPROC _glBindProgram;

  PFNGLDRAWBUFFERSPROC _glDrawBuffers;

  PFNGLGENQUERIESPROC _glGenQueries;
  PFNGLBEGINQUERYPROC _glBeginQuery;
  PFNGLENDQUERYPROC _glEndQuery;
  PFNGLDELETEQUERIESPROC _glDeleteQueries;
  PFNGLGETQUERYIVPROC _glGetQueryiv;
  PFNGLGETQUERYOBJECTUIVPROC _glGetQueryObjectuiv;

  PFNGLACTIVESTENCILFACEEXTPROC _glActiveStencilFaceEXT;

  GLenum _edge_clamp;
  GLenum _border_clamp;
  GLenum _mirror_repeat;
  GLenum _mirror_clamp;
  GLenum _mirror_edge_clamp;
  GLenum _mirror_border_clamp;

  Mutex _lock;
  typedef pvector<GLuint> DeletedDisplayLists;
  DeletedDisplayLists _deleted_display_lists;
  DeletedDisplayLists _deleted_queries;

  static PStatCollector _load_display_list_pcollector;
  static PStatCollector _primitive_batches_display_list_pcollector;
  static PStatCollector _vertices_display_list_pcollector;
  static PStatCollector _vertices_immediate_pcollector;

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

  friend class CLP(ShaderContext);
  friend class CLP(GraphicsBuffer);
  friend class CLP(OcclusionQueryContext);
};

#include "glGraphicsStateGuardian_src.I"


