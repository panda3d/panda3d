// Filename: glGraphicsStateGuardian_src.cxx
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

#include "config_util.h"
#include "displayRegion.h"
#include "renderBuffer.h"
#include "geom.h"
#include "geomIssuer.h"
#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "lens.h"
#include "perspectiveLens.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "planeNode.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "alphaTestAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "colorWriteAttrib.h"
#include "texMatrixAttrib.h"
#include "materialAttrib.h"
#include "renderModeAttrib.h"
#include "fogAttrib.h"
#include "depthOffsetAttrib.h"
#include "fog.h"
#include "clockObject.h"
#include "string_utils.h"
#include "nodePath.h"
#include "dcast.h"
#include "pvector.h"
#include "vector_string.h"
#include "string_utils.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#endif

#include <algorithm>

TypeHandle CLP(GraphicsStateGuardian)::_type_handle;

#if !defined(CPPPARSER) && defined(DO_PSTATS)
PStatCollector CLP(GraphicsStateGuardian)::_vertices_display_list_pcollector("Vertices:Display lists");
#endif

static void
issue_vertex_gl(const Geom *geom, Geom::VertexIterator &viterator, 
                GraphicsStateGuardianBase *) {
  const Vertexf &vertex = geom->get_next_vertex(viterator);
  // GLCAT.spam() << "Issuing vertex " << vertex << "\n";
  GLP(Vertex3fv)(vertex.get_data());
}

static void
issue_normal_gl(const Geom *geom, Geom::NormalIterator &niterator, 
                GraphicsStateGuardianBase *) {
  const Normalf &normal = geom->get_next_normal(niterator);
  // GLCAT.spam() << "Issuing normal " << normal << "\n";
  GLP(Normal3fv)(normal.get_data());
}

static void
issue_texcoord_gl(const Geom *geom, Geom::TexCoordIterator &tciterator, 
                GraphicsStateGuardianBase *) {
  const TexCoordf &texcoord = geom->get_next_texcoord(tciterator);
  //  GLCAT.spam() << "Issuing texcoord " << texcoord << "\n";
  GLP(TexCoord2fv)(texcoord.get_data());
}

static void
issue_color_gl(const Geom *geom, Geom::ColorIterator &citerator,
               GraphicsStateGuardianBase *) {
  const Colorf &color = geom->get_next_color(citerator);
  //  GLCAT.spam() << "Issuing color " << color << "\n";
  GLP(Color4fv)(color.get_data());
}

static void
issue_transformed_color_gl(const Geom *geom, Geom::ColorIterator &citerator,
                           GraphicsStateGuardianBase *gsg) {
  const CLP(GraphicsStateGuardian) *glgsg = DCAST(CLP(GraphicsStateGuardian), gsg);
  const Colorf &color = geom->get_next_color(citerator);
  glgsg->issue_transformed_color(color);
}

////////////////////////////////////////////////////////////////////
//     Function: uchar_bgr_to_rgb
//  Description: Recopies the given array of pixels, converting from
//               BGR to RGB arrangement.
////////////////////////////////////////////////////////////////////
static void
uchar_bgr_to_rgb(unsigned char *dest, const unsigned char *source, 
                 int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[2];
    dest[1] = source[1];
    dest[2] = source[0];
    dest += 3;
    source += 3;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: uchar_bgra_to_rgba
//  Description: Recopies the given array of pixels, converting from
//               BGRA to RGBA arrangement.
////////////////////////////////////////////////////////////////////
static void
uchar_bgra_to_rgba(unsigned char *dest, const unsigned char *source, 
                   int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[2];
    dest[1] = source[1];
    dest[2] = source[0];
    dest[3] = source[3];
    dest += 4;
    source += 4;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ushort_bgr_to_rgb
//  Description: Recopies the given array of pixels, converting from
//               BGR to RGB arrangement.
////////////////////////////////////////////////////////////////////
static void
ushort_bgr_to_rgb(unsigned short *dest, const unsigned short *source, 
                  int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[2];
    dest[1] = source[1];
    dest[2] = source[0];
    dest += 3;
    source += 3;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ushort_bgra_to_rgba
//  Description: Recopies the given array of pixels, converting from
//               BGRA to RGBA arrangement.
////////////////////////////////////////////////////////////////////
static void
ushort_bgra_to_rgba(unsigned short *dest, const unsigned short *source, 
                    int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[2];
    dest[1] = source[1];
    dest[2] = source[0];
    dest[3] = source[3];
    dest += 4;
    source += 4;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: fix_component_ordering
//  Description: Reverses the order of the components within the
//               image, to convert (for instance) GL_BGR to GL_RGB.
//               Returns the PTA_uchar representing the converted
//               image, or the original image if it is unchanged.
////////////////////////////////////////////////////////////////////
static PTA_uchar
fix_component_ordering(GLenum external_format, PixelBuffer *pb) {
  PTA_uchar new_image = pb->_image;

  switch (external_format) {
  case GL_RGB:
    switch (pb->get_image_type()) {
    case PixelBuffer::T_unsigned_byte:
      new_image = PTA_uchar::empty_array(pb->_image.size());
      uchar_bgr_to_rgb(new_image, pb->_image, pb->_image.size() / 3);
      break;

    case PixelBuffer::T_unsigned_short:
      new_image = PTA_uchar::empty_array(pb->_image.size());
      ushort_bgr_to_rgb((unsigned short *)new_image.p(), 
                        (unsigned short *)pb->_image.p(), 
                        pb->_image.size() / 6);
      break;

    default:
      break;
    }
    break;

  case GL_RGBA:
    switch (pb->get_image_type()) {
    case PixelBuffer::T_unsigned_byte:
      new_image = PTA_uchar::empty_array(pb->_image.size());
      uchar_bgra_to_rgba(new_image, pb->_image, pb->_image.size() / 4);
      break;

    case PixelBuffer::T_unsigned_short:
      new_image = PTA_uchar::empty_array(pb->_image.size());
      ushort_bgra_to_rgba((unsigned short *)new_image.p(), 
                          (unsigned short *)pb->_image.p(), 
                          pb->_image.size() / 8);
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }

  return new_image;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsStateGuardian)::
CLP(GraphicsStateGuardian)(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties) 
{
  _error_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsStateGuardian)::
~CLP(GraphicsStateGuardian)() {
  close_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
reset() {
  free_pointers();
  GraphicsStateGuardian::reset();

  _buffer_mask = 0;

  // All GL implementations have the following buffers. (?)
  _buffer_mask = (RenderBuffer::T_color |
                  RenderBuffer::T_depth |
                  RenderBuffer::T_stencil |
                  RenderBuffer::T_accum);

  // Check to see if we have double-buffering.

  // This isn't completely right.  Instead of just clearing this bit
  // and disallowing writes to T_back, we need to set T_front on
  // operations that might have had T_back set otherwise.
  GLboolean has_back;
  GLP(GetBooleanv)(GL_DOUBLEBUFFER, &has_back);
  if (!has_back) {
    _buffer_mask &= ~RenderBuffer::T_back;
  }

#if 0
  //Note 8/01: This is incorrect for mono displays, if stereo is not in use, we need
  //           to use the GL_BACK constants and not the GL_BACK_LEFT ones, which
  //           under the RenderBuffer flag schemes require both left and right flags set.

  // Check to see if we have stereo (and therefore a right buffer).
  GLboolean has_stereo;
  GLP(GetBooleanv)(GL_STEREO, &has_stereo);
  if (!has_stereo) {
    _buffer_mask &= ~RenderBuffer::T_right;
  }
#endif

  // Set up our clear values to invalid values, so the GLP(Clear)* calls
  // will be made initially.
  _clear_color_red = -1.0f;
  _clear_color_green = -1.0f;
  _clear_color_blue = -1.0f;
  _clear_color_alpha = -1.0f;
  _clear_depth = -1.0f;
  _clear_stencil = -1;
  _clear_accum_red = -1.0f;
  _clear_accum_green = -1.0f;
  _clear_accum_blue = -1.0f;
  _clear_accum_alpha = -1.0f;

  // Set up the specific state values to GL's known initial values.
  _shade_model_mode = GL_SMOOTH;
  GLP(FrontFace)(GL_CCW);

  _scissor_x = 0;
  _scissor_y = 0;
  _scissor_width = -1;
  _scissor_height = -1;
  _viewport_x = 0;
  _viewport_y = 0;
  _viewport_width = -1;
  _viewport_height = -1;
  _lmodel_local = 0;
  _lmodel_twoside = 0;
  _line_width = 1.0f;
  _point_size = 1.0f;
  _blend_source_func = GL_ONE;
  _blend_dest_func = GL_ZERO;
  _depth_mask = false;
  _fog_mode = GL_EXP;
  _fog_density = 1.0f;
  _fog_color.set(0.0f, 0.0f, 0.0f, 0.0f);
  _alpha_func = GL_ALWAYS;
  _alpha_func_ref = 0;
  _polygon_mode = GL_FILL;

  _pack_alignment = 4;
  _unpack_alignment = 4;

  // Set up all the enabled/disabled flags to GL's known initial
  // values: everything off.
  _multisample_enabled = false;
  _line_smooth_enabled = false;
  _point_smooth_enabled = false;
  _scissor_enabled = false;
  _texturing_enabled = false;
  _stencil_test_enabled = false;
  _multisample_alpha_one_enabled = false;
  _multisample_alpha_mask_enabled = false;
  _blend_enabled = false;
  _depth_test_enabled = false;
  _fog_enabled = false;
  _alpha_test_enabled = false;
  _polygon_offset_enabled = false;
  _decal_level = 0;

  // Dither is on by default in GL; let's turn it off
  GLP(Disable)(GL_DITHER);
  _dithering_enabled = false;

  // Antialiasing.
  enable_line_smooth(false);
  enable_multisample(true);

  // Should we normalize lighting normals?
  if (CLP(auto_normalize_lighting)) {
    GLP(Enable)(GL_NORMALIZE);
  }

  // Count the max number of lights
  GLint max_lights;
  GLP(GetIntegerv)(GL_MAX_LIGHTS, &max_lights);
  _max_lights = max_lights;

  // Count the max number of clipping planes
  GLint max_clip_planes;
  GLP(GetIntegerv)(GL_MAX_CLIP_PLANES, &max_clip_planes);
  _max_clip_planes = max_clip_planes;

  _current_projection_mat = LMatrix4f::ident_mat();
  _projection_mat_stack_count = 0;

  // Make sure the GL state matches all of our initial attribute
  // states.
  CPT(RenderAttrib) dta = DepthTestAttrib::make(DepthTestAttrib::M_less);
  CPT(RenderAttrib) dwa = DepthWriteAttrib::make(DepthWriteAttrib::M_on);
  CPT(RenderAttrib) cfa = CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise);
  CPT(RenderAttrib) ta = TextureAttrib::make_off();

  dta->issue(this);
  dwa->issue(this);
  cfa->issue(this);
  ta->issue(this);

  Material empty;
  apply_material(&empty);

  if (CLP(cheap_textures)) {
    GLCAT.info()
      << "Setting GLP(Hint)() for fastest textures.\n";
    GLP(Hint)(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  }

  // use per-vertex fog if per-pixel fog requires SW renderer
  GLP(Hint)(GL_FOG_HINT, GL_DONT_CARE);

  GLint num_red_bits;
  GLP(GetIntegerv)(GL_RED_BITS, &num_red_bits);
  if (num_red_bits < 8) {
    GLP(Enable)(GL_DITHER);
    _dithering_enabled = true;
    if (GLCAT.is_debug()) {
      GLCAT.debug() 
        << "frame buffer depth = " << num_red_bits
        << " bits/channel, enabling dithering\n";
    }
  }

  // Output the vendor and version strings.
  show_gl_string("GL_VENDOR", GL_VENDOR);
  show_gl_string("GL_RENDERER", GL_RENDERER);
  show_gl_string("GL_VERSION", GL_VERSION);

  // Save the extensions tokens.
  save_extensions((const char *)glGetString(GL_EXTENSIONS));
  get_extra_extensions();
  report_extensions();

  _supports_bgr = has_extension("GL_EXT_bgra");
  _error_count = 0;

  report_my_gl_errors();
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
do_clear(const RenderBuffer &buffer) {
  nassertv(buffer._gsg == this);
  int buffer_type = buffer._buffer_type;
  GLbitfield mask = 0;
  CPT(RenderState) state = RenderState::make_empty();

  if (buffer_type & RenderBuffer::T_color) {
    call_glClearColor(_color_clear_value[0],
                      _color_clear_value[1],
                      _color_clear_value[2],
                      _color_clear_value[3]);
    state = state->add_attrib(ColorWriteAttrib::make(ColorWriteAttrib::M_on));
    mask |= GL_COLOR_BUFFER_BIT;

    set_draw_buffer(buffer);
  }

  if (buffer_type & RenderBuffer::T_depth) {
    call_glClearDepth(_depth_clear_value);
    mask |= GL_DEPTH_BUFFER_BIT;

    // In order to clear the depth buffer, the depth mask must enable
    // writing to the depth buffer.
    if (!_depth_mask) {
      state = state->add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_on));
    }
  }

  if (buffer_type & RenderBuffer::T_stencil) {
    call_glClearStencil(_stencil_clear_value != false);
    mask |= GL_STENCIL_BUFFER_BIT;
  }

  if (buffer_type & RenderBuffer::T_accum) {
    call_glClearAccum(_accum_clear_value[0],
                      _accum_clear_value[1],
                      _accum_clear_value[2],
                      _accum_clear_value[3]);
    mask |= GL_ACCUM_BUFFER_BIT;
  }

#ifdef GSG_VERBOSE
  GLCAT.spam() << "glClear(";
  if (mask & GL_COLOR_BUFFER_BIT) {
    GLCAT.spam(false) << "GL_COLOR_BUFFER_BIT|";
  }
  if (mask & GL_DEPTH_BUFFER_BIT) {
    GLCAT.spam(false) << "GL_DEPTH_BUFFER_BIT|";
  }
  if (mask & GL_STENCIL_BUFFER_BIT) {
    GLCAT.spam(false) << "GL_STENCIL_BUFFER_BIT|";
  }
  if (mask & GL_ACCUM_BUFFER_BIT) {
    GLCAT.spam(false) << "GL_ACCUM_BUFFER_BIT|";
  }
  GLCAT.spam(false) << ")" << endl;
#endif

  modify_state(state);

  GLP(Clear)(mask);
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//       scissor region and viewport)
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
prepare_display_region() {
  if (_current_display_region == (DisplayRegion*)0L) {
    GLCAT.error()
      << "Invalid NULL display region in prepare_display_region()\n";
    enable_scissor(false);

  } else if (_current_display_region != _actual_display_region) {
    _actual_display_region = _current_display_region;

    int l, b, w, h;
    _actual_display_region->get_region_pixels(l, b, w, h);
    GLint x = GLint(l);
    GLint y = GLint(b);
    GLsizei width = GLsizei(w);
    GLsizei height = GLsizei(h);

    enable_scissor( true );
    call_glScissor( x, y, width, height );
    call_glViewport( x, y, width, height );
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with push_lens()) active, so that
//               it will transform future rendered geometry.  Normally
//               this is only called from the draw process, and
//               usually it is called immediately after a call to
//               push_lens().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
prepare_lens() {
  if (_current_lens == (Lens *)NULL) {
    return false;
  }

  if (!_current_lens->is_linear()) {
    return false;
  }

  const LMatrix4f &projection_mat = _current_lens->get_projection_mat();

  // The projection matrix must always be right-handed Y-up, even if
  // our coordinate system of choice is otherwise, because certain GL
  // calls (specifically glTexGen(GL_SPHERE_MAP)) assume this kind of
  // a coordinate system.  Sigh.  In order to implement a Z-up (or
  // other arbitrary) coordinate system, we'll use a Y-up projection
  // matrix, and store the conversion to our coordinate system of
  // choice in the modelview matrix.
  LMatrix4f new_projection_mat =
    LMatrix4f::convert_mat(CS_yup_right, _current_lens->get_coordinate_system()) *
    projection_mat;

#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glMatrixMode(GL_PROJECTION): " << new_projection_mat << endl;
#endif
  GLP(MatrixMode)(GL_PROJECTION);
  GLP(LoadMatrixf)(new_projection_mat.get_data());
  report_my_gl_errors();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_frame
//       Access: Public, Virtual
//  Description: Called before each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup before
//               beginning the frame.
//
//               The return value is true if successful (in which case
//               the frame will be drawn and end_frame() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_frame() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
begin_frame() {
  if (!GraphicsStateGuardian::begin_frame()) {
    return false;
  }

  report_my_gl_errors();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
end_frame() {
  GraphicsStateGuardian::end_frame();

  // Calling glFlush() at the end of the frame is particularly
  // necessary if this is a single-buffered visual, so that the frame
  // will be finished drawing before we return to the application.
  // It's not clear what effect this has on our total frame time.
  GLP(Flush)();

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_point(GeomPoint *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_point()" << endl;
#endif
#ifdef DO_PSTATS
  PStatTimer timer(_draw_primitive_pcollector);
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  call_glPointSize(geom->get_size());
  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  GLP(Begin)(GL_POINTS);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    // Draw per vertex, same thing.
    issuer.issue_color(G_PER_VERTEX, ci);
    issuer.issue_normal(G_PER_VERTEX, ni);
    issuer.issue_texcoord(G_PER_VERTEX, ti);
    issuer.issue_vertex(G_PER_VERTEX, vi);
  }

  GLP(End)();
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_line(GeomLine *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_line()" << endl;
#endif
#ifdef DO_PSTATS
  PStatTimer timer(_draw_primitive_pcollector);
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  call_glLineWidth(geom->get_width());
  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  GLP(Begin)(GL_LINES);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    for (int j = 0; j < 2; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
  }

  GLP(End)();
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_linestrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_linestrip(GeomLinestrip *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_linestrip()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  call_glLineWidth(geom->get_width());
  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  const int *plen = geom->get_lengths();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    int num_verts = *(plen++);
    nassertv(num_verts >= 2);

    GLP(Begin)(GL_LINE_STRIP);

    // Per-component attributes for the first line segment?
    issuer.issue_color(G_PER_COMPONENT, ci);
    issuer.issue_normal(G_PER_COMPONENT, ni);

    // Draw the first 2 vertices
    int v;
    for (v = 0; v < 2; v++) {
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }

    // Now draw each of the remaining vertices.  Each vertex from
    // this point on defines a new line segment.
    for (v = 2; v < num_verts; v++) {
      // Per-component attributes?
      issuer.issue_color(G_PER_COMPONENT, ci);
      issuer.issue_normal(G_PER_COMPONENT, ni);

      // Per-vertex attributes
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    GLP(End)();
  }
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_sprite
//       Access: Public, Virtual
//  Description: CSN, 7/11/00
////////////////////////////////////////////////////////////////////

// this class exists because an alpha sort is necessary for correct
// sprite rendering, and we can't simply sort the vertex arrays as
// each vertex may or may not have corresponding information in the
// x/y texel-world-ratio and rotation arrays.
class WrappedSprite {
public:
  Vertexf _v;
  Colorf _c;
  float _x_ratio;
  float _y_ratio;
  float _theta;
};

// this struct exists because the STL can sort faster than i can.
struct draw_sprite_vertex_less {
  INLINE bool operator ()(const WrappedSprite& v0,
                          const WrappedSprite& v1) const {
    return v0._v[2] < v1._v[2]; }
};

void CLP(GraphicsStateGuardian)::
draw_sprite(GeomSprite *geom, GeomContext *) {
  // this is a little bit of a mess, but it's ok.  Here's the deal:
  // we want to draw, and draw quickly, an arbitrarily large number
  // of sprites all facing the screen.  Performing the billboard math
  // for ~1000 sprites is way too slow.  Ideally, we want one
  // matrix transformation that will handle everything, and this is
  // just about what ends up happening. We're getting the front-facing
  // effect by setting up a new frustum (of the same z-depth as the
  // current one) that is very small in x and y.  This way regularly
  // rendered triangles that might not be EXACTLY facing the camera
  // will certainly look close enough.  Then, we transform to camera-space
  // by hand and apply the inverse frustum to the transformed point.
  // For some cracked out reason, this actually works.
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_sprite()" << endl;
#endif

  // get the array traversal set up.
  int nprims = geom->get_num_prims();
  if (nprims==0) {
      return;
  }

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  // need some interface so user can set 2d dimensions if no texture specified
  float tex_xsize = 1.0f;  
  float tex_ysize = 1.0f;

  Texture *tex = geom->get_texture();
  if(tex != NULL) {
    // set up the texture-rendering state
    modify_state(RenderState::make
                 (TextureAttrib::make(tex),
                  TextureApplyAttrib::make(TextureApplyAttrib::M_modulate)));
    tex_xsize = tex->_pbuffer->get_xsize();
    tex_ysize = tex->_pbuffer->get_ysize();
  }

  // save the modelview matrix
  const LMatrix4f &modelview_mat = _transform->get_mat();

  // We don't need to mess with the aspect ratio, since we are now
  // using the default projection matrix, which has the right aspect
  // ratio built in.

  // load up our own matrices
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(LoadIdentity)();

  // precomputation stuff
  float tex_left = geom->get_ll_uv()[0];
  float tex_right = geom->get_ur_uv()[0];
  float tex_bottom = geom->get_ll_uv()[1];
  float tex_top = geom->get_ur_uv()[1];

  float half_width =  0.5f * tex_xsize * fabs(tex_right - tex_left);
  float half_height = 0.5f * tex_ysize * fabs(tex_top - tex_bottom);
  float scaled_width = 0.0f;
  float scaled_height = 0.0f;

  // the user can override alpha sorting if they want
  bool alpha = false;

  if (!geom->get_alpha_disable()) {
    // figure out if alpha's enabled (if not, no reason to sort)
    const TransparencyAttrib *trans = _state->get_transparency();
    if (trans != (const TransparencyAttrib *)NULL) {
      alpha = (trans->get_mode() != TransparencyAttrib::M_none);
    }
  }

  // sort container and iterator
  pvector< WrappedSprite > cameraspace_vector;
  pvector< WrappedSprite >::iterator vec_iter;

  // inner loop vars
  int i;
  Vertexf source_vert, cameraspace_vert;
  float *x_walk = (float *)NULL;
  float *y_walk = (float *)NULL;
  float *theta_walk = (float *)NULL;
  float theta = 0.0f;

  nassertv(geom->get_x_bind_type() != G_PER_VERTEX);
  nassertv(geom->get_y_bind_type() != G_PER_VERTEX);

  // set up the non-built-in bindings
  bool x_overall = (geom->get_x_bind_type() == G_OVERALL);
  bool y_overall = (geom->get_y_bind_type() == G_OVERALL);
  bool theta_overall = (geom->get_theta_bind_type() == G_OVERALL);
  bool color_overall = (geom->get_binding(G_COLOR) == G_OVERALL);
  bool theta_on = !(geom->get_theta_bind_type() == G_OFF);

  // x direction
  if (x_overall)
    scaled_width = geom->_x_texel_ratio[0] * half_width;
  else {
    nassertv(((int)geom->_x_texel_ratio.size() >= geom->get_num_prims()));
    x_walk = &geom->_x_texel_ratio[0];
  }

  // y direction
  if (y_overall)
    scaled_height = geom->_y_texel_ratio[0] * half_height;
  else {
    nassertv(((int)geom->_y_texel_ratio.size() >= geom->get_num_prims()));
    y_walk = &geom->_y_texel_ratio[0];
  }

  // theta
  if (theta_on) {
    if (theta_overall)
      theta = geom->_theta[0];
    else {
      nassertv(((int)geom->_theta.size() >= geom->get_num_prims()));
      theta_walk = &geom->_theta[0];
    }
  }

  /////////////////////////////////////////////////////////////////////
  // INNER LOOP PART 1 STARTS HERE
  // Here we transform each point to cameraspace and fill our sort
  // vector with the final geometric information.
  /////////////////////////////////////////////////////////////////////

  cameraspace_vector.reserve(nprims);   //pre-alloc space for nprims

  // the state is set, start running the prims
  for (i = 0; i < nprims; i++) {
    WrappedSprite ws;

    source_vert = geom->get_next_vertex(vi);

    // this mult converts to y-up cameraspace.
    cameraspace_vert = source_vert * modelview_mat;
    // build the final object that will go into the vector.
    ws._v.set(cameraspace_vert[0],cameraspace_vert[1],cameraspace_vert[2]);

    if (!color_overall)
      ws._c = geom->get_next_color(ci);
    if (!x_overall)
      ws._x_ratio = *x_walk++;
    if (!y_overall)
      ws._y_ratio = *y_walk++;
    if (theta_on) {
      if (!theta_overall)
        ws._theta = *theta_walk++;
    }

    cameraspace_vector.push_back(ws);
  }

  // now the verts are properly sorted by alpha (if necessary).  Of course,
  // the sort is only local, not scene-global, so if you look closely you'll
  // notice that alphas may be screwy.  It's ok though, because this is fast.
  // if you want accuracy, use billboards and take the speed hit.
  if (alpha) {
    sort(cameraspace_vector.begin(), cameraspace_vector.end(),
         draw_sprite_vertex_less());

     if (_dithering_enabled)
         GLP(Disable)(GL_DITHER);
  }

  Vertexf ul, ur, ll, lr;

  if (color_overall)
    GLP(Color4fv)(geom->get_next_color(ci).get_data());

  ////////////////////////////////////////////////////////////////////////////
  // INNER LOOP PART 2 STARTS HERE
  // Now we run through the cameraspace vector and compute the geometry for each
  // tristrip.  This includes scaling as per the ratio arrays, as well as
  // rotating in the z.
  ////////////////////////////////////////////////////////////////////////////

  vec_iter = cameraspace_vector.begin();
  for (; vec_iter != cameraspace_vector.end(); vec_iter++) {
    WrappedSprite& cur_image = *vec_iter;

    // if not G_OVERALL, calculate the scale factors
    if (x_overall == false)
      scaled_width = cur_image._x_ratio * half_width;

    if (y_overall == false)
      scaled_height = cur_image._y_ratio * half_height;

    // if not G_OVERALL, do some trig for this z rotate
    if (theta_on) {
      if (theta_overall == false)
        theta = cur_image._theta;

      // create the rotated points
      LMatrix3f xform_mat = LMatrix3f::rotate_mat(theta) * LMatrix3f::scale_mat(scaled_width, scaled_height);

      ur = (LVector3f( 1,  1, 0) * xform_mat) + cur_image._v;
      ul = (LVector3f(-1,  1, 0) * xform_mat) + cur_image._v;
      lr = (LVector3f( 1, -1, 0) * xform_mat) + cur_image._v;
      ll = (LVector3f(-1, -1, 0) * xform_mat) + cur_image._v;
    }
    else {
      // create the normal points
      ur.set(scaled_width, scaled_height, 0);
      ul.set(-scaled_width, scaled_height, 0);
      lr.set(scaled_width, -scaled_height, 0);
      ll.set(-scaled_width, -scaled_height, 0);

      ur += cur_image._v;
      ul += cur_image._v;
      lr += cur_image._v;
      ll += cur_image._v;
    }

    // set the color
    if (color_overall == false)
      GLP(Color4fv)(cur_image._c.get_data());

    // draw each one as a 2-element tri-strip
    GLP(Begin)(GL_TRIANGLE_STRIP);
    GLP(Normal3f)(0.0f, 0.0f, 1.0f);
    GLP(TexCoord2f)(tex_left, tex_bottom);  GLP(Vertex3fv)(ll.get_data());
    GLP(TexCoord2f)(tex_right, tex_bottom); GLP(Vertex3fv)(lr.get_data());
    GLP(TexCoord2f)(tex_left, tex_top);     GLP(Vertex3fv)(ul.get_data());
    GLP(TexCoord2f)(tex_right, tex_top);    GLP(Vertex3fv)(ur.get_data());
    GLP(End)();
  }

  // restore the matrices
  GLP(LoadMatrixf)(modelview_mat.get_data());

  if(alpha && _dithering_enabled)
     GLP(Enable)(GL_DITHER);

  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_polygon
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_polygon(GeomPolygon *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_polygon()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  const int *plen = geom->get_lengths();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    int num_verts = *(plen++);
    nassertv(num_verts >= 3);

    GLP(Begin)(GL_POLYGON);

    // Draw the vertices.
    int v;
    for (v = 0; v < num_verts; v++) {
      // Per-vertex attributes.
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    GLP(End)();
  }
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_tri
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_tri(GeomTri *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_tri()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_tri_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  GLP(Begin)(GL_TRIANGLES);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    for (int j = 0; j < 3; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
  }

  GLP(End)();
  report_my_gl_errors();
#ifdef DO_PSTATS
  _draw_primitive_pcollector.stop();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_quad
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_quad(GeomQuad *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_quad()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  GLP(Begin)(GL_QUADS);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    for (int j = 0; j < 4; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
  }

  GLP(End)();
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_tristrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_tristrip(GeomTristrip *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_tristrip()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_tristrip_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  const int *plen = geom->get_lengths();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    int num_verts = *(plen++);
    nassertv(num_verts >= 3);

    GLP(Begin)(GL_TRIANGLE_STRIP);

    // Per-component attributes for the first triangle?
    issuer.issue_color(G_PER_COMPONENT, ci);
    issuer.issue_normal(G_PER_COMPONENT, ni);

    // Draw the first three vertices.
    int v;
    for (v = 0; v < 3; v++) {
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }

    // Now draw each of the remaining vertices.  Each vertex from
    // this point on defines a new triangle.
    for (v = 3; v < num_verts; v++) {
      // Per-component attributes?
      issuer.issue_color(G_PER_COMPONENT, ci);
      issuer.issue_normal(G_PER_COMPONENT, ni);

      // Per-vertex attributes.
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    GLP(End)();
  }
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_trifan
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_trifan(GeomTrifan *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_trifan()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_trifan_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  const int *plen = geom->get_lengths();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if ((geom->get_binding(G_COLOR) == G_PER_VERTEX && wants_colors()) ||
      (geom->get_binding(G_NORMAL) == G_PER_VERTEX && wants_normals())) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    int num_verts = *(plen++);
    nassertv(num_verts >= 3);

    GLP(Begin)(GL_TRIANGLE_FAN);

    // Per-component attributes for the first triangle?
    issuer.issue_color(G_PER_COMPONENT, ci);
    issuer.issue_normal(G_PER_COMPONENT, ni);

    // Draw the first three vertices.
    int v;
    for (v = 0; v < 3; v++) {
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }

    // Now draw each of the remaining vertices.  Each vertex from
    // this point on defines a new triangle.
    for (v = 3; v < num_verts; v++) {
      // Per-component attributes?
      issuer.issue_color(G_PER_COMPONENT, ci);
      issuer.issue_normal(G_PER_COMPONENT, ni);

      // Per-vertex attributes.
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    GLP(End)();
  }
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_sphere(GeomSphere *geom, GeomContext *) {
#ifdef GSG_VERBOSE
  GLCAT.spam() << "draw_sphere()" << endl;
#endif

#ifdef DO_PSTATS
  //  PStatTimer timer(_draw_primitive_pcollector);
  // Using PStatTimer may cause a compiler crash.
  _draw_primitive_pcollector.start();
  _vertices_other_pcollector.add_level(geom->get_num_vertices());
#endif

  issue_scene_graph_color();

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

  if (_color_transform_enabled == 0) {
    issue_color = issue_color_gl;
  } else {
    issue_color = issue_transformed_color_gl;
  }

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color);

  if (wants_normals()) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);

  GLUquadricObj *sph = GLUP(NewQuadric)();
  GLUP(QuadricNormals)(sph, wants_normals() ? (GLenum)GLU_SMOOTH : (GLenum)GLU_NONE);
  GLUP(QuadricTexture)(sph, wants_texcoords() ? (GLenum)GL_TRUE : (GLenum)GL_FALSE);
  GLUP(QuadricOrientation)(sph, (GLenum)GLU_OUTSIDE);
  GLUP(QuadricDrawStyle)(sph, (GLenum)GLU_FILL);
  //GLUP(QuadricDrawStyle)(sph, (GLenum)GLU_LINE);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);

    for (int j = 0; j < 2; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
    }
    Vertexf center = geom->get_next_vertex(vi);
    Vertexf edge = geom->get_next_vertex(vi);
    LVector3f v = edge - center;
    float r = sqrt(dot(v, v));

    // Since GLUP(Sphere) doesn't have a center parameter, we have to use
    // a matrix transform.

    GLP(MatrixMode)(GL_MODELVIEW);
    GLP(PushMatrix)();
    GLP(MultMatrixf)(LMatrix4f::translate_mat(center).get_data());

    // Now render the sphere using GLU calls.
    GLUP(Sphere)(sph, r, 16, 10);

    GLP(MatrixMode)(GL_MODELVIEW);
    GLP(PopMatrix)();
  }

  GLUP(DeleteQuadric)(sph);
  report_my_gl_errors();
  DO_PSTATS_STUFF(_draw_primitive_pcollector.stop());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::prepare_texture
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given texture, and returns a newly-allocated
//               TextureContext pointer to reference it.  It is the
//               responsibility of the calling function to later
//               call release_texture() with this same pointer (which
//               will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *CLP(GraphicsStateGuardian)::
prepare_texture(Texture *tex) {
  CLP(TextureContext) *gtc = new CLP(TextureContext)(tex);
  GLP(GenTextures)(1, &gtc->_index);

  bind_texture(gtc);
  GLP(PrioritizeTextures)(1, &gtc->_index, &gtc->_priority);
  specify_texture(tex);
  apply_texture_immediate(tex);

  report_my_gl_errors();
  return gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::apply_texture
//       Access: Public, Virtual
//  Description: Makes the texture the currently available texture for
//               rendering.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
apply_texture(TextureContext *tc) {
  add_to_texture_record(tc);
  bind_texture(tc);

  int dirty = tc->get_dirty_flags();
  if ((dirty & (Texture::DF_wrap | Texture::DF_filter)) != 0) {
    // We need to re-specify the texture properties.
    specify_texture(tc->_texture);
  }
  if ((dirty & (Texture::DF_image | Texture::DF_mipmap)) != 0) {
    // We need to re-apply the image.
    apply_texture_immediate(tc->_texture);
  }

  tc->clear_dirty_flags();

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.  This function should never be called
//               directly; instead, call Texture::release() (or simply
//               let the Texture destruct).
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
release_texture(TextureContext *tc) {
  CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);

  GLP(DeleteTextures)(1, &gtc->_index);
  report_my_gl_errors();

  gtc->_index = 0;
  delete gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::prepare_geom_node
//       Access: Public, Virtual
//  Description: Prepares the indicated GeomNode for retained-mode
//               rendering.  If this function returns non-NULL, the
//               value returned will be passed back to a future call
//               to draw_geom_node(), which is expected to draw the
//               contents of the node.
////////////////////////////////////////////////////////////////////
GeomNodeContext *CLP(GraphicsStateGuardian)::
prepare_geom_node(GeomNode *node) {
#if 0  // temporarily disabled until we bring to new scene graph

  // Make sure we have at least some static Geoms in the GeomNode;
  // otherwise there's no point in building a display list.
  int num_geoms = node->get_num_geoms();
  bool all_dynamic = true;
  int i;

  for (i = 0; (i < num_geoms) && all_dynamic; i++) {
    dDrawable *geom = node->get_geom(i);
    all_dynamic = geom->is_dynamic();
  }
  if (all_dynamic) {
    // Never mind.
    return (GeomNodeContext *)NULL;
  }

  // Ok, we've got something; use it.
  CLP(GeomNodeContext) *ggnc = new CLP(GeomNodeContext)(node);
  ggnc->_index = GLP(GenLists)(1);
  if (ggnc->_index == 0) {
    GLCAT.error()
      << "Ran out of display list indices.\n";
    delete ggnc;
    return (GeomNodeContext *)NULL;
  }

  // We need to temporarily force normals and UV's on, so the display
  // list will have them built in.
  bool old_normals_enabled = _normals_enabled;
  bool old_texturing_enabled = _texturing_enabled;
  bool old_vertex_colors_enabled = _vertex_colors_enabled;
  _normals_enabled = true;
  _texturing_enabled = true;
  _vertex_colors_enabled = true;

#ifdef DO_PSTATS
  // Count up the number of vertices we're about to render, by
  // checking the PStats vertex counters now, and at the end.  This is
  // kind of hacky, but this is debug code.
  float num_verts_before = 
    _vertices_tristrip_pcollector.get_level() +
    _vertices_trifan_pcollector.get_level() +
    _vertices_tri_pcollector.get_level() +
    _vertices_other_pcollector.get_level();
#endif

  // Now define the display list.
  GLP(NewList)(ggnc->_index, GL_COMPILE);
  for (i = 0; i < num_geoms; i++) {
    dDrawable *geom = node->get_geom(i);
    if (geom->is_dynamic()) {
      // Wait, this is a dynamic Geom.  We can't safely put it in the
      // display list, because it may change from one frame to the
      // next; instead, we'll keep it out.
      ggnc->_dynamic_geoms.push_back(geom);
    } else {
      // A static Geom becomes part of the display list.
      geom->draw(this);
    }
  }
  GLP(EndList)();

#ifdef DO_PSTATS
  float num_verts_after = 
    _vertices_tristrip_pcollector.get_level() +
    _vertices_trifan_pcollector.get_level() +
    _vertices_tri_pcollector.get_level() +
    _vertices_other_pcollector.get_level();
  float num_verts = num_verts_after - num_verts_before;
  ggnc->_num_verts = (int)(num_verts + 0.5);
#endif

  _normals_enabled = old_normals_enabled;
  _texturing_enabled = old_texturing_enabled;
  _vertex_colors_enabled = old_vertex_colors_enabled;

  bool inserted = _prepared_objects->mark_prepared_geom_node(ggnc);

  // If this assertion fails, the same GeomNode was prepared twice,
  // which shouldn't be possible, since the GeomNode itself should
  // detect this.
  nassertr(inserted, NULL);

  return ggnc;
#endif  // temporarily disabled until we bring to new scene graph
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_geom_node
//       Access: Public, Virtual
//  Description: Draws a GeomNode previously indicated by a call to
//               prepare_geom_node().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_geom_node(GeomNode *node, const RenderState *state,
               GeomNodeContext *gnc) {
#if 0  // temporarily disabled until we bring to new scene graph
  if (gnc == (GeomNodeContext *)NULL) {
    // We don't have a saved context; just draw the GeomNode in
    // immediate mode.
    int num_geoms = node->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      node->get_geom(i)->draw(this);
    }

  } else {
    // We do have a saved context; use it.
    add_to_geom_node_record(gnc);
    CLP(GeomNodeContext) *ggnc = DCAST(CLP(GeomNodeContext), gnc);
    GLP(CallList)(ggnc->_index);

#ifdef DO_PSTATS 
    PStatTimer timer(_draw_primitive_pcollector);
    _vertices_display_list_pcollector.add_level(ggnc->_num_verts);
#endif

    // Also draw all the dynamic Geoms.
    int num_geoms = ggnc->_dynamic_geoms.size();
    for (int i = 0; i < num_geoms; i++) {
      ggnc->_dynamic_geoms[i]->draw(this);
    }
  }
#endif  // temporarily disabled until we bring to new scene graph
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::release_geom_node
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_geom_node(), including deleting the
//               GeomNodeContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
release_geom_node(GeomNodeContext *gnc) {
#if 0  // temporarily disabled until we bring to new scene graph
  if (gnc != (GeomNodeContext *)NULL) {
    CLP(GeomNodeContext) *ggnc = DCAST(CLP(GeomNodeContext), gnc);
    GLP(DeleteLists)(ggnc->_index, 1);

    bool erased = _prepared_objects->unmark_prepared_geom_node(ggnc);

    // If this assertion fails, a GeomNode was released that hadn't
    // been prepared (or a GeomNode was released twice).
    nassertv(erased);
    
    ggnc->_node->clear_gsg(this);
    delete ggnc;
  }
#endif  // temporarily disabled until we bring to new scene graph
}

#if 0
static int logs[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
                      4096, 0 };

// This function returns the smallest power of two greater than or
// equal to x.
static int binary_log_cap(const int x) {
  int i = 0;
  for (; (x > logs[i]) && (logs[i] != 0); ++i);
  if (logs[i] == 0)
    return 4096;
  return logs[i];
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::copy_texture
//       Access: Public, Virtual
//  Description: Copy the pixel region indicated by the display
//               region from the framebuffer into texture memory
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
copy_texture(TextureContext *tc, const DisplayRegion *dr) {
  nassertv(tc != NULL && dr != NULL);
  Texture *tex = tc->_texture;

  int xo, yo, w, h;

#if 0
  // Determine the size of the grab from the given display region
  // If the requested region is not a power of two, grab a region that is
  // a power of two that contains the requested region
  int req_w, req_h;
  dr->get_region_pixels(xo, yo, req_w, req_h);
  w = binary_log_cap(req_w);
  h = binary_log_cap(req_h);
  if (w != req_w || h != req_h) {
    tex->_requested_w = req_w;
    tex->_requested_h = req_h;
    tex->_has_requested_size = true;
  }
#else
  // doing the above is bad unless you also provide some way
  // for the caller to adjust his texture coordinates accordingly
  // this was done for 'draw_texture' but not for anything else
  
  dr->get_region_pixels(xo, yo, w, h);
#endif

  PixelBuffer *pb = tex->_pbuffer;
  pb->set_size(xo,yo,w,h);

  bind_texture(tc);

  GLP(CopyTexImage2D)(GL_TEXTURE_2D, 0,
                   get_internal_image_format(pb->get_format()),
                   xo, yo, w, h, pb->get_border());

  // Clear the internal texture state, since we've just monkeyed with it.
  modify_state(get_untextured_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::copy_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
copy_texture(TextureContext *tc, const DisplayRegion *dr, const RenderBuffer &rb) {
  set_read_buffer(rb);
  copy_texture(tc, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb) {
  // This code is now invalidated by the new design; perhaps the
  // interface is not needed anyway.
#if 0
  nassertv(tc != NULL && pb != NULL);
  Texture *tex = tc->_texture;

  int w = tex->_pbuffer->get_xsize();
  int h = tex->_pbuffer->get_ysize();

  PT(DisplayRegion) dr = _win->make_scratch_display_region(w, h);

  FrameBufferStack old_fb = push_frame_buffer
    (get_render_buffer(RenderBuffer::T_back | RenderBuffer::T_depth),
     dr);

  texture_to_pixel_buffer(tc, pb, dr);

  pop_frame_buffer(old_fb);
  report_my_gl_errors();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                        const DisplayRegion *dr) {
  nassertv(tc != NULL && pb != NULL && dr != NULL);
  Texture *tex = tc->_texture;

  // Do a deep copy to initialize the pixel buffer
  pb->copy(tex->_pbuffer);

  // If the image was empty, we need to render the texture into the frame
  // buffer and then copy the results into the pixel buffer's image
  if (pb->_image.empty()) {
    int w = pb->get_xsize();
    int h = pb->get_ysize();
    draw_texture(tc, dr);
    pb->_image = PTA_uchar::empty_array(w * h * pb->get_num_components());
    copy_pixel_buffer(pb, dr);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr) {
  nassertr(pb != NULL && dr != NULL, false);
  set_pack_alignment(1);

  // Bug fix for RE, RE2, and VTX - need to disable texturing in order
  // for GLP(ReadPixels)() to work
  // NOTE: reading the depth buffer is *much* slower than reading the
  // color buffer
  modify_state(get_untextured_state());

  int xo, yo, w, h;
  dr->get_region_pixels(xo, yo, w, h);

  GLenum external_format = get_external_image_format(pb->get_format());

#ifdef GSG_VERBOSE
  GLCAT.debug()
    << "glReadPixels(" << pb->get_xorg() << ", " << pb->get_yorg()
    << ", " << pb->get_xsize() << ", " << pb->get_ysize()
    << ", ";
  switch (external_format) {
  case GL_DEPTH_COMPONENT:
    GLCAT.debug(false) << "GL_DEPTH_COMPONENT, ";
    break;
  case GL_RGB:
    GLCAT.debug(false) << "GL_RGB, ";
    break;
  case GL_RGBA:
    GLCAT.debug(false) << "GL_RGBA, ";
    break;
  case GL_BGR:
    GLCAT.debug(false) << "GL_BGR, ";
    break;
  case GL_BGRA:
    GLCAT.debug(false) << "GL_BGRA, ";
    break;
  default:
    GLCAT.debug(false) << "unknown, ";
    break;
  }
  switch (get_image_type(pb->get_image_type())) {
  case GL_UNSIGNED_BYTE:
    GLCAT.debug(false) << "GL_UNSIGNED_BYTE, ";
    break;
  case GL_FLOAT:
    GLCAT.debug(false) << "GL_FLOAT, ";
    break;
  default:
    GLCAT.debug(false) << "unknown, ";
    break;
  }
  GLCAT.debug(false)
    << (void *)pb->_image.p() << ")" << endl;
#endif

  // pixelbuffer "origin" represents upper left screen point at which
  // pixelbuffer should be drawn using draw_pixel_buffer
  nassertr(!pb->_image.empty(), false);
  GLP(ReadPixels)(pb->get_xorg() + xo, pb->get_yorg() + yo,
               pb->get_xsize(), pb->get_ysize(),
               external_format,
               get_image_type(pb->get_image_type()),
               pb->_image.p());

  // We may have to reverse the byte ordering of the image if GL
  // didn't do it for us.
  pb->_image = fix_component_ordering(external_format, pb);

  report_my_gl_errors();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const RenderBuffer &rb) {
  set_read_buffer(rb);
  return copy_pixel_buffer(pb, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::apply_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::apply_material(const Material *material) {
  GLenum face = material->get_twoside() ? GL_FRONT_AND_BACK : GL_FRONT;

  GLP(Materialfv)(face, GL_SPECULAR, material->get_specular().get_data());
  GLP(Materialfv)(face, GL_EMISSION, material->get_emission().get_data());
  GLP(Materialf)(face, GL_SHININESS, material->get_shininess());

  if (material->has_ambient() && material->has_diffuse()) {
    // The material has both an ambient and diffuse specified.  This
    // means we do not need glMaterialColor().
    GLP(Disable)(GL_COLOR_MATERIAL);
    GLP(Materialfv)(face, GL_AMBIENT, material->get_ambient().get_data());
    GLP(Materialfv)(face, GL_DIFFUSE, material->get_diffuse().get_data());

  } else if (material->has_ambient()) {
    // The material specifies an ambient, but not a diffuse component.
    // The diffuse component comes from the object's color.
    GLP(Materialfv)(face, GL_AMBIENT, material->get_ambient().get_data());
    GLP(ColorMaterial)(face, GL_DIFFUSE);
    GLP(Enable)(GL_COLOR_MATERIAL);

  } else if (material->has_diffuse()) {
    // The material specifies a diffuse, but not an ambient component.
    // The ambient component comes from the object's color.
    GLP(Materialfv)(face, GL_DIFFUSE, material->get_diffuse().get_data());
    GLP(ColorMaterial)(face, GL_AMBIENT);
    GLP(Enable)(GL_COLOR_MATERIAL);

  } else {
    // The material specifies neither a diffuse nor an ambient
    // component.  Both components come from the object's color.
    GLP(ColorMaterial)(face, GL_AMBIENT_AND_DIFFUSE);
    GLP(Enable)(GL_COLOR_MATERIAL);
  }

  call_glLightModelLocal(material->get_local());
  call_glLightModelTwoSide(material->get_twoside());
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
apply_fog(Fog *fog) {
  Fog::Mode fmode = fog->get_mode();
  call_glFogMode(get_fog_mode_type(fmode));

  if (fmode == Fog::M_linear) {
    float onset, opaque;
    fog->get_linear_range(onset, opaque);
    call_glFogStart(onset);
    call_glFogEnd(opaque);

  } else {
    // Exponential fog is always camera-relative.
    call_glFogDensity(fog->get_exp_density());
  }

  call_glFogColor(fog->get_color());
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_transform
//       Access: Public, Virtual
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_transform(const TransformState *transform) {
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glLoadMatrix(GL_MODELVIEW): " << transform->get_mat() << endl;
#endif
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(LoadMatrixf)(transform->get_mat().get_data());

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_tex_matrix
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_tex_matrix(const TexMatrixAttrib *attrib) {
  GLP(MatrixMode)(GL_TEXTURE);
  GLP(LoadMatrixf)(attrib->get_mat().get_data());
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_texture(const TextureAttrib *attrib) {
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));
  if (attrib->is_off()) {
    enable_texturing(false);
  } else {
    enable_texturing(true);
    Texture *tex = attrib->get_texture();
    nassertv(tex != (Texture *)NULL);

    TextureContext *tc = tex->prepare_now(_prepared_objects, this);
    apply_texture(tc);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_material(const MaterialAttrib *attrib) {
  const Material *material = attrib->get_material();
  if (material != (const Material *)NULL) {
    apply_material(material);
  } else {
    // Apply a default material when materials are turned off.
    Material empty;
    apply_material(&empty);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_render_mode
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_render_mode(const RenderModeAttrib *attrib) {
  RenderModeAttrib::Mode mode = attrib->get_mode();

  switch (mode) {
  case RenderModeAttrib::M_filled:
    call_glPolygonMode(GL_FILL);
    break;

  case RenderModeAttrib::M_wireframe:
    call_glLineWidth(attrib->get_line_width());
    call_glPolygonMode(GL_LINE);
    break;

  default:
    GLCAT.error()
      << "Unknown render mode " << (int)mode << endl;
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_texture_apply
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_texture_apply(const TextureApplyAttrib *attrib) {
  GLint glmode = get_texture_apply_mode_type(attrib->get_mode());
  GLP(TexEnvi)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, glmode);
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_color_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_color_write(const ColorWriteAttrib *attrib) {
  // If we did not override this function, the default implementation
  // would achieve turning off color writes by changing the blend mode
  // in set_blend_mode().  However, since GL does support an easy way
  // to disable writes to the color buffer, we can take advantage of
  // it here.
  if (CLP(color_mask)) {
    ColorWriteAttrib::Mode mode = attrib->get_mode();
    if (mode == ColorWriteAttrib::M_off) {
      GLP(ColorMask)(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    } else {
      GLP(ColorMask)(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    report_my_gl_errors();

  } else {
    // Some implementations don't seem to handle GLP(ColorMask)() very
    // robustly, however, so we provide this fallback.
    GraphicsStateGuardian::issue_color_write(attrib);
  }
}

// PandaCompareFunc - 1 + 0x200 === GL_NEVER, etc.  order is sequential
#define PANDA_TO_GL_COMPAREFUNC(PANDACMPFUNC) (PANDACMPFUNC-1 +0x200)

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_depth_test(const DepthTestAttrib *attrib) {
  DepthTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == DepthTestAttrib::M_none) {
    enable_depth_test(false);
  } else {
    enable_depth_test(true);
    GLP(DepthFunc)(PANDA_TO_GL_COMPAREFUNC(mode));
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_alpha_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_alpha_test(const AlphaTestAttrib *attrib) {
  AlphaTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == AlphaTestAttrib::M_none) {
    enable_alpha_test(false);
  } else {
    assert(GL_NEVER==(AlphaTestAttrib::M_never-1+0x200));
    call_glAlphaFunc(PANDA_TO_GL_COMPAREFUNC(mode), attrib->get_reference_alpha());
    enable_alpha_test(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_depth_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_depth_write(const DepthWriteAttrib *attrib) {
  DepthWriteAttrib::Mode mode = attrib->get_mode();
  if (mode == DepthWriteAttrib::M_off) {
    GLP(DepthMask)(GL_FALSE);
  } else {
    GLP(DepthMask)(GL_TRUE);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_cull_face
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_cull_face(const CullFaceAttrib *attrib) {
  CullFaceAttrib::Mode mode = attrib->get_effective_mode();

  switch (mode) {
  case CullFaceAttrib::M_cull_none:
    GLP(Disable)(GL_CULL_FACE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    GLP(Enable)(GL_CULL_FACE);
    GLP(CullFace)(GL_BACK);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    GLP(Enable)(GL_CULL_FACE);
    GLP(CullFace)(GL_FRONT);
    break;
  default:
    GLCAT.error()
      << "invalid cull face mode " << (int)mode << endl;
    break;
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_fog(const FogAttrib *attrib) {
  if (!attrib->is_off()) {
    enable_fog(true);
    Fog *fog = attrib->get_fog();
    nassertv(fog != (Fog *)NULL);
    apply_fog(fog);
  } else {
    enable_fog(false);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_depth_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_depth_offset(const DepthOffsetAttrib *attrib) {
  int offset = attrib->get_offset();

  if (offset != 0) {
    // The relationship between these two parameters is a little
    // unclear and poorly explained in the GL man pages.
    GLP(PolygonOffset)((GLfloat) -offset, (GLfloat) -offset);
    enable_polygon_offset(true);

  } else {
    enable_polygon_offset(false);
  }

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
bind_light(PointLight *light, int light_id) {
  GLenum id = get_light_id(light_id);
  static const Colorf black(0.0f, 0.0f, 0.0f, 1.0f);
  GLP(Lightfv)(id, GL_AMBIENT, black.get_data());
  GLP(Lightfv)(id, GL_DIFFUSE, light->get_color().get_data());
  GLP(Lightfv)(id, GL_SPECULAR, light->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_scene_root());
  LPoint3f pos = light->get_point() * light_mat;

  LPoint4f fpos(pos[0], pos[1], pos[2], 1.0f);
  GLP(Lightfv)(id, GL_POSITION, fpos.get_data());

  // GL_SPOT_DIRECTION is not significant when cutoff == 180

  // Exponent == 0 implies uniform light distribution
  GLP(Lightf)(id, GL_SPOT_EXPONENT, 0.0f);

  // Cutoff == 180 means uniform point light source
  GLP(Lightf)(id, GL_SPOT_CUTOFF, 180.0f);

  const LVecBase3f &att = light->get_attenuation();
  GLP(Lightf)(id, GL_CONSTANT_ATTENUATION, att[0]);
  GLP(Lightf)(id, GL_LINEAR_ATTENUATION, att[1]);
  GLP(Lightf)(id, GL_QUADRATIC_ATTENUATION, att[2]);

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
bind_light(DirectionalLight *light, int light_id) {
  GLenum id = get_light_id( light_id );
  static const Colorf black(0.0f, 0.0f, 0.0f, 1.0f);
  GLP(Lightfv)(id, GL_AMBIENT, black.get_data());
  GLP(Lightfv)(id, GL_DIFFUSE, light->get_color().get_data());
  GLP(Lightfv)(id, GL_SPECULAR, light->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w.
  // w == 0 implies light is at infinity
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_scene_root());
  LVector3f dir = light->get_direction() * light_mat;
  LPoint4f fdir(-dir[0], -dir[1], -dir[2], 0);
  GLP(Lightfv)(id, GL_POSITION, fdir.get_data());

  // GL_SPOT_DIRECTION is not significant when cutoff == 180
  // In this case, position x, y, z specifies direction

  // Exponent == 0 implies uniform light distribution
  GLP(Lightf)(id, GL_SPOT_EXPONENT, 0.0f);

  // Cutoff == 180 means uniform point light source
  GLP(Lightf)(id, GL_SPOT_CUTOFF, 180.0f);

  // Default attenuation values (only spotlight and point light can
  // modify these)
  GLP(Lightf)(id, GL_CONSTANT_ATTENUATION, 1.0f);
  GLP(Lightf)(id, GL_LINEAR_ATTENUATION, 0.0f);
  GLP(Lightf)(id, GL_QUADRATIC_ATTENUATION, 0.0f);

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
bind_light(Spotlight *light, int light_id) {
  Lens *lens = light->get_lens();
  nassertv(lens != (Lens *)NULL);

  GLenum id = get_light_id(light_id);
  static const Colorf black(0.0f, 0.0f, 0.0f, 1.0f);
  GLP(Lightfv)(id, GL_AMBIENT, black.get_data());
  GLP(Lightfv)(id, GL_DIFFUSE, light->get_color().get_data());
  GLP(Lightfv)(id, GL_SPECULAR, light->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_scene_root());
  LPoint3f pos = lens->get_nodal_point() * light_mat;
  LVector3f dir = lens->get_view_vector() * light_mat;

  LPoint4f fpos(pos[0], pos[1], pos[2], 1.0f);
  GLP(Lightfv)(id, GL_POSITION, fpos.get_data());
  GLP(Lightfv)(id, GL_SPOT_DIRECTION, dir.get_data());

  GLP(Lightf)(id, GL_SPOT_EXPONENT, light->get_exponent());
  GLP(Lightf)(id, GL_SPOT_CUTOFF, lens->get_hfov());

  const LVecBase3f &att = light->get_attenuation();
  GLP(Lightf)(id, GL_CONSTANT_ATTENUATION, att[0]);
  GLP(Lightf)(id, GL_LINEAR_ATTENUATION, att[1]);
  GLP(Lightf)(id, GL_QUADRATIC_ATTENUATION, att[2]);

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::wants_normals
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
wants_normals() const {
  return (_lighting_enabled || _normals_enabled);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::wants_texcoords
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
wants_texcoords() const {
  return _texturing_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
depth_offset_decals() {
  return CLP(depth_offset_decals);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_internal_coordinate_system
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return the
//               coordinate system used internally by the GSG, if any
//               one particular coordinate system is used.  The
//               default, CS_default, indicates that the GSG can use
//               any coordinate system.
//
//               If this returns other than CS_default, the
//               GraphicsEngine will automatically convert all
//               transforms into the indicated coordinate system.
////////////////////////////////////////////////////////////////////
CoordinateSystem CLP(GraphicsStateGuardian)::
get_internal_coordinate_system() const {
  return CS_yup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::compute_distance_to
//       Access: Public, Virtual
//  Description: This function may only be called during a render
//               traversal; it will compute the distance to the
//               indicated point, assumed to be in modelview
//               coordinates, from the camera plane.
////////////////////////////////////////////////////////////////////
float CLP(GraphicsStateGuardian)::
compute_distance_to(const LPoint3f &point) const {
  // In the case of a CLP(GraphicsStateGuardian), we know that the
  // modelview matrix already includes the relative transform from the
  // camera, as well as a to-y-up conversion.  Thus, the distance to
  // the camera plane is simply the -z distance.

  return -point[2];
}

////////////////////////////////////////////////////////////////////
//     Function: report_errors_loop
//       Access: Protected, Static
//  Description: The internal implementation of report_errors().
//               Don't call this function; use report_errors()
//               instead.  The return value is true if everything is
//               ok, or false if we should shut down.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
report_errors_loop(int line, const char *source_file, GLenum error_code,
                   int &error_count) {
#ifndef NDEBUG
  static const int max_gl_errors_reported = 20;

  while ((error_count < max_gl_errors_reported) && 
         (error_code != GL_NO_ERROR)) {
    const GLubyte *error_string = GLUP(ErrorString)(error_code);
    if (error_string != (const GLubyte *)NULL) {
      GLCAT.error()
        << "at " << line << " of " << source_file << ": " 
        << error_string << "\n";
    } else {
      GLCAT.error()
        << "at " << line << " of " << source_file << ": " 
        << "GL error " << (int)error_code << "\n";
    }
    error_code = GLP(GetError)();
    error_count++;
  }

  return (error_code == GL_NO_ERROR);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::show_gl_string
//       Access: Protected
//  Description: Outputs the result of glGetString() on the indicated
//               tag.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
show_gl_string(const string &name, GLenum id) {
  if (GLCAT.is_debug()) {
    const GLubyte *text = GLP(GetString)(id);
    if (text == (const GLubyte *)NULL) {
      GLCAT.debug()
        << "Unable to query " << name << "\n";
    } else {
      GLCAT.debug()
        << name << " = " << (const char *)text << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::save_extensions
//       Access: Protected
//  Description: Separates the string returned by GL_EXTENSIONS (or
//               glx or wgl extensions) into its individual tokens
//               and saves them in the _extensions member.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
save_extensions(const char *extensions) {
  if (extensions != (const char *)NULL) {
    vector_string tokens;
    extract_words(extensions, tokens);
    
    vector_string::iterator ti;
    for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
      _extensions.insert(*ti);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_extra_extensions
//       Access: Protected, Virtual
//  Description: This may be redefined by a derived class (e.g. glx or
//               wgl) to get whatever further extensions strings may
//               be appropriate to that interface, in addition to the
//               GL extension strings return by glGetString().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
get_extra_extensions() {
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::report_extensions
//       Access: Protected
//  Description: Outputs the list of GL extensions to notify, if debug
//               mode is enabled.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
report_extensions() const {
  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "GL Extensions:\n";
    pset<string>::const_iterator ei;
    for (ei = _extensions.begin(); ei != _extensions.end(); ++ei) {
      GLCAT.debug() << (*ei) << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::has_extension
//       Access: Protected
//  Description: Returns true if the indicated extension is reported
//               by the GL system, false otherwise.  The extension
//               name is case-sensitive.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
has_extension(const string &extension) const {
  return (_extensions.find(extension) != _extensions.end());
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_draw_buffer
//       Access: Protected
//  Description: Sets up the GLP(DrawBuffer) to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
set_draw_buffer(const RenderBuffer &rb) {
  switch (rb._buffer_type & RenderBuffer::T_color) {
  case RenderBuffer::T_front:
    GLP(DrawBuffer)(GL_FRONT);
    break;

  case RenderBuffer::T_back:
    GLP(DrawBuffer)(GL_BACK);
    break;

  case RenderBuffer::T_right:
    GLP(DrawBuffer)(GL_RIGHT);
    break;

  case RenderBuffer::T_left:
    GLP(DrawBuffer)(GL_LEFT);
    break;

  case RenderBuffer::T_front_right:
    GLP(DrawBuffer)(GL_FRONT_RIGHT);
    break;

  case RenderBuffer::T_front_left:
    GLP(DrawBuffer)(GL_FRONT_LEFT);
    break;

  case RenderBuffer::T_back_right:
    GLP(DrawBuffer)(GL_BACK_RIGHT);
    break;

  case RenderBuffer::T_back_left:
    GLP(DrawBuffer)(GL_BACK_LEFT);
    break;

  default:
    GLP(DrawBuffer)(GL_FRONT_AND_BACK);
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_read_buffer
//       Access: Protected
//  Description: Sets up the GLP(ReadBuffer) to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
set_read_buffer(const RenderBuffer &rb) {
  switch (rb._buffer_type & RenderBuffer::T_color) {
  case RenderBuffer::T_front:
    GLP(ReadBuffer)(GL_FRONT);
    break;

  case RenderBuffer::T_back:
    GLP(ReadBuffer)(GL_BACK);
    break;

  case RenderBuffer::T_right:
    GLP(ReadBuffer)(GL_RIGHT);
    break;

  case RenderBuffer::T_left:
    GLP(ReadBuffer)(GL_LEFT);
    break;

  case RenderBuffer::T_front_right:
    GLP(ReadBuffer)(GL_FRONT_RIGHT);
    break;

  case RenderBuffer::T_front_left:
    GLP(ReadBuffer)(GL_FRONT_LEFT);
    break;

  case RenderBuffer::T_back_right:
    GLP(ReadBuffer)(GL_BACK_RIGHT);
    break;

  case RenderBuffer::T_back_left:
    GLP(ReadBuffer)(GL_BACK_LEFT);
    break;

  default:
    GLP(ReadBuffer)(GL_FRONT_AND_BACK);
  }
  report_my_gl_errors();
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::bind_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
bind_texture(TextureContext *tc) {
  CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);

#ifdef GSG_VERBOSE
  Texture *tex = tc->_texture;
  GLCAT.spam()
    << "glBindTexture(): " << tex->get_name() << "(" << (int)gtc->_index
    << ")" << endl;
#endif
  GLP(BindTexture)(GL_TEXTURE_2D, gtc->_index);
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::specify_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
specify_texture(Texture *tex) {
  GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  get_texture_wrap_mode(tex->get_wrapu()));
  GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  get_texture_wrap_mode(tex->get_wrapv()));

  if (CLP(force_mipmaps)) {
    GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  } else {
    GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    get_texture_filter_type(tex->get_minfilter()));
    GLP(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    get_texture_filter_type(tex->get_magfilter()));
  }
  report_my_gl_errors();
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: compute_gl_image_size
//  Description: Calculates how many bytes GL will expect to read for
//               a texture image, based on the number of pixels and
//               the GL format and type.  This is only used for
//               debugging.
////////////////////////////////////////////////////////////////////
static int
compute_gl_image_size(int xsize, int ysize, int external_format, int type) {
  int num_components = 0;
  switch (external_format) {
  case GL_COLOR_INDEX:
  case GL_STENCIL_INDEX:
  case GL_DEPTH_COMPONENT:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE:
  case GL_ALPHA:
  case GL_LUMINANCE:
    num_components = 1;
    break;

  case GL_LUMINANCE_ALPHA:
    num_components = 2;
    break;

  case GL_BGR:
  case GL_RGB:
    num_components = 3;
    break;

  case GL_BGRA:
  case GL_RGBA:
    num_components = 4;
    break;
  }

  int pixel_width = 0;
  switch (type) {
  case GL_UNSIGNED_BYTE:
    pixel_width = 1 * num_components;
    break;

  case GL_UNSIGNED_SHORT:
    pixel_width = 2 * num_components;
    break;

#ifdef GL_UNSIGNED_BYTE_3_3_2_EXT
  case GL_UNSIGNED_BYTE_3_3_2_EXT:
    nassertr(num_components == 3, 0);
    pixel_width = 1;
    break;
#endif

  case GL_FLOAT:
    pixel_width = 4 * num_components;
    break;
  }

  return xsize * ysize * pixel_width;
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::apply_texture_immediate
//       Access: Protected
//  Description: Sends the texture image to GL.  This can be used to
//               render a texture in immediate mode, or as part of the
//               process of creating a GL texture object.
//
//               The return value is true if successful, or false if
//               the texture has no image.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
apply_texture_immediate(Texture *tex) {
  PixelBuffer *pb = tex->get_ram_image();
  if (pb == (PixelBuffer *)NULL) {
    return false;
  }

  int xsize = pb->get_xsize();
  int ysize = pb->get_ysize();

  GLenum internal_format = get_internal_image_format(pb->get_format());
  GLenum external_format = get_external_image_format(pb->get_format());
  GLenum type = get_image_type(pb->get_image_type());

  PTA_uchar image = pb->_image;
  if (!_supports_bgr) {
    // If the GL doesn't claim to support BGR, we may have to reverse
    // the component ordering of the image.
    image = fix_component_ordering(external_format, pb);
  }

#ifndef NDEBUG
  int wanted_size = 
    compute_gl_image_size(xsize, ysize, external_format, type);
  nassertr(wanted_size == (int)pb->_image.size(), false);
#endif  // NDEBUG

  set_unpack_alignment(1);

#ifdef GSG_VERBOSE
  GLCAT.debug()
    << "glTexImage2D(GL_TEXTURE_2D, "
    << (int)internal_format << ", "
    << xsize << ", " << ysize << ", "
    << pb->get_border() << ", " << (int)external_format << ", "
    << (int)type << ", " << tex->get_name() << ")\n";
#endif

  if (!CLP(ignore_mipmaps) || CLP(force_mipmaps)) {
    if (tex->uses_mipmaps() || CLP(force_mipmaps)) {
#ifndef NDEBUG
      if (CLP(show_mipmaps)) {
        build_phony_mipmaps(tex);
      } else 
#endif
        {
          GLUP(Build2DMipmaps)(GL_TEXTURE_2D, internal_format,
                            xsize, ysize,
                            external_format, type, image);
#ifndef NDEBUG
          if (CLP(save_mipmaps)) {
            save_mipmap_images(tex);
          }
#endif
        }
      report_my_gl_errors();

      return true;
    }
  }

  nassertr(!pb->_image.empty(), false);
  GLP(TexImage2D)(GL_TEXTURE_2D, 0, internal_format,
               xsize, ysize, pb->get_border(),
               external_format, type, image);

  //report_my_gl_errors();
  // want to give explict error for texture creation failure
  GLenum error_code = GLP(GetError)();
  if(error_code != GL_NO_ERROR) {
    const GLubyte *error_string = GLUP(ErrorString)(error_code);
    GLCAT.error() << "GL texture creation failed for " << tex->get_name() << 
                        ((error_string != (const GLubyte *)NULL) ? " : " : "") << endl;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_texture
//       Access: Protected
//  Description: Copies the texture image directly onto the frame
//               buffer within the indicated display region.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_texture(TextureContext *tc, const DisplayRegion *dr) {
  nassertv(tc != NULL && dr != NULL);
  Texture *tex = tc->_texture;

  DisplayRegionStack old_dr = push_display_region(dr);
  prepare_display_region();

  static CPT(RenderState) basic_state;
  if (basic_state == (RenderState *)NULL) {
    // Create a State object for rendering textures in general.  Lots
    // of things get turned off.
    const RenderAttrib *attribs[] = {
      CullFaceAttrib::make(CullFaceAttrib::M_cull_none),
      DepthTestAttrib::make(DepthTestAttrib::M_none),
      DepthWriteAttrib::make(DepthWriteAttrib::M_off),
      TextureApplyAttrib::make(TextureApplyAttrib::M_decal),
      ColorWriteAttrib::make(ColorWriteAttrib::M_on),
      RenderModeAttrib::make(RenderModeAttrib::M_filled),
      //TexMatrixAttrib::make(LMatrix4f::ident_mat()),
      ColorBlendAttrib::make(ColorBlendAttrib::M_none),
      TransparencyAttrib::make(TransparencyAttrib::M_none),
    };
    basic_state = 
      RenderState::make(attribs, sizeof(attribs) / sizeof(void *));
  }      
  CPT(RenderState) state = basic_state->compose
    (RenderState::make(TextureAttrib::make(tex)));
  modify_state(state);
  set_transform(TransformState::make_identity());

  // We set up an orthographic projection that defines our entire
  // viewport to the range [0..1] in both dimensions.  Then, when we
  // create a unit square polygon below, it will exactly fill the
  // viewport (and thus exactly fill the display region).
  GLP(MatrixMode)(GL_PROJECTION);
  GLP(PushMatrix)();
  GLP(LoadIdentity)();
  GLUP(Ortho2D)(0, 1, 0, 1);

  float txl, txr, tyt, tyb;
  txl = tyb = 0.0f;
#if 0
 // remove this auto-scaling stuff for now
 // has_requested_size is only used here for draw_texture()
  if (tex->_has_requested_size) {
    txr = ((float)(tex->_requested_w)) / ((float)(tex->_pbuffer->get_xsize()));
    tyt = ((float)(tex->_requested_h)) / ((float)(tex->_pbuffer->get_ysize()));
  } else 
#endif     
  {
    txr = tyt = 1.0f;
  }

  // This two-triangle strip is actually a quad.  But it's usually
  // better to render quads as tristrips anyway.
  GLP(Begin)(GL_TRIANGLE_STRIP);
  GLP(TexCoord2f)(txl, tyb);   GLP(Vertex2i)(0, 0);
  GLP(TexCoord2f)(txr, tyb);   GLP(Vertex2i)(1, 0);
  GLP(TexCoord2f)(txl, tyt);   GLP(Vertex2i)(0, 1);
  GLP(TexCoord2f)(txr, tyt);   GLP(Vertex2i)(1, 1);
  GLP(End)();

  GLP(MatrixMode)(GL_PROJECTION);
  GLP(PopMatrix)();

  pop_display_region(old_dr);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_texture(TextureContext *tc, const DisplayRegion *dr, 
             const RenderBuffer &rb) {
  set_draw_buffer(rb);
  draw_texture(tc, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_pixel_buffer
//       Access: Protected
//  Description: Copies the indicated pixel buffer into the frame
//               buffer in the given display region.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr) {
  // This code is now invalidated by the new design; perhaps the
  // interface is not needed anyway.
#if 0
  nassertv(pb != NULL && dr != NULL);
  nassertv(!pb->_image.empty());
  DisplayRegionStack old_dr = push_display_region(dr);
  prepare_display_region();

  static CPT(RenderState) depth_state;
  static CPT(RenderState) color_state;
  if (depth_state == (RenderState *)NULL) {
    // Create a State object for rendering depth-mask pixel buffers.
    depth_state = 
      RenderState::make(TextureAttrib::make_off(),
                        ColorWriteAttrib::make(ColorWriteAttrib::M_off),
                        DepthTestAttrib::make(DepthTestAttrib::M_always),
                        DepthWriteAttrib::make(DepthWriteAttrib::M_on));

    // And one for rendering color buffers.
    color_state =
      RenderState::make(TextureAttrib::make_off(),
                        ColorWriteAttrib::make(ColorWriteAttrib::M_on),
                        DepthTestAttrib::make(DepthTestAttrib::M_none),
                        DepthWriteAttrib::make(DepthWriteAttrib::M_off));
  }

  switch (pb->get_format()) {
  case PixelBuffer::F_depth_component:
    modify_state(depth_state);
    break;

  case PixelBuffer::F_rgb:
  case PixelBuffer::F_rgb5:
  case PixelBuffer::F_rgb8:
  case PixelBuffer::F_rgb12:
  case PixelBuffer::F_rgba:
  case PixelBuffer::F_rgbm:
  case PixelBuffer::F_rgba4:
  case PixelBuffer::F_rgba5:
  case PixelBuffer::F_rgba8:
  case PixelBuffer::F_rgba12:
    modify_state(color_state);
    break;

  default:
    GLCAT.error()
      << "draw_pixel_buffer(): unknown buffer format" << endl;
  }
  set_transform(TransformState::make_identity());

  set_unpack_alignment(1);

  WindowProperties props = _win->get_properties();

  GLP(MatrixMode)( GL_PROJECTION );
  GLP(PushMatrix)();
  GLP(LoadIdentity)();
  GLUP(Ortho2D)(0, props.get_x_size(),
                0, props.get_y_size());

#ifdef GSG_VERBOSE
  GLCAT.debug()
    << "glDrawPixels(" << pb->get_xsize() << ", " << pb->get_ysize()
    << ", ";
  switch (get_external_image_format(pb->get_format())) {
  case GL_DEPTH_COMPONENT:
    GLCAT.debug(false) << "GL_DEPTH_COMPONENT, ";
    break;
  case GL_RGB:
    GLCAT.debug(false) << "GL_RGB, ";
    break;
  case GL_RGBA:
    GLCAT.debug(false) << "GL_RGBA, ";
    break;
  case GL_BGR:
    GLCAT.debug(false) << "GL_BGR, ";
    break;
  case GL_BGRA:
    GLCAT.debug(false) << "GL_BGRA, ";
    break;
  default:
    GLCAT.debug(false) << "unknown, ";
    break;
  }
  switch (get_image_type(pb->get_image_type())) {
  case GL_UNSIGNED_BYTE:
    GLCAT.debug(false) << "GL_UNSIGNED_BYTE, ";
    break;
  case GL_FLOAT:
    GLCAT.debug(false) << "GL_FLOAT, ";
    break;
  default:
    GLCAT.debug(false) << "unknown, ";
    break;
  }
  GLCAT.debug(false)
    << (void *)pb->_image.p() << ")" << endl;
#endif

  GLP(RasterPos2i)( pb->get_xorg(), pb->get_yorg() );
  GLP(DrawPixels)( pb->get_xsize(), pb->get_ysize(),
                get_external_image_format(pb->get_format()),
                get_image_type(pb->get_image_type()),
                pb->_image.p() );

  GLP(MatrixMode)( GL_PROJECTION );
  GLP(PopMatrix)();

  pop_display_region(old_dr);
  report_my_gl_errors();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::draw_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const RenderBuffer &rb) {
  set_draw_buffer(rb);
  draw_pixel_buffer(pb, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_texture_wrap_mode
//       Access: Protected
//  Description: Maps from the Texture's internal wrap mode symbols to
//               GL's.
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_texture_wrap_mode(Texture::WrapMode wm) {
  if (CLP(ignore_clamp)) {
    return GL_REPEAT;
  }
  switch (wm) {
  case Texture::WM_clamp:
    return GL_CLAMP;
  case Texture::WM_repeat:
    return GL_REPEAT;

  case Texture::WM_mirror:
  case Texture::WM_mirror_once:
  case Texture::WM_border_color:
    // These are unsupported for now.
    return GL_REPEAT;

  case Texture::WM_invalid:
    break;
  }
  GLCAT.error() << "Invalid Texture::WrapMode value!\n";
  return GL_CLAMP;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_texture_filter_type
//       Access: Protected
//  Description: Maps from the Texture's internal filter type symbols
//               to GL's.
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_texture_filter_type(Texture::FilterType ft) {
  if (CLP(ignore_filters)) {
    return GL_NEAREST;

  } else if (CLP(ignore_mipmaps)) {
    switch (ft) {
    case Texture::FT_nearest_mipmap_nearest:
    case Texture::FT_nearest:
      return GL_NEAREST;
    case Texture::FT_linear:
    case Texture::FT_linear_mipmap_nearest:
    case Texture::FT_nearest_mipmap_linear:
    case Texture::FT_linear_mipmap_linear:
      return GL_LINEAR;
    case Texture::FT_invalid:
      break;
    }

  } else {
    switch (ft) {
    case Texture::FT_nearest:
      return GL_NEAREST;
    case Texture::FT_linear:
      return GL_LINEAR;
    case Texture::FT_nearest_mipmap_nearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case Texture::FT_linear_mipmap_nearest:
      return GL_LINEAR_MIPMAP_NEAREST;
    case Texture::FT_nearest_mipmap_linear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case Texture::FT_linear_mipmap_linear:
      return GL_LINEAR_MIPMAP_LINEAR;
    case Texture::FT_invalid:
      break;
    }
  }
  GLCAT.error() << "Invalid Texture::FilterType value!\n";
  return GL_NEAREST;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_image_type
//       Access: Protected
//  Description: Maps from the PixelBuffer's internal Type symbols
//               to GL's.
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_image_type(PixelBuffer::Type type) {
  switch (type) {
  case PixelBuffer::T_unsigned_byte:
    return GL_UNSIGNED_BYTE;
  case PixelBuffer::T_unsigned_short:
    return GL_UNSIGNED_SHORT;
#ifdef GL_UNSIGNED_BYTE_3_3_2_EXT
  case PixelBuffer::T_unsigned_byte_332:
    return GL_UNSIGNED_BYTE_3_3_2_EXT;
#endif
  case PixelBuffer::T_float:
    return GL_FLOAT;

  default:
    GLCAT.error() << "Invalid PixelBuffer::Type value!\n";
    return GL_UNSIGNED_BYTE;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_external_image_format
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols
//               to GL's.
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_external_image_format(PixelBuffer::Format format) {
  switch (format) {
  case PixelBuffer::F_color_index:
    return GL_COLOR_INDEX;
  case PixelBuffer::F_stencil_index:
    return GL_STENCIL_INDEX;
  case PixelBuffer::F_depth_component:
    return GL_DEPTH_COMPONENT;
  case PixelBuffer::F_red:
    return GL_RED;
  case PixelBuffer::F_green:
    return GL_GREEN;
  case PixelBuffer::F_blue:
    return GL_BLUE;
  case PixelBuffer::F_alpha:
    return GL_ALPHA;
  case PixelBuffer::F_rgb:
  case PixelBuffer::F_rgb5:
  case PixelBuffer::F_rgb8:
  case PixelBuffer::F_rgb12:
  case PixelBuffer::F_rgb332:
    return _supports_bgr ? GL_BGR : GL_RGB;
  case PixelBuffer::F_rgba:
  case PixelBuffer::F_rgbm:
  case PixelBuffer::F_rgba4:
  case PixelBuffer::F_rgba5:
  case PixelBuffer::F_rgba8:
  case PixelBuffer::F_rgba12:
    return _supports_bgr ? GL_BGRA : GL_RGBA;
  case PixelBuffer::F_luminance:
    return GL_LUMINANCE;
  case PixelBuffer::F_luminance_alphamask:
  case PixelBuffer::F_luminance_alpha:
    return GL_LUMINANCE_ALPHA;
  }
  GLCAT.error()
    << "Invalid PixelBuffer::Format value in get_external_image_format(): "
    << (int)format << "\n";
  return GL_RGB;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_internal_image_format
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols to a
//               suitable internal format for GL textures.
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_internal_image_format(PixelBuffer::Format format) {
  switch (format) {
  case PixelBuffer::F_rgba:
  case PixelBuffer::F_rgbm:
    return GL_RGBA;
  case PixelBuffer::F_rgba4:
    return GL_RGBA4;
  case PixelBuffer::F_rgba8:
    return GL_RGBA8;
  case PixelBuffer::F_rgba12:
    return GL_RGBA12;

  case PixelBuffer::F_rgb:
    return GL_RGB;
  case PixelBuffer::F_rgb5:
    return GL_RGB5;
  case PixelBuffer::F_rgba5:
    return GL_RGB5_A1;
  case PixelBuffer::F_rgb8:
    return GL_RGB8;
  case PixelBuffer::F_rgb12:
    return GL_RGB12;
  case PixelBuffer::F_rgb332:
    return GL_R3_G3_B2;

  case PixelBuffer::F_alpha:
    return GL_ALPHA;

  case PixelBuffer::F_red:
  case PixelBuffer::F_green:
  case PixelBuffer::F_blue:
  case PixelBuffer::F_luminance:
    return GL_LUMINANCE;
  case PixelBuffer::F_luminance_alpha:
  case PixelBuffer::F_luminance_alphamask:
    return GL_LUMINANCE_ALPHA;

  default:
    GLCAT.error()
      << "Invalid image format in get_internal_image_format(): "
      << (int)format << "\n";
    return GL_RGB;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_texture_apply_mode_type
//       Access: Protected
//  Description: Maps from the texture environment's mode types
//       to the corresponding OpenGL ids
////////////////////////////////////////////////////////////////////
GLint CLP(GraphicsStateGuardian)::
get_texture_apply_mode_type(TextureApplyAttrib::Mode am) const {
  if (CLP(always_decal_textures)) {
    return GL_DECAL;
  }
  switch (am) {
  case TextureApplyAttrib::M_modulate: return GL_MODULATE;
  case TextureApplyAttrib::M_decal: return GL_DECAL;
  case TextureApplyAttrib::M_blend: return GL_BLEND;
  case TextureApplyAttrib::M_replace: return GL_REPLACE;
  case TextureApplyAttrib::M_add: return GL_ADD;
  }
  GLCAT.error()
    << "Invalid TextureApplyAttrib::Mode value" << endl;
  return GL_MODULATE;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_fog_mode_type
//       Access: Protected
//  Description: Maps from the fog types to gl version
////////////////////////////////////////////////////////////////////
GLenum CLP(GraphicsStateGuardian)::
get_fog_mode_type(Fog::Mode m) const {
  switch(m) {
  case Fog::M_linear: return GL_LINEAR;
  case Fog::M_exponential: return GL_EXP;
  case Fog::M_exponential_squared: return GL_EXP2;
    /*
      #ifdef GL_FOG_FUNC_SGIS
      case Fog::M_spline: return GL_FOG_FUNC_SGIS;
      #endif
    */

  default:
    GLCAT.error() << "Invalid Fog::Mode value" << endl;
    return GL_EXP;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::print_gfx_visual
//       Access: Public
//  Description: Prints a description of the current visual selected.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
print_gfx_visual() {
  GLint i;
  GLboolean j;
  cout << "Graphics Visual Info (# bits of each):" << endl;

  cout << "RGBA: ";
  GLP(GetIntegerv)( GL_RED_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_GREEN_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_BLUE_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_ALPHA_BITS, &i ); cout << i << endl;

  cout << "Accum RGBA: ";
  GLP(GetIntegerv)( GL_ACCUM_RED_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_ACCUM_GREEN_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_ACCUM_BLUE_BITS, &i ); cout << i << " ";
  GLP(GetIntegerv)( GL_ACCUM_ALPHA_BITS, &i ); cout << i << endl;

  GLP(GetIntegerv)( GL_INDEX_BITS, &i ); cout << "Color Index: " << i << endl;

  GLP(GetIntegerv)( GL_DEPTH_BITS, &i ); cout << "Depth: " << i << endl;
  GLP(GetIntegerv)( GL_ALPHA_BITS, &i ); cout << "Alpha: " << i << endl;
  GLP(GetIntegerv)( GL_STENCIL_BITS, &i ); cout << "Stencil: " << i << endl;

  GLP(GetBooleanv)( GL_DOUBLEBUFFER, &j ); cout << "DoubleBuffer? "
                                             << (int)j << endl;

  GLP(GetBooleanv)( GL_STEREO, &j ); cout << "Stereo? " << (int)j << endl;

#ifdef GL_MULTISAMPLE_SGIS
  GLP(GetBooleanv)( GL_MULTISAMPLE_SGIS, &j ); cout << "Multisample? "
                                                 << (int)j << endl;
#endif
#ifdef GL_SAMPLES_SGIS
  GLP(GetIntegerv)( GL_SAMPLES_SGIS, &i ); cout << "Samples: " << i << endl;
#endif

  GLP(GetBooleanv)( GL_BLEND, &j ); cout << "Blend? " << (int)j << endl;
  GLP(GetBooleanv)( GL_POINT_SMOOTH, &j ); cout << "Point Smooth? "
                                             << (int)j << endl;
  GLP(GetBooleanv)( GL_LINE_SMOOTH, &j ); cout << "Line Smooth? "
                                            << (int)j << endl;

  GLP(GetIntegerv)( GL_AUX_BUFFERS, &i ); cout << "Aux Buffers: " << i << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_transformed_color
//       Access: Public
//  Description: Transform the color by the current color matrix, and
//               calls the appropriate glColor function.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
issue_transformed_color(const Colorf &color) const {
  Colorf transformed
    ((color[0] * _current_color_scale[0]) + _current_color_offset[0],
     (color[1] * _current_color_scale[1]) + _current_color_offset[1],
     (color[2] * _current_color_scale[2]) + _current_color_offset[2],
     (color[3] * _current_color_scale[3]) + _current_color_offset[3]);

  GLP(Color4fv)(transformed.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::slot_new_light
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular light id will be used for the first time.
//               It is intended to allow the derived class to reserve
//               any additional resources, if required, for the new
//               light; and also to indicate whether the hardware
//               supports this many simultaneous lights.
//
//               The return value should be true if the additional
//               light is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
slot_new_light(int light_id) {
  return (light_id < _max_lights);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
enable_lighting(bool enable) {
  if (enable) {
    GLP(Enable)(GL_LIGHTING);
  } else {
    GLP(Disable)(GL_LIGHTING);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
set_ambient_light(const Colorf &color) {
  GLP(LightModelfv)(GL_LIGHT_MODEL_AMBIENT, color.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
enable_light(int light_id, bool enable) {
  if (enable) {
    GLP(Enable)(get_light_id(light_id));
  } else {
    GLP(Disable)(get_light_id(light_id));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::begin_bind_lights
//       Access: Protected, Virtual
//  Description: Called immediately before bind_light() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several lights.
//
//               The sequence is: begin_bind_lights() will be called,
//               then one or more bind_light() calls, then
//               end_bind_lights().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
begin_bind_lights() {
  // We need to temporarily load a new matrix so we can define the
  // light in a known coordinate system.  We pick the transform of the
  // root.  (Alternatively, we could leave the current transform where
  // it is and compute the light position relative to that transform
  // instead of relative to the root, by composing with the matrix
  // computed by _transform->invert_compose(render_transform).  But I
  // think loading a completely new matrix is simpler.)
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(PushMatrix)();
  GLP(LoadMatrixf)(_scene_setup->get_render_transform()->get_mat().get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::end_bind_lights
//       Access: Protected, Virtual
//  Description: Called after before bind_light() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_lights().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
end_bind_lights() {
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(PopMatrix)();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::slot_new_clip_plane
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular clip plane id will be used for the first
//               time.  It is intended to allow the derived class to
//               reserve any additional resources, if required, for
//               the new clip plane; and also to indicate whether the
//               hardware supports this many simultaneous clipping
//               planes.
//
//               The return value should be true if the additional
//               plane is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsStateGuardian)::
slot_new_clip_plane(int plane_id) {
  return (plane_id < _max_clip_planes);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated clip_plane id.  A specific
//               PlaneNode will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
enable_clip_plane(int plane_id, bool enable) {
  if (enable) {
    GLP(Enable)(get_clip_plane_id(plane_id));
  } else {
    GLP(Disable)(get_clip_plane_id(plane_id));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::begin_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called immediately before bind_clip_plane() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several clip_planes.
//
//               The sequence is: begin_bind_clip_planes() will be called,
//               then one or more bind_clip_plane() calls, then
//               end_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
begin_bind_clip_planes() {
  // We need to temporarily load a new matrix so we can define the
  // clip_plane in a known coordinate system.  We pick the transform of the
  // root.  (Alternatively, we could leave the current transform where
  // it is and compute the clip_plane position relative to that transform
  // instead of relative to the root, by composing with the matrix
  // computed by _transform->invert_compose(render_transform).  But I
  // think loading a completely new matrix is simpler.)
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(PushMatrix)();
  GLP(LoadMatrixf)(_scene_setup->get_render_transform()->get_mat().get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::bind_clip_plane
//       Access: Protected, Virtual
//  Description: Called the first time a particular clip_plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware clip_plane with the clip_plane's
//               properties.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
bind_clip_plane(PlaneNode *plane, int plane_id) {
  GLenum id = get_clip_plane_id(plane_id);

  NodePath plane_np(plane);
  const LMatrix4f &plane_mat = plane_np.get_mat(_scene_setup->get_scene_root());
  Planef xformed_plane = plane->get_plane() * plane_mat;

  Planed double_plane(LCAST(double, xformed_plane));
  GLP(ClipPlane)(id, double_plane.get_data());

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::end_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called after before bind_clip_plane() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
end_bind_clip_planes() {
  GLP(MatrixMode)(GL_MODELVIEW);
  GLP(PopMatrix)();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of these three blending states have
//               changed; this function is responsible for setting the
//               appropriate color blending mode based on the given
//               properties.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
set_blend_mode(ColorWriteAttrib::Mode color_write_mode,
               ColorBlendAttrib::Mode color_blend_mode,
               TransparencyAttrib::Mode transparency_mode) {
  // If color_write_mode is off, we disable writing to the color using
  // blending.  This case is only used if we can't use GLP(ColorMask) to
  // disable the color writing for some reason (usually a driver
  // problem).
  if (color_write_mode == ColorWriteAttrib::M_off) {
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    call_glBlendFunc(GL_ZERO, GL_ONE);
    return;
  }

  // Is there a color blend set?
  switch (color_blend_mode) {
  case ColorBlendAttrib::M_none:
    break;

  case ColorBlendAttrib::M_multiply:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    call_glBlendFunc(GL_DST_COLOR, GL_ZERO);
    return;

  case ColorBlendAttrib::M_add:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    call_glBlendFunc(GL_ONE, GL_ONE);
    return;

  case ColorBlendAttrib::M_multiply_add:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    call_glBlendFunc(GL_DST_COLOR, GL_ONE);
    return;

  default:
    GLCAT.error()
      << "Unknown color blend mode " << (int)color_blend_mode << endl;
    break;
  }

  // No color blend; is there a transparency set?
  switch (transparency_mode) {
      case TransparencyAttrib::M_none:
      case TransparencyAttrib::M_binary:
        break;
    
      case TransparencyAttrib::M_alpha:
      case TransparencyAttrib::M_alpha_sorted:
      case TransparencyAttrib::M_dual:
        // Should we really have an "alpha" and an "alpha_sorted"
        // mode, like Performer does?  (The difference is that
        // "alpha_sorted" is with the write to the depth buffer
        // disabled.)  Or should we just use the separate depth write
        // transition to control this?  Doing it implicitly requires a
        // bit more logic here and in the state management; for now we
        // require the user to explicitly turn off the depth write.
        enable_multisample_alpha_one(false);
        enable_multisample_alpha_mask(false);
        enable_blend(true);
        call_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return;
    
      case TransparencyAttrib::M_multisample:
        enable_multisample_alpha_one(true);
        enable_multisample_alpha_mask(true);
        enable_blend(false);
        return;
    
      case TransparencyAttrib::M_multisample_mask:
        enable_multisample_alpha_one(false);
        enable_multisample_alpha_mask(true);
        enable_blend(false);
        return;
    
      default:
        GLCAT.error()
          << "invalid transparency mode " << (int)transparency_mode << endl;
        break;
  }

  // Nothing's set, so disable blending.
  enable_multisample_alpha_one(false);
  enable_multisample_alpha_mask(false);
  enable_blend(false);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
free_pointers() {
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::save_frame_buffer
//       Access: Public
//  Description: Saves the indicated planes of the frame buffer
//               (within the indicated display region) and returns it
//               in some meaningful form that can be restored later
//               via restore_frame_buffer().  This is a helper
//               function for push_frame_buffer() and
//               pop_frame_buffer().
////////////////////////////////////////////////////////////////////
PT(SavedFrameBuffer) CLP(GraphicsStateGuardian)::
save_frame_buffer(const RenderBuffer &buffer,
                  CPT(DisplayRegion) dr) {
  CLP(SavedFrameBuffer) *sfb = new CLP(SavedFrameBuffer)(buffer, dr);

  if (buffer._buffer_type & RenderBuffer::T_depth) {
    // Save the depth buffer.
    sfb->_depth =
      new PixelBuffer(PixelBuffer::depth_buffer(dr->get_pixel_width(),
                                                dr->get_pixel_height()));
    copy_pixel_buffer(sfb->_depth, dr, buffer);
  }

  if (buffer._buffer_type & RenderBuffer::T_back) {
    // Save the color buffer.
    sfb->_back_rgba = new Texture;
    TextureContext *tc = sfb->_back_rgba->prepare_now(_prepared_objects, this);
    copy_texture(tc, dr, buffer);
  }

  return sfb;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::restore_frame_buffer
//       Access: Public
//  Description: Restores the frame buffer that was previously saved.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
restore_frame_buffer(SavedFrameBuffer *frame_buffer) {
  CLP(SavedFrameBuffer) *sfb = DCAST(CLP(SavedFrameBuffer), frame_buffer);

  if (sfb->_back_rgba != (Texture *)NULL &&
      (sfb->_buffer._buffer_type & RenderBuffer::T_back) != 0) {
    // Restore the color buffer.
    TextureContext *tc = sfb->_back_rgba->prepare_now(_prepared_objects, this);
    draw_texture(tc, sfb->_display_region, sfb->_buffer);
  }

  if (sfb->_depth != (PixelBuffer *)NULL &&
      (sfb->_buffer._buffer_type & RenderBuffer::T_depth) != 0) {
    // Restore the depth buffer.
    draw_pixel_buffer(sfb->_depth, sfb->_display_region, sfb->_buffer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_untextured_state
//       Access: Protected, Static
//  Description: Returns a RenderState object that represents
//               texturing off.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CLP(GraphicsStateGuardian)::
get_untextured_state() {
  static CPT(RenderState) state;
  if (state == (RenderState *)NULL) {
    state = RenderState::make(TextureAttrib::make_off());
  }
  return state;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::build_phony_mipmaps
//       Access: Protected
//  Description: Generates a series of colored mipmap levels to aid in
//               visualizing the mipmap levels as the hardware applies
//               them.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
build_phony_mipmaps(Texture *tex) {
  PixelBuffer *pb = tex->_pbuffer;
  int xsize = pb->get_xsize();
  int ysize = pb->get_ysize();

  GLCAT.info()
    << "Building phony mipmap levels for " << tex->get_name() << "\n";
  int level = 0;
  while (xsize > 0 && ysize > 0) {
    GLCAT.info(false)
      << "  level " << level << " is " << xsize << " by " << ysize << "\n";
    build_phony_mipmap_level(level, xsize, ysize);

    xsize >>= 1;
    ysize >>= 1;
    level++;
  }

  while (xsize > 0) {
    GLCAT.info(false)
      << "  level " << level << " is " << xsize << " by 1\n";
    build_phony_mipmap_level(level, xsize, 1);

    xsize >>= 1;
    level++;
  }

  while (ysize > 0) {
    GLCAT.info(false)
      << "  level " << level << " is 1 by " << ysize << "\n";
    build_phony_mipmap_level(level, 1, ysize);

    ysize >>= 1;
    level++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::build_phony_mipmap_level
//       Access: Protected
//  Description: Generates a single colored mipmap level.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
build_phony_mipmap_level(int level, int xsize, int ysize) {
  static const int num_levels = 10;
  static const char *level_filenames[num_levels] = {
    "mipmap_level_0.rgb",
    "mipmap_level_1.rgb",
    "mipmap_level_2.rgb",
    "mipmap_level_3.rgb",
    "mipmap_level_4.rgb",
    "mipmap_level_5.rgb",
    "mipmap_level_6.rgb",
    "mipmap_level_7.rgb",
    "mipmap_level_8.rgb",
    "mipmap_level_9.rgb"
  };
  static const RGBColorf level_colors[num_levels] = {
    RGBColorf(1.0f, 1.0f, 1.0f),
    RGBColorf(1.0f, 0.0f, 0.0f),
    RGBColorf(0.0f, 1.0f, 0.0f),
    RGBColorf(0.0f, 0.0f, 1.0f),
    RGBColorf(1.0f, 1.0f, 0.0f),
    RGBColorf(0.0f, 1.0f, 1.0f),
    RGBColorf(1.0f, 0.0f, 1.0f),
    RGBColorf(1.0f, 0.5, 0.0f),
    RGBColorf(0.0f, 1.0f, 0.5),
    RGBColorf(0.83, 0.71, 1.0f)
  };

  level = level % num_levels;
  Filename filename(level_filenames[level]);

  PNMImage image_sized(xsize, ysize);
  PNMImage image_source;
  if (filename.resolve_filename(get_texture_path()) ||
      filename.resolve_filename(get_model_path())) {
    image_source.read(filename);
  }

  if (image_source.is_valid()) {
    image_sized.quick_filter_from(image_source);

  } else {
    GLCAT.info(false)
      << "    " << filename << " cannot be read, making solid color mipmap.\n";
    image_sized.fill(level_colors[level][0],
                     level_colors[level][1],
                     level_colors[level][2]);
  }

  PixelBuffer *pb = new PixelBuffer;
  pb->load(image_sized);

  GLenum internal_format = get_internal_image_format(pb->get_format());
  GLenum external_format = get_external_image_format(pb->get_format());
  GLenum type = get_image_type(pb->get_image_type());

  GLP(TexImage2D)(GL_TEXTURE_2D, level, internal_format,
               pb->get_xsize(), pb->get_ysize(), pb->get_border(),
               external_format, type, pb->_image );

  delete pb;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::save_mipmap_images
//       Access: Protected
//  Description: Saves out each mipmap level of the indicated texture
//               (which must also be the currently active texture in
//               the GL state) as a separate image file to disk.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsStateGuardian)::
save_mipmap_images(Texture *tex) {
  Filename filename = tex->get_name();
  string name;
  if (filename.empty()) {
    static int index = 0;
    name = "texture" + format_string(index);
    index++;
  } else {
    name = filename.get_basename_wo_extension();
  }

  PixelBuffer *pb = tex->get_ram_image();
  nassertv(pb != (PixelBuffer *)NULL);

  GLenum external_format = get_external_image_format(pb->get_format());
  GLenum type = get_image_type(pb->get_image_type());

  int xsize = pb->get_xsize();
  int ysize = pb->get_ysize();

  // Specify byte-alignment for the pixels on output.
  set_pack_alignment(1);

  int mipmap_level = 0;
  do {
    xsize = max(xsize, 1);
    ysize = max(ysize, 1);

    PT(PixelBuffer) mpb = 
      new PixelBuffer(xsize, ysize, pb->get_num_components(),
                      pb->get_component_width(), pb->get_image_type(),
                      pb->get_format());
    GLP(GetTexImage)(GL_TEXTURE_2D, mipmap_level, external_format, 
                  type, mpb->_image);
    Filename mipmap_filename = name + "_" + format_string(mipmap_level) + ".rgb";
    nout << "Writing mipmap level " << mipmap_level
         << " (" << xsize << " by " << ysize << ") " 
         << mipmap_filename << "\n";
    mpb->write(mipmap_filename);

    xsize >>= 1;
    ysize >>= 1;
    mipmap_level++;
  } while (xsize > 0 && ysize > 0);
}
#endif  // NDEBUG

TypeHandle CLP(GraphicsStateGuardian)::get_type(void) const {
  return get_class_type();
}

TypeHandle CLP(GraphicsStateGuardian)::get_class_type(void) {
  return _type_handle;
}

void CLP(GraphicsStateGuardian)::init_type(void) {
  GraphicsStateGuardian::init_type();
  register_type(_type_handle, CLASSPREFIX_QUOTED "GraphicsStateGuardian",
                GraphicsStateGuardian::get_class_type());
}


#ifdef GSG_VERBOSE

void CLP(GraphicsStateGuardian)::
dump_state(void)
{
  if (GLCAT.is_debug())
    {
      ostream &dump = GLCAT.debug(false);
      GLCAT.debug() << "Dumping GL State" << endl;

      dump << "\t\t" << "GL_LINE_SMOOTH " << _line_smooth_enabled << " " << (bool)GLP(IsEnabled)(GL_LINE_SMOOTH) << "\n";
      dump << "\t\t" << "GL_POINT_SMOOTH " << _point_smooth_enabled << " " << (bool)GLP(IsEnabled)(GL_POINT_SMOOTH) << "\n";
      dump << "\t\t" << "GL_LIGHTING " << _lighting_enabled << " " << (bool)GLP(IsEnabled)(GL_LIGHTING) << "\n";
      dump << "\t\t" << "GL_SCISSOR_TEST " << _scissor_enabled << " " << (bool)GLP(IsEnabled)(GL_SCISSOR_TEST) << "\n";
      dump << "\t\t" << "GL_TEXTURE_2D " << _texturing_enabled << " " << (bool)GLP(IsEnabled)(GL_TEXTURE_2D) << "\n";
      dump << "\t\t" << "GL_STENCIL_TEST " << " " << (bool)GLP(IsEnabled)(GL_STENCIL_TEST) << "\n";
      dump << "\t\t" << "GL_BLEND " << _blend_enabled << " " << (bool)GLP(IsEnabled)(GL_BLEND) << "\n";
      dump << "\t\t" << "GL_DEPTH_TEST " << _depth_test_enabled << " " << (bool)GLP(IsEnabled)(GL_DEPTH_TEST) << "\n";
      dump << "\t\t" << "GL_FOG " << _fog_enabled << " " << (bool)GLP(IsEnabled)(GL_FOG) << "\n";
      dump << "\t\t" << "GL_ALPHA_TEST " << _alpha_test_enabled << " " << (bool)GLP(IsEnabled)(GL_ALPHA_TEST) << "\n";
      dump << "\t\t" << "GL_POLYGON_OFFSET_FILL " << _polygon_offset_enabled << " " << (bool)GLP(IsEnabled)(GL_POLYGON_OFFSET_FILL) << "\n";

      dump << endl;
    }
}

#else  // GSG_VERBOSE

// This function does nothing unless GSG_VERBOSE is compiled in.
void CLP(GraphicsStateGuardian)::
dump_state(void)
{
}

#endif  // GSG_VERBOSE


#ifdef GSG_VERBOSE

// This is a handy function to output a GLenum value as a string, for
// debugging.
ostream &output_gl_enum(ostream &out, GLenum v) {
  switch (v) {
  case GL_FALSE:
    return out << "GL_FALSE";
  case GL_TRUE:
    return out << "GL_TRUE";

    /* Data types */
  case GL_BYTE:
    return out << "GL_BYTE";
  case GL_UNSIGNED_BYTE:
    return out << "GL_UNSIGNED_BYTE";
  case GL_SHORT:
    return out << "GL_SHORT";
  case GL_UNSIGNED_SHORT:
    return out << "GL_UNSIGNED_SHORT";
  case GL_INT:
    return out << "GL_INT";
  case GL_UNSIGNED_INT:
    return out << "GL_UNSIGNED_INT";
  case GL_FLOAT:
    return out << "GL_FLOAT";
  case GL_DOUBLE:
    return out << "GL_DOUBLE";
  case GL_2_BYTES:
    return out << "GL_2_BYTES";
  case GL_3_BYTES:
    return out << "GL_3_BYTES";
  case GL_4_BYTES:
    return out << "GL_4_BYTES";

    /* Primitives */
    /*
      case GL_LINES:
      return out << "GL_LINES";
      case GL_POINTS:
      return out << "GL_POINTS";
    */
  case GL_LINE_STRIP:
    return out << "GL_LINE_STRIP";
  case GL_LINE_LOOP:
    return out << "GL_LINE_LOOP";
  case GL_TRIANGLES:
    return out << "GL_TRIANGLES";
  case GL_TRIANGLE_STRIP:
    return out << "GL_TRIANGLE_STRIP";
  case GL_TRIANGLE_FAN:
    return out << "GL_TRIANGLE_FAN";
  case GL_QUADS:
    return out << "GL_QUADS";
  case GL_QUAD_STRIP:
    return out << "GL_QUAD_STRIP";
  case GL_POLYGON:
    return out << "GL_POLYGON";
  case GL_EDGE_FLAG:
    return out << "GL_EDGE_FLAG";

    /* Vertex Arrays */
  case GL_VERTEX_ARRAY:
    return out << "GL_VERTEX_ARRAY";
  case GL_NORMAL_ARRAY:
    return out << "GL_NORMAL_ARRAY";
  case GL_COLOR_ARRAY:
    return out << "GL_COLOR_ARRAY";
  case GL_INDEX_ARRAY:
    return out << "GL_INDEX_ARRAY";
  case GL_TEXTURE_COORD_ARRAY:
    return out << "GL_TEXTURE_COORD_ARRAY";
  case GL_EDGE_FLAG_ARRAY:
    return out << "GL_EDGE_FLAG_ARRAY";
  case GL_VERTEX_ARRAY_SIZE:
    return out << "GL_VERTEX_ARRAY_SIZE";
  case GL_VERTEX_ARRAY_TYPE:
    return out << "GL_VERTEX_ARRAY_TYPE";
  case GL_VERTEX_ARRAY_STRIDE:
    return out << "GL_VERTEX_ARRAY_STRIDE";
  case GL_NORMAL_ARRAY_TYPE:
    return out << "GL_NORMAL_ARRAY_TYPE";
  case GL_NORMAL_ARRAY_STRIDE:
    return out << "GL_NORMAL_ARRAY_STRIDE";
  case GL_COLOR_ARRAY_SIZE:
    return out << "GL_COLOR_ARRAY_SIZE";
  case GL_COLOR_ARRAY_TYPE:
    return out << "GL_COLOR_ARRAY_TYPE";
  case GL_COLOR_ARRAY_STRIDE:
    return out << "GL_COLOR_ARRAY_STRIDE";
  case GL_INDEX_ARRAY_TYPE:
    return out << "GL_INDEX_ARRAY_TYPE";
  case GL_INDEX_ARRAY_STRIDE:
    return out << "GL_INDEX_ARRAY_STRIDE";
  case GL_TEXTURE_COORD_ARRAY_SIZE:
    return out << "GL_TEXTURE_COORD_ARRAY_SIZE";
  case GL_TEXTURE_COORD_ARRAY_TYPE:
    return out << "GL_TEXTURE_COORD_ARRAY_TYPE";
  case GL_TEXTURE_COORD_ARRAY_STRIDE:
    return out << "GL_TEXTURE_COORD_ARRAY_STRIDE";
  case GL_EDGE_FLAG_ARRAY_STRIDE:
    return out << "GL_EDGE_FLAG_ARRAY_STRIDE";
  case GL_VERTEX_ARRAY_POINTER:
    return out << "GL_VERTEX_ARRAY_POINTER";
  case GL_NORMAL_ARRAY_POINTER:
    return out << "GL_NORMAL_ARRAY_POINTER";
  case GL_COLOR_ARRAY_POINTER:
    return out << "GL_COLOR_ARRAY_POINTER";
  case GL_INDEX_ARRAY_POINTER:
    return out << "GL_INDEX_ARRAY_POINTER";
  case GL_TEXTURE_COORD_ARRAY_POINTER:
    return out << "GL_TEXTURE_COORD_ARRAY_POINTER";
  case GL_EDGE_FLAG_ARRAY_POINTER:
    return out << "GL_EDGE_FLAG_ARRAY_POINTER";
  case GL_V2F:
    return out << "GL_V2F";
  case GL_V3F:
    return out << "GL_V3F";
  case GL_C4UB_V2F:
    return out << "GL_C4UB_V2F";
  case GL_C4UB_V3F:
    return out << "GL_C4UB_V3F";
  case GL_C3F_V3F:
    return out << "GL_C3F_V3F";
  case GL_N3F_V3F:
    return out << "GL_N3F_V3F";
  case GL_C4F_N3F_V3F:
    return out << "GL_C4F_N3F_V3F";
  case GL_T2F_V3F:
    return out << "GL_T2F_V3F";
  case GL_T4F_V4F:
    return out << "GL_T4F_V4F";
  case GL_T2F_C4UB_V3F:
    return out << "GL_T2F_C4UB_V3F";
  case GL_T2F_C3F_V3F:
    return out << "GL_T2F_C3F_V3F";
  case GL_T2F_N3F_V3F:
    return out << "GL_T2F_N3F_V3F";
  case GL_T2F_C4F_N3F_V3F:
    return out << "GL_T2F_C4F_N3F_V3F";
  case GL_T4F_C4F_N3F_V4F:
    return out << "GL_T4F_C4F_N3F_V4F";

    /* Matrix Mode */
  case GL_MATRIX_MODE:
    return out << "GL_MATRIX_MODE";
  case GL_MODELVIEW:
    return out << "GL_MODELVIEW";
  case GL_PROJECTION:
    return out << "GL_PROJECTION";
  case GL_TEXTURE:
    return out << "GL_TEXTURE";

    /* Points */
  case GL_POINT_SMOOTH:
    return out << "GL_POINT_SMOOTH";
  case GL_POINT_SIZE:
    return out << "GL_POINT_SIZE";
  case GL_POINT_SIZE_GRANULARITY:
    return out << "GL_POINT_SIZE_GRANULARITY";
  case GL_POINT_SIZE_RANGE:
    return out << "GL_POINT_SIZE_RANGE";

    /* Lines */
  case GL_LINE_SMOOTH:
    return out << "GL_LINE_SMOOTH";
  case GL_LINE_STIPPLE:
    return out << "GL_LINE_STIPPLE";
  case GL_LINE_STIPPLE_PATTERN:
    return out << "GL_LINE_STIPPLE_PATTERN";
  case GL_LINE_STIPPLE_REPEAT:
    return out << "GL_LINE_STIPPLE_REPEAT";
  case GL_LINE_WIDTH:
    return out << "GL_LINE_WIDTH";
  case GL_LINE_WIDTH_GRANULARITY:
    return out << "GL_LINE_WIDTH_GRANULARITY";
  case GL_LINE_WIDTH_RANGE:
    return out << "GL_LINE_WIDTH_RANGE";

    /* Polygons */
  case GL_POINT:
    return out << "GL_POINT";
  case GL_LINE:
    return out << "GL_LINE";
  case GL_FILL:
    return out << "GL_FILL";
  case GL_CCW:
    return out << "GL_CCW";
  case GL_CW:
    return out << "GL_CW";
  case GL_FRONT:
    return out << "GL_FRONT";
  case GL_BACK:
    return out << "GL_BACK";
  case GL_CULL_FACE:
    return out << "GL_CULL_FACE";
  case GL_CULL_FACE_MODE:
    return out << "GL_CULL_FACE_MODE";
  case GL_POLYGON_SMOOTH:
    return out << "GL_POLYGON_SMOOTH";
  case GL_POLYGON_STIPPLE:
    return out << "GL_POLYGON_STIPPLE";
  case GL_FRONT_FACE:
    return out << "GL_FRONT_FACE";
  case GL_POLYGON_MODE:
    return out << "GL_POLYGON_MODE";
  case GL_POLYGON_OFFSET_FACTOR:
    return out << "GL_POLYGON_OFFSET_FACTOR";
  case GL_POLYGON_OFFSET_UNITS:
    return out << "GL_POLYGON_OFFSET_UNITS";
  case GL_POLYGON_OFFSET_POINT:
    return out << "GL_POLYGON_OFFSET_POINT";
  case GL_POLYGON_OFFSET_LINE:
    return out << "GL_POLYGON_OFFSET_LINE";
  case GL_POLYGON_OFFSET_FILL:
    return out << "GL_POLYGON_OFFSET_FILL";

    /* Display Lists */
  case GL_COMPILE:
    return out << "GL_COMPILE";
  case GL_COMPILE_AND_EXECUTE:
    return out << "GL_COMPILE_AND_EXECUTE";
  case GL_LIST_BASE:
    return out << "GL_LIST_BASE";
  case GL_LIST_INDEX:
    return out << "GL_LIST_INDEX";
  case GL_LIST_MODE:
    return out << "GL_LIST_MODE";

    /* Depth buffer */
  case GL_NEVER:
    return out << "GL_NEVER";
  case GL_LESS:
    return out << "GL_LESS";
  case GL_GEQUAL:
    return out << "GL_GEQUAL";
  case GL_LEQUAL:
    return out << "GL_LEQUAL";
  case GL_GREATER:
    return out << "GL_GREATER";
  case GL_NOTEQUAL:
    return out << "GL_NOTEQUAL";
  case GL_EQUAL:
    return out << "GL_EQUAL";
  case GL_ALWAYS:
    return out << "GL_ALWAYS";
  case GL_DEPTH_TEST:
    return out << "GL_DEPTH_TEST";
  case GL_DEPTH_BITS:
    return out << "GL_DEPTH_BITS";
  case GL_DEPTH_CLEAR_VALUE:
    return out << "GL_DEPTH_CLEAR_VALUE";
  case GL_DEPTH_FUNC:
    return out << "GL_DEPTH_FUNC";
  case GL_DEPTH_RANGE:
    return out << "GL_DEPTH_RANGE";
  case GL_DEPTH_WRITEMASK:
    return out << "GL_DEPTH_WRITEMASK";
  case GL_DEPTH_COMPONENT:
    return out << "GL_DEPTH_COMPONENT";

    /* Lighting */
  case GL_LIGHTING:
    return out << "GL_LIGHTING";
  case GL_LIGHT0:
    return out << "GL_LIGHT0";
  case GL_LIGHT1:
    return out << "GL_LIGHT1";
  case GL_LIGHT2:
    return out << "GL_LIGHT2";
  case GL_LIGHT3:
    return out << "GL_LIGHT3";
  case GL_LIGHT4:
    return out << "GL_LIGHT4";
  case GL_LIGHT5:
    return out << "GL_LIGHT5";
  case GL_LIGHT6:
    return out << "GL_LIGHT6";
  case GL_LIGHT7:
    return out << "GL_LIGHT7";
  case GL_SPOT_EXPONENT:
    return out << "GL_SPOT_EXPONENT";
  case GL_SPOT_CUTOFF:
    return out << "GL_SPOT_CUTOFF";
  case GL_CONSTANT_ATTENUATION:
    return out << "GL_CONSTANT_ATTENUATION";
  case GL_LINEAR_ATTENUATION:
    return out << "GL_LINEAR_ATTENUATION";
  case GL_QUADRATIC_ATTENUATION:
    return out << "GL_QUADRATIC_ATTENUATION";
  case GL_AMBIENT:
    return out << "GL_AMBIENT";
  case GL_DIFFUSE:
    return out << "GL_DIFFUSE";
  case GL_SPECULAR:
    return out << "GL_SPECULAR";
  case GL_SHININESS:
    return out << "GL_SHININESS";
  case GL_EMISSION:
    return out << "GL_EMISSION";
  case GL_POSITION:
    return out << "GL_POSITION";
  case GL_SPOT_DIRECTION:
    return out << "GL_SPOT_DIRECTION";
  case GL_AMBIENT_AND_DIFFUSE:
    return out << "GL_AMBIENT_AND_DIFFUSE";
  case GL_COLOR_INDEXES:
    return out << "GL_COLOR_INDEXES";
  case GL_LIGHT_MODEL_TWO_SIDE:
    return out << "GL_LIGHT_MODEL_TWO_SIDE";
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    return out << "GL_LIGHT_MODEL_LOCAL_VIEWER";
  case GL_LIGHT_MODEL_AMBIENT:
    return out << "GL_LIGHT_MODEL_AMBIENT";
  case GL_FRONT_AND_BACK:
    return out << "GL_FRONT_AND_BACK";
  case GL_SHADE_MODEL:
    return out << "GL_SHADE_MODEL";
  case GL_FLAT:
    return out << "GL_FLAT";
  case GL_SMOOTH:
    return out << "GL_SMOOTH";
  case GL_COLOR_MATERIAL:
    return out << "GL_COLOR_MATERIAL";
  case GL_COLOR_MATERIAL_FACE:
    return out << "GL_COLOR_MATERIAL_FACE";
  case GL_COLOR_MATERIAL_PARAMETER:
    return out << "GL_COLOR_MATERIAL_PARAMETER";
  case GL_NORMALIZE:
    return out << "GL_NORMALIZE";

    /* User clipping planes */
  case GL_CLIP_PLANE0:
    return out << "GL_CLIP_PLANE0";
  case GL_CLIP_PLANE1:
    return out << "GL_CLIP_PLANE1";
  case GL_CLIP_PLANE2:
    return out << "GL_CLIP_PLANE2";
  case GL_CLIP_PLANE3:
    return out << "GL_CLIP_PLANE3";
  case GL_CLIP_PLANE4:
    return out << "GL_CLIP_PLANE4";
  case GL_CLIP_PLANE5:
    return out << "GL_CLIP_PLANE5";

    /* Accumulation buffer */
  case GL_ACCUM_RED_BITS:
    return out << "GL_ACCUM_RED_BITS";
  case GL_ACCUM_GREEN_BITS:
    return out << "GL_ACCUM_GREEN_BITS";
  case GL_ACCUM_BLUE_BITS:
    return out << "GL_ACCUM_BLUE_BITS";
  case GL_ACCUM_ALPHA_BITS:
    return out << "GL_ACCUM_ALPHA_BITS";
  case GL_ACCUM_CLEAR_VALUE:
    return out << "GL_ACCUM_CLEAR_VALUE";
  case GL_ACCUM:
    return out << "GL_ACCUM";
  case GL_ADD:
    return out << "GL_ADD";
  case GL_LOAD:
    return out << "GL_LOAD";
  case GL_MULT:
    return out << "GL_MULT";

    /* Alpha testing */
  case GL_ALPHA_TEST:
    return out << "GL_ALPHA_TEST";
  case GL_ALPHA_TEST_REF:
    return out << "GL_ALPHA_TEST_REF";
  case GL_ALPHA_TEST_FUNC:
    return out << "GL_ALPHA_TEST_FUNC";

    /* Blending */
  case GL_BLEND:
    return out << "GL_BLEND";
  case GL_BLEND_SRC:
    return out << "GL_BLEND_SRC";
  case GL_BLEND_DST:
    return out << "GL_BLEND_DST";
    /*
      case GL_ZERO:
      return out << "GL_ZERO";
      case GL_ONE:
      return out << "GL_ONE";
    */
  case GL_SRC_COLOR:
    return out << "GL_SRC_COLOR";
  case GL_ONE_MINUS_SRC_COLOR:
    return out << "GL_ONE_MINUS_SRC_COLOR";
  case GL_DST_COLOR:
    return out << "GL_DST_COLOR";
  case GL_ONE_MINUS_DST_COLOR:
    return out << "GL_ONE_MINUS_DST_COLOR";
  case GL_SRC_ALPHA:
    return out << "GL_SRC_ALPHA";
  case GL_ONE_MINUS_SRC_ALPHA:
    return out << "GL_ONE_MINUS_SRC_ALPHA";
  case GL_DST_ALPHA:
    return out << "GL_DST_ALPHA";
  case GL_ONE_MINUS_DST_ALPHA:
    return out << "GL_ONE_MINUS_DST_ALPHA";
  case GL_SRC_ALPHA_SATURATE:
    return out << "GL_SRC_ALPHA_SATURATE";
  case GL_CONSTANT_COLOR:
    return out << "GL_CONSTANT_COLOR";
  case GL_ONE_MINUS_CONSTANT_COLOR:
    return out << "GL_ONE_MINUS_CONSTANT_COLOR";
  case GL_CONSTANT_ALPHA:
    return out << "GL_CONSTANT_ALPHA";
  case GL_ONE_MINUS_CONSTANT_ALPHA:
    return out << "GL_ONE_MINUS_CONSTANT_ALPHA";

    /* Render Mode */
  case GL_FEEDBACK:
    return out << "GL_FEEDBACK";
  case GL_RENDER:
    return out << "GL_RENDER";
  case GL_SELECT:
    return out << "GL_SELECT";

    /* Feedback */
  case GL_2D:
    return out << "GL_2D";
  case GL_3D:
    return out << "GL_3D";
  case GL_3D_COLOR:
    return out << "GL_3D_COLOR";
  case GL_3D_COLOR_TEXTURE:
    return out << "GL_3D_COLOR_TEXTURE";
  case GL_4D_COLOR_TEXTURE:
    return out << "GL_4D_COLOR_TEXTURE";
  case GL_POINT_TOKEN:
    return out << "GL_POINT_TOKEN";
  case GL_LINE_TOKEN:
    return out << "GL_LINE_TOKEN";
  case GL_LINE_RESET_TOKEN:
    return out << "GL_LINE_RESET_TOKEN";
  case GL_POLYGON_TOKEN:
    return out << "GL_POLYGON_TOKEN";
  case GL_BITMAP_TOKEN:
    return out << "GL_BITMAP_TOKEN";
  case GL_DRAW_PIXEL_TOKEN:
    return out << "GL_DRAW_PIXEL_TOKEN";
  case GL_COPY_PIXEL_TOKEN:
    return out << "GL_COPY_PIXEL_TOKEN";
  case GL_PASS_THROUGH_TOKEN:
    return out << "GL_PASS_THROUGH_TOKEN";
  case GL_FEEDBACK_BUFFER_POINTER:
    return out << "GL_FEEDBACK_BUFFER_POINTER";
  case GL_FEEDBACK_BUFFER_SIZE:
    return out << "GL_FEEDBACK_BUFFER_SIZE";
  case GL_FEEDBACK_BUFFER_TYPE:
    return out << "GL_FEEDBACK_BUFFER_TYPE";

    /* Selection */
  case GL_SELECTION_BUFFER_POINTER:
    return out << "GL_SELECTION_BUFFER_POINTER";
  case GL_SELECTION_BUFFER_SIZE:
    return out << "GL_SELECTION_BUFFER_SIZE";

    /* Fog */
  case GL_FOG:
    return out << "GL_FOG";
  case GL_FOG_MODE:
    return out << "GL_FOG_MODE";
  case GL_FOG_DENSITY:
    return out << "GL_FOG_DENSITY";
  case GL_FOG_COLOR:
    return out << "GL_FOG_COLOR";
  case GL_FOG_INDEX:
    return out << "GL_FOG_INDEX";
  case GL_FOG_START:
    return out << "GL_FOG_START";
  case GL_FOG_END:
    return out << "GL_FOG_END";
  case GL_LINEAR:
    return out << "GL_LINEAR";
  case GL_EXP:
    return out << "GL_EXP";
  case GL_EXP2:
    return out << "GL_EXP2";

    /* Logic Ops */
  case GL_LOGIC_OP:
    return out << "GL_LOGIC_OP";
    /*
      case GL_INDEX_LOGIC_OP:
      return out << "GL_INDEX_LOGIC_OP";
    */
  case GL_COLOR_LOGIC_OP:
    return out << "GL_COLOR_LOGIC_OP";
  case GL_LOGIC_OP_MODE:
    return out << "GL_LOGIC_OP_MODE";
  case GL_CLEAR:
    return out << "GL_CLEAR";
  case GL_SET:
    return out << "GL_SET";
  case GL_COPY:
    return out << "GL_COPY";
  case GL_COPY_INVERTED:
    return out << "GL_COPY_INVERTED";
  case GL_NOOP:
    return out << "GL_NOOP";
  case GL_INVERT:
    return out << "GL_INVERT";
  case GL_AND:
    return out << "GL_AND";
  case GL_NAND:
    return out << "GL_NAND";
  case GL_OR:
    return out << "GL_OR";
  case GL_NOR:
    return out << "GL_NOR";
  case GL_XOR:
    return out << "GL_XOR";
  case GL_EQUIV:
    return out << "GL_EQUIV";
  case GL_AND_REVERSE:
    return out << "GL_AND_REVERSE";
  case GL_AND_INVERTED:
    return out << "GL_AND_INVERTED";
  case GL_OR_REVERSE:
    return out << "GL_OR_REVERSE";
  case GL_OR_INVERTED:
    return out << "GL_OR_INVERTED";

    /* Stencil */
  case GL_STENCIL_TEST:
    return out << "GL_STENCIL_TEST";
  case GL_STENCIL_WRITEMASK:
    return out << "GL_STENCIL_WRITEMASK";
  case GL_STENCIL_BITS:
    return out << "GL_STENCIL_BITS";
  case GL_STENCIL_FUNC:
    return out << "GL_STENCIL_FUNC";
  case GL_STENCIL_VALUE_MASK:
    return out << "GL_STENCIL_VALUE_MASK";
  case GL_STENCIL_REF:
    return out << "GL_STENCIL_REF";
  case GL_STENCIL_FAIL:
    return out << "GL_STENCIL_FAIL";
  case GL_STENCIL_PASS_DEPTH_PASS:
    return out << "GL_STENCIL_PASS_DEPTH_PASS";
  case GL_STENCIL_PASS_DEPTH_FAIL:
    return out << "GL_STENCIL_PASS_DEPTH_FAIL";
  case GL_STENCIL_CLEAR_VALUE:
    return out << "GL_STENCIL_CLEAR_VALUE";
  case GL_STENCIL_INDEX:
    return out << "GL_STENCIL_INDEX";
  case GL_KEEP:
    return out << "GL_KEEP";
  case GL_REPLACE:
    return out << "GL_REPLACE";
  case GL_INCR:
    return out << "GL_INCR";
  case GL_DECR:
    return out << "GL_DECR";

    /* Buffers, Pixel Drawing/Reading */
    /*
      case GL_NONE:
      return out << "GL_NONE";
    */
  case GL_LEFT:
    return out << "GL_LEFT";
  case GL_RIGHT:
    return out << "GL_RIGHT";
  case GL_FRONT_LEFT:
    return out << "GL_FRONT_LEFT";
  case GL_FRONT_RIGHT:
    return out << "GL_FRONT_RIGHT";
  case GL_BACK_LEFT:
    return out << "GL_BACK_LEFT";
  case GL_BACK_RIGHT:
    return out << "GL_BACK_RIGHT";
  case GL_AUX0:
    return out << "GL_AUX0";
  case GL_AUX1:
    return out << "GL_AUX1";
  case GL_AUX2:
    return out << "GL_AUX2";
  case GL_AUX3:
    return out << "GL_AUX3";
  case GL_COLOR_INDEX:
    return out << "GL_COLOR_INDEX";
  case GL_RED:
    return out << "GL_RED";
  case GL_GREEN:
    return out << "GL_GREEN";
  case GL_BLUE:
    return out << "GL_BLUE";
  case GL_ALPHA:
    return out << "GL_ALPHA";
  case GL_LUMINANCE:
    return out << "GL_LUMINANCE";
  case GL_LUMINANCE_ALPHA:
    return out << "GL_LUMINANCE_ALPHA";
  case GL_ALPHA_BITS:
    return out << "GL_ALPHA_BITS";
  case GL_RED_BITS:
    return out << "GL_RED_BITS";
  case GL_GREEN_BITS:
    return out << "GL_GREEN_BITS";
  case GL_BLUE_BITS:
    return out << "GL_BLUE_BITS";
  case GL_INDEX_BITS:
    return out << "GL_INDEX_BITS";
  case GL_SUBPIXEL_BITS:
    return out << "GL_SUBPIXEL_BITS";
  case GL_AUX_BUFFERS:
    return out << "GL_AUX_BUFFERS";
  case GL_READ_BUFFER:
    return out << "GL_READ_BUFFER";
  case GL_DRAW_BUFFER:
    return out << "GL_DRAW_BUFFER";
  case GL_DOUBLEBUFFER:
    return out << "GL_DOUBLEBUFFER";
  case GL_STEREO:
    return out << "GL_STEREO";
  case GL_BITMAP:
    return out << "GL_BITMAP";
  case GL_COLOR:
    return out << "GL_COLOR";
  case GL_DEPTH:
    return out << "GL_DEPTH";
  case GL_STENCIL:
    return out << "GL_STENCIL";
  case GL_DITHER:
    return out << "GL_DITHER";
  case GL_RGB:
    return out << "GL_RGB";
  case GL_RGBA:
    return out << "GL_RGBA";

    /* Implementation limits */
  case GL_MAX_LIST_NESTING:
    return out << "GL_MAX_LIST_NESTING";
  case GL_MAX_ATTRIB_STACK_DEPTH:
    return out << "GL_MAX_ATTRIB_STACK_DEPTH";
  case GL_MAX_MODELVIEW_STACK_DEPTH:
    return out << "GL_MAX_MODELVIEW_STACK_DEPTH";
  case GL_MAX_NAME_STACK_DEPTH:
    return out << "GL_MAX_NAME_STACK_DEPTH";
  case GL_MAX_PROJECTION_STACK_DEPTH:
    return out << "GL_MAX_PROJECTION_STACK_DEPTH";
  case GL_MAX_TEXTURE_STACK_DEPTH:
    return out << "GL_MAX_TEXTURE_STACK_DEPTH";
  case GL_MAX_EVAL_ORDER:
    return out << "GL_MAX_EVAL_ORDER";
  case GL_MAX_LIGHTS:
    return out << "GL_MAX_LIGHTS";
  case GL_MAX_CLIP_PLANES:
    return out << "GL_MAX_CLIP_PLANES";
  case GL_MAX_TEXTURE_SIZE:
    return out << "GL_MAX_TEXTURE_SIZE";
  case GL_MAX_PIXEL_MAP_TABLE:
    return out << "GL_MAX_PIXEL_MAP_TABLE";
  case GL_MAX_VIEWPORT_DIMS:
    return out << "GL_MAX_VIEWPORT_DIMS";
  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
    return out << "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH";

    /* Gets */
  case GL_ATTRIB_STACK_DEPTH:
    return out << "GL_ATTRIB_STACK_DEPTH";
  case GL_CLIENT_ATTRIB_STACK_DEPTH:
    return out << "GL_CLIENT_ATTRIB_STACK_DEPTH";
  case GL_COLOR_CLEAR_VALUE:
    return out << "GL_COLOR_CLEAR_VALUE";
  case GL_COLOR_WRITEMASK:
    return out << "GL_COLOR_WRITEMASK";
  case GL_CURRENT_INDEX:
    return out << "GL_CURRENT_INDEX";
  case GL_CURRENT_COLOR:
    return out << "GL_CURRENT_COLOR";
  case GL_CURRENT_NORMAL:
    return out << "GL_CURRENT_NORMAL";
  case GL_CURRENT_RASTER_COLOR:
    return out << "GL_CURRENT_RASTER_COLOR";
  case GL_CURRENT_RASTER_DISTANCE:
    return out << "GL_CURRENT_RASTER_DISTANCE";
  case GL_CURRENT_RASTER_INDEX:
    return out << "GL_CURRENT_RASTER_INDEX";
  case GL_CURRENT_RASTER_POSITION:
    return out << "GL_CURRENT_RASTER_POSITION";
  case GL_CURRENT_RASTER_TEXTURE_COORDS:
    return out << "GL_CURRENT_RASTER_TEXTURE_COORDS";
  case GL_CURRENT_RASTER_POSITION_VALID:
    return out << "GL_CURRENT_RASTER_POSITION_VALID";
  case GL_CURRENT_TEXTURE_COORDS:
    return out << "GL_CURRENT_TEXTURE_COORDS";
  case GL_INDEX_CLEAR_VALUE:
    return out << "GL_INDEX_CLEAR_VALUE";
  case GL_INDEX_MODE:
    return out << "GL_INDEX_MODE";
  case GL_INDEX_WRITEMASK:
    return out << "GL_INDEX_WRITEMASK";
  case GL_MODELVIEW_MATRIX:
    return out << "GL_MODELVIEW_MATRIX";
  case GL_MODELVIEW_STACK_DEPTH:
    return out << "GL_MODELVIEW_STACK_DEPTH";
  case GL_NAME_STACK_DEPTH:
    return out << "GL_NAME_STACK_DEPTH";
  case GL_PROJECTION_MATRIX:
    return out << "GL_PROJECTION_MATRIX";
  case GL_PROJECTION_STACK_DEPTH:
    return out << "GL_PROJECTION_STACK_DEPTH";
  case GL_RENDER_MODE:
    return out << "GL_RENDER_MODE";
  case GL_RGBA_MODE:
    return out << "GL_RGBA_MODE";
  case GL_TEXTURE_MATRIX:
    return out << "GL_TEXTURE_MATRIX";
  case GL_TEXTURE_STACK_DEPTH:
    return out << "GL_TEXTURE_STACK_DEPTH";
  case GL_VIEWPORT:
    return out << "GL_VIEWPORT";


    /* Evaluators */
  case GL_AUTO_NORMAL:
    return out << "GL_AUTO_NORMAL";
  case GL_MAP1_COLOR_4:
    return out << "GL_MAP1_COLOR_4";
  case GL_MAP1_GRID_DOMAIN:
    return out << "GL_MAP1_GRID_DOMAIN";
  case GL_MAP1_GRID_SEGMENTS:
    return out << "GL_MAP1_GRID_SEGMENTS";
  case GL_MAP1_INDEX:
    return out << "GL_MAP1_INDEX";
  case GL_MAP1_NORMAL:
    return out << "GL_MAP1_NORMAL";
  case GL_MAP1_TEXTURE_COORD_1:
    return out << "GL_MAP1_TEXTURE_COORD_1";
  case GL_MAP1_TEXTURE_COORD_2:
    return out << "GL_MAP1_TEXTURE_COORD_2";
  case GL_MAP1_TEXTURE_COORD_3:
    return out << "GL_MAP1_TEXTURE_COORD_3";
  case GL_MAP1_TEXTURE_COORD_4:
    return out << "GL_MAP1_TEXTURE_COORD_4";
  case GL_MAP1_VERTEX_3:
    return out << "GL_MAP1_VERTEX_3";
  case GL_MAP1_VERTEX_4:
    return out << "GL_MAP1_VERTEX_4";
  case GL_MAP2_COLOR_4:
    return out << "GL_MAP2_COLOR_4";
  case GL_MAP2_GRID_DOMAIN:
    return out << "GL_MAP2_GRID_DOMAIN";
  case GL_MAP2_GRID_SEGMENTS:
    return out << "GL_MAP2_GRID_SEGMENTS";
  case GL_MAP2_INDEX:
    return out << "GL_MAP2_INDEX";
  case GL_MAP2_NORMAL:
    return out << "GL_MAP2_NORMAL";
  case GL_MAP2_TEXTURE_COORD_1:
    return out << "GL_MAP2_TEXTURE_COORD_1";
  case GL_MAP2_TEXTURE_COORD_2:
    return out << "GL_MAP2_TEXTURE_COORD_2";
  case GL_MAP2_TEXTURE_COORD_3:
    return out << "GL_MAP2_TEXTURE_COORD_3";
  case GL_MAP2_TEXTURE_COORD_4:
    return out << "GL_MAP2_TEXTURE_COORD_4";
  case GL_MAP2_VERTEX_3:
    return out << "GL_MAP2_VERTEX_3";
  case GL_MAP2_VERTEX_4:
    return out << "GL_MAP2_VERTEX_4";
  case GL_COEFF:
    return out << "GL_COEFF";
  case GL_DOMAIN:
    return out << "GL_DOMAIN";
  case GL_ORDER:
    return out << "GL_ORDER";

    /* Hints */
  case GL_FOG_HINT:
    return out << "GL_FOG_HINT";
  case GL_LINE_SMOOTH_HINT:
    return out << "GL_LINE_SMOOTH_HINT";
  case GL_PERSPECTIVE_CORRECTION_HINT:
    return out << "GL_PERSPECTIVE_CORRECTION_HINT";
  case GL_POINT_SMOOTH_HINT:
    return out << "GL_POINT_SMOOTH_HINT";
  case GL_POLYGON_SMOOTH_HINT:
    return out << "GL_POLYGON_SMOOTH_HINT";
  case GL_DONT_CARE:
    return out << "GL_DONT_CARE";
  case GL_FASTEST:
    return out << "GL_FASTEST";
  case GL_NICEST:
    return out << "GL_NICEST";

    /* Scissor box */
  case GL_SCISSOR_TEST:
    return out << "GL_SCISSOR_TEST";
  case GL_SCISSOR_BOX:
    return out << "GL_SCISSOR_BOX";

    /* Pixel Mode / Transfer */
  case GL_MAP_COLOR:
    return out << "GL_MAP_COLOR";
  case GL_MAP_STENCIL:
    return out << "GL_MAP_STENCIL";
  case GL_INDEX_SHIFT:
    return out << "GL_INDEX_SHIFT";
  case GL_INDEX_OFFSET:
    return out << "GL_INDEX_OFFSET";
  case GL_RED_SCALE:
    return out << "GL_RED_SCALE";
  case GL_RED_BIAS:
    return out << "GL_RED_BIAS";
  case GL_GREEN_SCALE:
    return out << "GL_GREEN_SCALE";
  case GL_GREEN_BIAS:
    return out << "GL_GREEN_BIAS";
  case GL_BLUE_SCALE:
    return out << "GL_BLUE_SCALE";
  case GL_BLUE_BIAS:
    return out << "GL_BLUE_BIAS";
  case GL_ALPHA_SCALE:
    return out << "GL_ALPHA_SCALE";
  case GL_ALPHA_BIAS:
    return out << "GL_ALPHA_BIAS";
  case GL_DEPTH_SCALE:
    return out << "GL_DEPTH_SCALE";
  case GL_DEPTH_BIAS:
    return out << "GL_DEPTH_BIAS";
  case GL_PIXEL_MAP_S_TO_S_SIZE:
    return out << "GL_PIXEL_MAP_S_TO_S_SIZE";
  case GL_PIXEL_MAP_I_TO_I_SIZE:
    return out << "GL_PIXEL_MAP_I_TO_I_SIZE";
  case GL_PIXEL_MAP_I_TO_R_SIZE:
    return out << "GL_PIXEL_MAP_I_TO_R_SIZE";
  case GL_PIXEL_MAP_I_TO_G_SIZE:
    return out << "GL_PIXEL_MAP_I_TO_G_SIZE";
  case GL_PIXEL_MAP_I_TO_B_SIZE:
    return out << "GL_PIXEL_MAP_I_TO_B_SIZE";
  case GL_PIXEL_MAP_I_TO_A_SIZE:
    return out << "GL_PIXEL_MAP_I_TO_A_SIZE";
  case GL_PIXEL_MAP_R_TO_R_SIZE:
    return out << "GL_PIXEL_MAP_R_TO_R_SIZE";
  case GL_PIXEL_MAP_G_TO_G_SIZE:
    return out << "GL_PIXEL_MAP_G_TO_G_SIZE";
  case GL_PIXEL_MAP_B_TO_B_SIZE:
    return out << "GL_PIXEL_MAP_B_TO_B_SIZE";
  case GL_PIXEL_MAP_A_TO_A_SIZE:
    return out << "GL_PIXEL_MAP_A_TO_A_SIZE";
  case GL_PIXEL_MAP_S_TO_S:
    return out << "GL_PIXEL_MAP_S_TO_S";
  case GL_PIXEL_MAP_I_TO_I:
    return out << "GL_PIXEL_MAP_I_TO_I";
  case GL_PIXEL_MAP_I_TO_R:
    return out << "GL_PIXEL_MAP_I_TO_R";
  case GL_PIXEL_MAP_I_TO_G:
    return out << "GL_PIXEL_MAP_I_TO_G";
  case GL_PIXEL_MAP_I_TO_B:
    return out << "GL_PIXEL_MAP_I_TO_B";
  case GL_PIXEL_MAP_I_TO_A:
    return out << "GL_PIXEL_MAP_I_TO_A";
  case GL_PIXEL_MAP_R_TO_R:
    return out << "GL_PIXEL_MAP_R_TO_R";
  case GL_PIXEL_MAP_G_TO_G:
    return out << "GL_PIXEL_MAP_G_TO_G";
  case GL_PIXEL_MAP_B_TO_B:
    return out << "GL_PIXEL_MAP_B_TO_B";
  case GL_PIXEL_MAP_A_TO_A:
    return out << "GL_PIXEL_MAP_A_TO_A";
  case GL_PACK_ALIGNMENT:
    return out << "GL_PACK_ALIGNMENT";
  case GL_PACK_LSB_FIRST:
    return out << "GL_PACK_LSB_FIRST";
  case GL_PACK_ROW_LENGTH:
    return out << "GL_PACK_ROW_LENGTH";
  case GL_PACK_SKIP_PIXELS:
    return out << "GL_PACK_SKIP_PIXELS";
  case GL_PACK_SKIP_ROWS:
    return out << "GL_PACK_SKIP_ROWS";
  case GL_PACK_SWAP_BYTES:
    return out << "GL_PACK_SWAP_BYTES";
  case GL_UNPACK_ALIGNMENT:
    return out << "GL_UNPACK_ALIGNMENT";
  case GL_UNPACK_LSB_FIRST:
    return out << "GL_UNPACK_LSB_FIRST";
  case GL_UNPACK_ROW_LENGTH:
    return out << "GL_UNPACK_ROW_LENGTH";
  case GL_UNPACK_SKIP_PIXELS:
    return out << "GL_UNPACK_SKIP_PIXELS";
  case GL_UNPACK_SKIP_ROWS:
    return out << "GL_UNPACK_SKIP_ROWS";
  case GL_UNPACK_SWAP_BYTES:
    return out << "GL_UNPACK_SWAP_BYTES";
  case GL_ZOOM_X:
    return out << "GL_ZOOM_X";
  case GL_ZOOM_Y:
    return out << "GL_ZOOM_Y";

    /* Texture mapping */
  case GL_TEXTURE_ENV:
    return out << "GL_TEXTURE_ENV";
  case GL_TEXTURE_ENV_MODE:
    return out << "GL_TEXTURE_ENV_MODE";
  case GL_TEXTURE_1D:
    return out << "GL_TEXTURE_1D";
  case GL_TEXTURE_2D:
    return out << "GL_TEXTURE_2D";
  case GL_TEXTURE_WRAP_S:
    return out << "GL_TEXTURE_WRAP_S";
  case GL_TEXTURE_WRAP_T:
    return out << "GL_TEXTURE_WRAP_T";
  case GL_TEXTURE_MAG_FILTER:
    return out << "GL_TEXTURE_MAG_FILTER";
  case GL_TEXTURE_MIN_FILTER:
    return out << "GL_TEXTURE_MIN_FILTER";
  case GL_TEXTURE_ENV_COLOR:
    return out << "GL_TEXTURE_ENV_COLOR";
  case GL_TEXTURE_GEN_S:
    return out << "GL_TEXTURE_GEN_S";
  case GL_TEXTURE_GEN_T:
    return out << "GL_TEXTURE_GEN_T";
  case GL_TEXTURE_GEN_MODE:
    return out << "GL_TEXTURE_GEN_MODE";
  case GL_TEXTURE_BORDER_COLOR:
    return out << "GL_TEXTURE_BORDER_COLOR";
  case GL_TEXTURE_WIDTH:
    return out << "GL_TEXTURE_WIDTH";
  case GL_TEXTURE_HEIGHT:
    return out << "GL_TEXTURE_HEIGHT";
  case GL_TEXTURE_BORDER:
    return out << "GL_TEXTURE_BORDER";
  case GL_TEXTURE_COMPONENTS:
    return out << "GL_TEXTURE_COMPONENTS";
  case GL_TEXTURE_RED_SIZE:
    return out << "GL_TEXTURE_RED_SIZE";
  case GL_TEXTURE_GREEN_SIZE:
    return out << "GL_TEXTURE_GREEN_SIZE";
  case GL_TEXTURE_BLUE_SIZE:
    return out << "GL_TEXTURE_BLUE_SIZE";
  case GL_TEXTURE_ALPHA_SIZE:
    return out << "GL_TEXTURE_ALPHA_SIZE";
  case GL_TEXTURE_LUMINANCE_SIZE:
    return out << "GL_TEXTURE_LUMINANCE_SIZE";
  case GL_TEXTURE_INTENSITY_SIZE:
    return out << "GL_TEXTURE_INTENSITY_SIZE";
  case GL_NEAREST_MIPMAP_NEAREST:
    return out << "GL_NEAREST_MIPMAP_NEAREST";
  case GL_NEAREST_MIPMAP_LINEAR:
    return out << "GL_NEAREST_MIPMAP_LINEAR";
  case GL_LINEAR_MIPMAP_NEAREST:
    return out << "GL_LINEAR_MIPMAP_NEAREST";
  case GL_LINEAR_MIPMAP_LINEAR:
    return out << "GL_LINEAR_MIPMAP_LINEAR";
  case GL_OBJECT_LINEAR:
    return out << "GL_OBJECT_LINEAR";
  case GL_OBJECT_PLANE:
    return out << "GL_OBJECT_PLANE";
  case GL_EYE_LINEAR:
    return out << "GL_EYE_LINEAR";
  case GL_EYE_PLANE:
    return out << "GL_EYE_PLANE";
  case GL_SPHERE_MAP:
    return out << "GL_SPHERE_MAP";
  case GL_DECAL:
    return out << "GL_DECAL";
  case GL_MODULATE:
    return out << "GL_MODULATE";
  case GL_NEAREST:
    return out << "GL_NEAREST";
  case GL_REPEAT:
    return out << "GL_REPEAT";
  case GL_CLAMP:
    return out << "GL_CLAMP";
  case GL_S:
    return out << "GL_S";
  case GL_T:
    return out << "GL_T";
  case GL_R:
    return out << "GL_R";
  case GL_Q:
    return out << "GL_Q";
  case GL_TEXTURE_GEN_R:
    return out << "GL_TEXTURE_GEN_R";
  case GL_TEXTURE_GEN_Q:
    return out << "GL_TEXTURE_GEN_Q";

    /* GL 1.1 texturing */
  case GL_PROXY_TEXTURE_1D:
    return out << "GL_PROXY_TEXTURE_1D";
  case GL_PROXY_TEXTURE_2D:
    return out << "GL_PROXY_TEXTURE_2D";
  case GL_TEXTURE_PRIORITY:
    return out << "GL_TEXTURE_PRIORITY";
  case GL_TEXTURE_RESIDENT:
    return out << "GL_TEXTURE_RESIDENT";
  case GL_TEXTURE_BINDING_1D:
    return out << "GL_TEXTURE_BINDING_1D";
  case GL_TEXTURE_BINDING_2D:
    return out << "GL_TEXTURE_BINDING_2D";
    /*
      case GL_TEXTURE_INTERNAL_FORMAT:
      return out << "GL_TEXTURE_INTERNAL_FORMAT";
    */

    /* GL 1.2 texturing */
  case GL_PACK_SKIP_IMAGES:
    return out << "GL_PACK_SKIP_IMAGES";
  case GL_PACK_IMAGE_HEIGHT:
    return out << "GL_PACK_IMAGE_HEIGHT";
  case GL_UNPACK_SKIP_IMAGES:
    return out << "GL_UNPACK_SKIP_IMAGES";
  case GL_UNPACK_IMAGE_HEIGHT:
    return out << "GL_UNPACK_IMAGE_HEIGHT";
  case GL_TEXTURE_3D:
    return out << "GL_TEXTURE_3D";
  case GL_PROXY_TEXTURE_3D:
    return out << "GL_PROXY_TEXTURE_3D";
  case GL_TEXTURE_DEPTH:
    return out << "GL_TEXTURE_DEPTH";
  case GL_TEXTURE_WRAP_R:
    return out << "GL_TEXTURE_WRAP_R";
  case GL_MAX_3D_TEXTURE_SIZE:
    return out << "GL_MAX_3D_TEXTURE_SIZE";
  case GL_TEXTURE_BINDING_3D:
    return out << "GL_TEXTURE_BINDING_3D";

    /* Internal texture formats (GL 1.1) */
  case GL_ALPHA4:
    return out << "GL_ALPHA4";
  case GL_ALPHA8:
    return out << "GL_ALPHA8";
  case GL_ALPHA12:
    return out << "GL_ALPHA12";
  case GL_ALPHA16:
    return out << "GL_ALPHA16";
  case GL_LUMINANCE4:
    return out << "GL_LUMINANCE4";
  case GL_LUMINANCE8:
    return out << "GL_LUMINANCE8";
  case GL_LUMINANCE12:
    return out << "GL_LUMINANCE12";
  case GL_LUMINANCE16:
    return out << "GL_LUMINANCE16";
  case GL_LUMINANCE4_ALPHA4:
    return out << "GL_LUMINANCE4_ALPHA4";
  case GL_LUMINANCE6_ALPHA2:
    return out << "GL_LUMINANCE6_ALPHA2";
  case GL_LUMINANCE8_ALPHA8:
    return out << "GL_LUMINANCE8_ALPHA8";
  case GL_LUMINANCE12_ALPHA4:
    return out << "GL_LUMINANCE12_ALPHA4";
  case GL_LUMINANCE12_ALPHA12:
    return out << "GL_LUMINANCE12_ALPHA12";
  case GL_LUMINANCE16_ALPHA16:
    return out << "GL_LUMINANCE16_ALPHA16";
  case GL_INTENSITY:
    return out << "GL_INTENSITY";
  case GL_INTENSITY4:
    return out << "GL_INTENSITY4";
  case GL_INTENSITY8:
    return out << "GL_INTENSITY8";
  case GL_INTENSITY12:
    return out << "GL_INTENSITY12";
  case GL_INTENSITY16:
    return out << "GL_INTENSITY16";
  case GL_R3_G3_B2:
    return out << "GL_R3_G3_B2";
  case GL_RGB4:
    return out << "GL_RGB4";
  case GL_RGB5:
    return out << "GL_RGB5";
  case GL_RGB8:
    return out << "GL_RGB8";
  case GL_RGB10:
    return out << "GL_RGB10";
  case GL_RGB12:
    return out << "GL_RGB12";
  case GL_RGB16:
    return out << "GL_RGB16";
  case GL_RGBA2:
    return out << "GL_RGBA2";
  case GL_RGBA4:
    return out << "GL_RGBA4";
  case GL_RGB5_A1:
    return out << "GL_RGB5_A1";
  case GL_RGBA8:
    return out << "GL_RGBA8";
  case GL_RGB10_A2:
    return out << "GL_RGB10_A2";
  case GL_RGBA12:
    return out << "GL_RGBA12";
  case GL_RGBA16:
    return out << "GL_RGBA16";

    /* Utility */
  case GL_VENDOR:
    return out << "GL_VENDOR";
  case GL_RENDERER:
    return out << "GL_RENDERER";
  case GL_VERSION:
    return out << "GL_VERSION";
  case GL_EXTENSIONS:
    return out << "GL_EXTENSIONS";

    /* Errors */
  case GL_INVALID_VALUE:
    return out << "GL_INVALID_VALUE";
  case GL_INVALID_ENUM:
    return out << "GL_INVALID_ENUM";
  case GL_INVALID_OPERATION:
    return out << "GL_INVALID_OPERATION";
  case GL_STACK_OVERFLOW:
    return out << "GL_STACK_OVERFLOW";
  case GL_STACK_UNDERFLOW:
    return out << "GL_STACK_UNDERFLOW";
  case GL_OUT_OF_MEMORY:
    return out << "GL_OUT_OF_MEMORY";

    /* OpenGL 1.2 */
  case GL_RESCALE_NORMAL:
    return out << "GL_RESCALE_NORMAL";
  case GL_CLAMP_TO_EDGE:
    return out << "GL_CLAMP_TO_EDGE";
  case GL_MAX_ELEMENTS_VERTICES:
    return out << "GL_MAX_ELEMENTS_VERTICES";
  case GL_MAX_ELEMENTS_INDICES:
    return out << "GL_MAX_ELEMENTS_INDICES";
  case GL_BGR:
    return out << "GL_BGR";
  case GL_BGRA:
    return out << "GL_BGRA";
  case GL_UNSIGNED_BYTE_3_3_2:
    return out << "GL_UNSIGNED_BYTE_3_3_2";
  case GL_UNSIGNED_BYTE_2_3_3_REV:
    return out << "GL_UNSIGNED_BYTE_2_3_3_REV";
  case GL_UNSIGNED_SHORT_5_6_5:
    return out << "GL_UNSIGNED_SHORT_5_6_5";
  case GL_UNSIGNED_SHORT_5_6_5_REV:
    return out << "GL_UNSIGNED_SHORT_5_6_5_REV";
  case GL_UNSIGNED_SHORT_4_4_4_4:
    return out << "GL_UNSIGNED_SHORT_4_4_4_4";
  case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    return out << "GL_UNSIGNED_SHORT_4_4_4_4_REV";
  case GL_UNSIGNED_SHORT_5_5_5_1:
    return out << "GL_UNSIGNED_SHORT_5_5_5_1";
  case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    return out << "GL_UNSIGNED_SHORT_1_5_5_5_REV";
  case GL_UNSIGNED_INT_8_8_8_8:
    return out << "GL_UNSIGNED_INT_8_8_8_8";
  case GL_UNSIGNED_INT_8_8_8_8_REV:
    return out << "GL_UNSIGNED_INT_8_8_8_8_REV";
  case GL_UNSIGNED_INT_10_10_10_2:
    return out << "GL_UNSIGNED_INT_10_10_10_2";
  case GL_UNSIGNED_INT_2_10_10_10_REV:
    return out << "GL_UNSIGNED_INT_2_10_10_10_REV";
  case GL_LIGHT_MODEL_COLOR_CONTROL:
    return out << "GL_LIGHT_MODEL_COLOR_CONTROL";
  case GL_SINGLE_COLOR:
    return out << "GL_SINGLE_COLOR";
  case GL_SEPARATE_SPECULAR_COLOR:
    return out << "GL_SEPARATE_SPECULAR_COLOR";
  case GL_TEXTURE_MIN_LOD:
    return out << "GL_TEXTURE_MIN_LOD";
  case GL_TEXTURE_MAX_LOD:
    return out << "GL_TEXTURE_MAX_LOD";
  case GL_TEXTURE_BASE_LEVEL:
    return out << "GL_TEXTURE_BASE_LEVEL";
  case GL_TEXTURE_MAX_LEVEL:
    return out << "GL_TEXTURE_MAX_LEVEL";
  }

  return out << (int)v;
}
#endif
