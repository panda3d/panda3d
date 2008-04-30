// Filename: tinyGraphicsStateGuardian.cxx
// Created by:  drose (24Apr08)
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

#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "config_tinydisplay.h"
#include "pStatTimer.h"

TypeHandle TinyGraphicsStateGuardian::_type_handle;

PStatCollector TinyGraphicsStateGuardian::_vertices_immediate_pcollector("Vertices:Immediate mode");

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

/*
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
*/

////////////////////////////////////////////////////////////////////
//     Function: uchar_bgra_to_rgb
//  Description: Recopies the given array of pixels, converting from
//               BGRA to RGB arrangement, dropping alpha.
////////////////////////////////////////////////////////////////////
static void
uchar_bgra_to_rgb(unsigned char *dest, const unsigned char *source,
                  int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[2] * source[3] / 255;
    dest[1] = source[1] * source[3] / 255;
    dest[2] = source[0] * source[3] / 255;
    dest += 3;
    source += 4;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: uchar_lum_to_rgb
//  Description: Recopies the given array of pixels, converting from
//               luminance to RGB arrangement.
////////////////////////////////////////////////////////////////////
static void
uchar_lum_to_rgb(unsigned char *dest, const unsigned char *source,
                 int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[0] = source[0];
    dest[1] = source[0];
    dest[2] = source[0];
    dest += 3;
    source += 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: uchar_la_to_rgb
//  Description: Recopies the given array of pixels, converting from
//               luminance-alpha to RGB arrangement.
////////////////////////////////////////////////////////////////////
static void
uchar_la_to_rgb(unsigned char *dest, const unsigned char *source,
                 int num_pixels) {
  for (int i = 0; i < num_pixels; i++) {
    dest[2] = dest[1] = dest[0] = source[0] * source[1] / 255;
    dest += 3;
    source += 2;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: restructure_image
//  Description: Converts the pixels of the image to the appropriate
//               format for TinyGL (i.e. GL_RGB).
////////////////////////////////////////////////////////////////////
static PTA_uchar
restructure_image(Texture *tex) {
  int num_pixels = tex->get_x_size() * tex->get_y_size();
  CPTA_uchar orig_image = tex->get_ram_image();
  PTA_uchar new_image = PTA_uchar::empty_array(num_pixels * 3);

  switch (tex->get_format()) {
  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgb8:
  case Texture::F_rgb12:
  case Texture::F_rgb332:
    uchar_bgr_to_rgb(new_image, orig_image, num_pixels);
    break;

  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgba4:
  case Texture::F_rgba5:
  case Texture::F_rgba8:
  case Texture::F_rgba12:
  case Texture::F_rgba16:
  case Texture::F_rgba32:
    uchar_bgra_to_rgb(new_image, orig_image, num_pixels);
    break;

  case Texture::F_luminance:
  case Texture::F_alpha:
    uchar_lum_to_rgb(new_image, orig_image, num_pixels);
    break;

  case Texture::F_luminance_alphamask:
  case Texture::F_luminance_alpha:
    uchar_la_to_rgb(new_image, orig_image, num_pixels);
    break;

  default:
    break;
  }

  return new_image;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsStateGuardian::
TinyGraphicsStateGuardian(GraphicsPipe *pipe,
			 TinyGraphicsStateGuardian *share_with) :
  GraphicsStateGuardian(CS_yup_right, pipe)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsStateGuardian::
~TinyGraphicsStateGuardian() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
reset() {
  free_pointers();
  GraphicsStateGuardian::reset();

  _supported_geom_rendering =
    Geom::GR_point | 
    Geom::GR_indexed_other |
    Geom::GR_flat_last_vertex;

  _max_texture_dimension = 256;

  // Count the max number of lights
  GLint max_lights;
  glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
  _max_lights = max_lights;

  _color_scale_via_lighting = false;
  _alpha_scale_via_texture = false;

  // Now that the GSG has been initialized, make it available for
  // optimizations.
  add_gsg(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) TinyGraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(TinyGeomMunger) munger = new TinyGeomMunger(this, state);
  return GeomMunger::register_munger(munger, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::clear
//       Access: Public
//  Description: Clears the framebuffer within the current
//               DisplayRegion, according to the flags indicated by
//               the given DrawableRegion object.
//
//               This does not set the DisplayRegion first.  You
//               should call prepare_display_region() to specify the
//               region you wish the clear operation to apply to.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
clear(DrawableRegion *clearable) {
  PStatTimer timer(_clear_pcollector);

  if ((!clearable->get_clear_color_active())&&
      (!clearable->get_clear_depth_active())&&
      (!clearable->get_clear_stencil_active())) {
    return;
  }
  
  set_state_and_transform(RenderState::make_empty(), _internal_transform);

  int mask = 0;
  
  if (clearable->get_clear_color_active()) {
    Colorf v = clearable->get_clear_color();
    glClearColor(v[0],v[1],v[2],v[3]);
    mask |= GL_COLOR_BUFFER_BIT;
  }
  
  if (clearable->get_clear_depth_active()) {
    glClearDepth(clearable->get_clear_depth());
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  
  glClear(mask);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//               scissor region and viewport)
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
prepare_display_region(DisplayRegionPipelineReader *dr,
                       Lens::StereoChannel stereo_channel) {
  nassertv(dr != (DisplayRegionPipelineReader *)NULL);
  GraphicsStateGuardian::prepare_display_region(dr, stereo_channel);

  int l, b, w, h;
  dr->get_region_pixels(l, b, w, h);
  GLint x = GLint(l);
  GLint y = GLint(b);
  GLsizei width = GLsizei(w);
  GLsizei height = GLsizei(h);
  
  glViewport(x, y, width, height);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::calc_projection_mat
//       Access: Public, Virtual
//  Description: Given a lens, calculates the appropriate projection
//               matrix for use with this gsg.  Note that the
//               projection matrix depends a lot upon the coordinate
//               system of the rendering API.
//
//               The return value is a TransformState if the lens is
//               acceptable, NULL if it is not.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TinyGraphicsStateGuardian::
calc_projection_mat(const Lens *lens) {
  if (lens == (Lens *)NULL) {
    return NULL;
  }

  if (!lens->is_linear()) {
    return NULL;
  }

  // The projection matrix must always be right-handed Y-up, even if
  // our coordinate system of choice is otherwise, because certain GL
  // calls (specifically glTexGen(GL_SPHERE_MAP)) assume this kind of
  // a coordinate system.  Sigh.  In order to implement a Z-up (or
  // other arbitrary) coordinate system, we'll use a Y-up projection
  // matrix, and store the conversion to our coordinate system of
  // choice in the modelview matrix.

  LMatrix4f result =
    LMatrix4f::convert_mat(CS_yup_right, _current_lens->get_coordinate_system()) *
    lens->get_projection_mat(_current_stereo_channel);

  if (_scene_setup->get_inverted()) {
    // If the scene is supposed to be inverted, then invert the
    // projection matrix.
    result *= LMatrix4f::scale_mat(1.0f, -1.0f, 1.0f);
  }

  return TransformState::make_mat(result);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with set_scene()) active, so
//               that it will transform future rendered geometry.
//               Normally this is only called from the draw process,
//               and usually it is called by set_scene().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
prepare_lens() {
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << "glLoadMatrix(GL_PROJECTION): " << _projection_mat->get_mat() << endl;
  }
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(_projection_mat->get_mat().get_data());

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
bool TinyGraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

#ifdef DO_PSTATS
  _vertices_immediate_pcollector.clear_level();
#endif

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the beginning of drawing commands for a "scene"
//               (usually a particular DisplayRegion) within a frame.
//               All 3-D drawing commands, except the clear operation,
//               must be enclosed within begin_scene() .. end_scene().
//
//               The return value is true if successful (in which case
//               the scene will be drawn and end_scene() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_scene() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
begin_scene() {
  return GraphicsStateGuardian::begin_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::end_scene
//       Access: Protected, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_scene() {
  GraphicsStateGuardian::end_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  GraphicsStateGuardian::end_frame(current_thread);

  // Flush any PCollectors specific to this kind of GSG.
  _vertices_immediate_pcollector.flush_level();
}


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::begin_draw_primitives
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomMunger *munger,
                      const GeomVertexDataPipelineReader *data_reader,
                      bool force) {
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "begin_draw_primitives: " << *(data_reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, munger, data_reader, force)) {
    return false;
  }
  nassertr(_data_reader != (GeomVertexDataPipelineReader *)NULL, false);

  // We must use immediate mode to render primitives.
  _sender.clear();

  _sender.add_column(_data_reader, InternalName::get_normal(),
                     NULL, NULL, glNormal3f, NULL);
  if (!_sender.add_column(_data_reader, InternalName::get_color(),
                          NULL, NULL, glColor3f, glColor4f)) {
    // If we didn't have a color column, the item color is white.
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  }

  // TinyGL only supports single-texturing, so only bother with the
  // first texture stage.
  int max_stage_index = _effective_texture->get_num_on_ff_stages();
  if (max_stage_index > 0) {
    TextureStage *stage = _effective_texture->get_on_ff_stage(0);
    const InternalName *name = stage->get_texcoord_name();
    _sender.add_column(_data_reader, name,
                       NULL, glTexCoord2f, NULL, NULL);
  }

  // We must add vertex last, because glVertex3f() is the key
  // function call that actually issues the vertex.
  _sender.add_column(_data_reader, InternalName::get_vertex(),
                     NULL, glVertex2f, glVertex3f, glVertex4f);

  if (_transform_stale) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(_internal_transform->get_mat().get_data());
  }

  if (_data_reader->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());

#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_triangles: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  draw_immediate_simple_primitives(reader, GL_TRIANGLES);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_lines: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  draw_immediate_simple_primitives(reader, GL_LINES);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_points: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  draw_immediate_simple_primitives(reader, GL_POINTS);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_draw_primitives() {
  if (_transform_stale) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(_internal_transform->get_mat().get_data());
  }

  if (_data_reader->is_vertex_transformed()) {
    // Restore the matrices that we pushed above.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  GraphicsStateGuardian::end_draw_primitives();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::framebuffer_copy_to_texture
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display
//               region from the framebuffer into texture memory.
//
//               If z > -1, it is the cube map index into which to
//               copy.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr,
                            const RenderBuffer &rb) {
}


////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::framebuffer_copy_to_ram
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display region
//               from the framebuffer into system memory, not texture
//               memory.  Returns true on success, false on failure.
//
//               This completely redefines the ram image of the
//               indicated texture.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr,
                        const RenderBuffer &rb) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::set_state_and_transform
//       Access: Public, Virtual
//  Description: Simultaneously resets the render state and the
//               transform state.
//
//               This transform specified is the "internal" net
//               transform, already converted into the GSG's internal
//               coordinate space by composing it to
//               get_cs_transform().  (Previously, this used to be the
//               "external" net transform, with the assumption that
//               that GSG would convert it internally, but that is no
//               longer the case.)
//
//               Special case: if (state==NULL), then the target
//               state is already stored in _target.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
set_state_and_transform(const RenderState *target,
                        const TransformState *transform) {
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << "Setting GSG state to " << (void *)target << ":\n";
    target->write(tinydisplay_cat.spam(false), 2);
  }
#endif

  _state_pcollector.add_level(1);
  PStatTimer timer1(_draw_set_state_pcollector);

  if (transform != _internal_transform) {
    PStatTimer timer(_draw_set_state_transform_pcollector);
    _state_pcollector.add_level(1);
    _internal_transform = transform;
    do_issue_transform();
  }

  if (target == _state_rs) {
    return;
  }
  _target_rs = target;
  _target.clear_to_defaults();
  target->store_into_slots(&_target);
  _state_rs = 0;

  if (_target._color != _state._color ||
      _target._color_scale != _state._color_scale) {
    PStatTimer timer(_draw_set_state_color_pcollector);
    do_issue_color();
    do_issue_color_scale();
    _state._color = _target._color;
    _state._color_scale = _target._color_scale;
  }

  if (_target._cull_face != _state._cull_face) {
    PStatTimer timer(_draw_set_state_cull_face_pcollector);
    do_issue_cull_face();
    _state._cull_face = _target._cull_face;
  }

  if (_target._render_mode != _state._render_mode) {
    PStatTimer timer(_draw_set_state_render_mode_pcollector);
    do_issue_render_mode();
    _state._render_mode = _target._render_mode;
  }

  if (_target._shade_model != _state._shade_model) {
    PStatTimer timer(_draw_set_state_shade_model_pcollector);
    do_issue_shade_model();
    _state._shade_model = _target._shade_model;
  }

  if ((_target._transparency != _state._transparency)||
      (_target._color_write != _state._color_write)||
      (_target._color_blend != _state._color_blend)) {
    PStatTimer timer(_draw_set_state_blending_pcollector);
    do_issue_blending();
    _state._transparency = _target._transparency;
    _state._color_write = _target._color_write;
    _state._color_blend = _target._color_blend;
  }

  if (_target._texture != _state._texture) {
    PStatTimer timer(_draw_set_state_texture_pcollector);
    determine_effective_texture();
    do_issue_texture();
    _state._texture = _target._texture;
  }
  
  if (_target._material != _state._material) {
    PStatTimer timer(_draw_set_state_material_pcollector);
    do_issue_material();
    _state._material = _target._material;
  }

  if (_target._light != _state._light) {
    PStatTimer timer(_draw_set_state_light_pcollector);
    do_issue_light();
    _state._light = _target._light;
  }

  _state_rs = _target_rs;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description: Creates whatever structures the GSG requires to
//               represent the texture internally, and returns a
//               newly-allocated TextureContext object with this data.
//               It is the responsibility of the calling function to
//               later call release_texture() with this same pointer
//               (which will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *TinyGraphicsStateGuardian::
prepare_texture(Texture *tex) {
  if (tex->get_texture_type() != Texture::TT_2d_texture) {
    tinydisplay_cat.info()
      << "not loading texture " << tex->get_name() << ": "
      << tex->get_texture_type() << "\n";
    return NULL;
  }
  if (tex->get_ram_image_compression() != Texture::CM_off) {
    tinydisplay_cat.info()
      << "not loading texture " << tex->get_name() << ": "
      << tex->get_ram_image_compression() << "\n";
    return NULL;
  }
  if (tex->get_component_type() != Texture::T_unsigned_byte) {
    tinydisplay_cat.info()
      << "not loading texture " << tex->get_name() << ": "
      << tex->get_component_type() << "\n";
    return NULL;
  }

  TinyTextureContext *gtc = new TinyTextureContext(_prepared_objects, tex);
  glGenTextures(1, &gtc->_index);

  apply_texture(gtc);
  return gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.  This function should never be called
//               directly; instead, call Texture::release() (or simply
//               let the Texture destruct).
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
release_texture(TextureContext *tc) {
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  glDeleteTextures(1, &gtc->_index);

  gtc->_index = 0;
  delete gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by do_issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
enable_lighting(bool enable) {
  static PStatCollector _draw_set_state_light_enable_lighting_pcollector("Draw:Set State:Light:Enable lighting");
  PStatTimer timer(_draw_set_state_light_enable_lighting_pcollector);
  
  if (enable) {
    glEnable(GL_LIGHTING);
  } else {
    glDisable(GL_LIGHTING);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by do_issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
set_ambient_light(const Colorf &color) {
  static PStatCollector _draw_set_state_light_ambient_pcollector("Draw:Set State:Light:Ambient");
  PStatTimer timer(_draw_set_state_light_ambient_pcollector);
  
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (float *)color.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
enable_light(int light_id, bool enable) {
  static PStatCollector _draw_set_state_light_enable_light_pcollector("Draw:Set State:Light:Enable light");
  PStatTimer timer(_draw_set_state_light_enable_light_pcollector);
  
  if (enable) {
    glEnable(get_light_id(light_id));
  } else {
    glDisable(get_light_id(light_id));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::begin_bind_lights
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
void TinyGraphicsStateGuardian::
begin_bind_lights() {
  static PStatCollector _draw_set_state_light_begin_bind_pcollector("Draw:Set State:Light:Begin bind");
  PStatTimer timer(_draw_set_state_light_begin_bind_pcollector);
  
  // We need to temporarily load a new matrix so we can define the
  // light in a known coordinate system.  We pick the transform of the
  // root.  (Alternatively, we could leave the current transform where
  // it is and compute the light position relative to that transform
  // instead of relative to the root, by composing with the matrix
  // computed by _internal_transform->invert_compose(render_transform).
  // But I think loading a completely new matrix is simpler.)
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(render_transform->get_mat().get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_bind_lights
//       Access: Protected, Virtual
//  Description: Called after before bind_light() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_lights().
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_bind_lights() {
  static PStatCollector _draw_set_state_light_end_bind_pcollector("Draw:Set State:Light:End bind");
  PStatTimer timer(_draw_set_state_light_end_bind_pcollector);
  
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_transform
//       Access: Protected
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
//
//               This transform is the internal_transform, already
//               converted into the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_transform() {
  const TransformState *transform = _internal_transform;
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << "glLoadMatrix(GL_MODELVIEW): " << transform->get_mat() << endl;
  }

  _transform_state_pcollector.add_level(1);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(transform->get_mat().get_data());
  _transform_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_shade_model
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_shade_model() {
  const ShadeModelAttrib *attrib = _target._shade_model;
  switch (attrib->get_mode()) {
  case ShadeModelAttrib::M_smooth:
    glShadeModel(GL_SMOOTH);
    break;

  case ShadeModelAttrib::M_flat:
    glShadeModel(GL_FLAT);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_render_mode
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_render_mode() {
  const RenderModeAttrib *attrib = _target._render_mode;

  switch (attrib->get_mode()) {
  case RenderModeAttrib::M_unchanged:
  case RenderModeAttrib::M_filled:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;

  case RenderModeAttrib::M_wireframe:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;

  case RenderModeAttrib::M_point:
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;

  default:
    tinydisplay_cat.error()
      << "Unknown render mode " << (int)attrib->get_mode() << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_cull_face
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_cull_face() {
  const CullFaceAttrib *attrib = _target._cull_face;
  CullFaceAttrib::Mode mode = attrib->get_effective_mode();

  switch (mode) {
  case CullFaceAttrib::M_cull_none:
    glDisable(GL_CULL_FACE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    break;
  default:
    tinydisplay_cat.error()
      << "invalid cull face mode " << (int)mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_material
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_material() {
  static Material empty;
  const Material *material;
  if (_target._material == (MaterialAttrib *)NULL ||
      _target._material->is_off()) {
    material = &empty;
  } else {
    material = _target._material->get_material();
  }

  GLenum face = material->get_twoside() ? GL_FRONT_AND_BACK : GL_FRONT;

  glMaterialfv(face, GL_SPECULAR, (GLfloat *)material->get_specular().get_data());
  glMaterialfv(face, GL_EMISSION, (GLfloat *)material->get_emission().get_data());
  glMaterialf(face, GL_SHININESS, material->get_shininess());

  if (material->has_ambient() && material->has_diffuse()) {
    // The material has both an ambient and diffuse specified.  This
    // means we do not need glMaterialColor().
    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(face, GL_AMBIENT, (GLfloat *)material->get_ambient().get_data());
    glMaterialfv(face, GL_DIFFUSE, (GLfloat *)material->get_diffuse().get_data());

  } else if (material->has_ambient()) {
    // The material specifies an ambient, but not a diffuse component.
    // The diffuse component comes from the object's color.
    glMaterialfv(face, GL_AMBIENT, (GLfloat *)material->get_ambient().get_data());
    if (_has_material_force_color) {
      glDisable(GL_COLOR_MATERIAL);
      glMaterialfv(face, GL_DIFFUSE, (GLfloat *)_material_force_color.get_data());
    } else {
      glColorMaterial(face, GL_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
    }

  } else if (material->has_diffuse()) {
    // The material specifies a diffuse, but not an ambient component.
    // The ambient component comes from the object's color.
    glMaterialfv(face, GL_DIFFUSE, (GLfloat *)material->get_diffuse().get_data());
    if (_has_material_force_color) {
      glDisable(GL_COLOR_MATERIAL);
      glMaterialfv(face, GL_AMBIENT, (GLfloat *)_material_force_color.get_data());
    } else {
      glColorMaterial(face, GL_AMBIENT);
      glEnable(GL_COLOR_MATERIAL);
    }

  } else {
    // The material specifies neither a diffuse nor an ambient
    // component.  Both components come from the object's color.
    if (_has_material_force_color) {
      glDisable(GL_COLOR_MATERIAL);
      glMaterialfv(face, GL_AMBIENT, (GLfloat *)_material_force_color.get_data());
      glMaterialfv(face, GL_DIFFUSE, (GLfloat *)_material_force_color.get_data());
    } else {
      glColorMaterial(face, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
    }
  }

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, material->get_local());
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_texture() {
  glDisable(GL_TEXTURE_2D);

  int num_stages = _effective_texture->get_num_on_ff_stages();
  if (num_stages == 0) {
    // No texturing.
    return;
  }
  nassertv(num_stages == 1);

  TextureStage *stage = _effective_texture->get_on_ff_stage(0);
  Texture *texture = _effective_texture->get_on_texture(stage);
  nassertv(texture != (Texture *)NULL);
    
  TextureContext *tc = texture->prepare_now(_prepared_objects, this);
  if (tc == (TextureContext *)NULL) {
    // Something wrong with this texture; skip it.
    return;
  }
    
  // Then, turn on the current texture mode.
  glEnable(GL_TEXTURE_2D);
  apply_texture(tc);

  /*
  GLint glmode = get_texture_apply_mode_type(stage->get_mode());
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, glmode);
  */
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_blending
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_blending() {
  /*
  // Handle the color_write attrib.  If color_write is off, then
  // all the other blending-related stuff doesn't matter.  If the
  // device doesn't support color-write, we use blending tricks
  // to effectively disable color write.
  unsigned int color_channels =
    _target._color_write->get_channels() & _color_write_mask;
  if (color_channels == ColorWriteAttrib::C_off) {
    if (_target._color_write != _state._color_write) {
      enable_multisample_alpha_one(false);
      enable_multisample_alpha_mask(false);
      if (CLP(color_mask)) {
        enable_blend(false);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      } else {
        enable_blend(true);
        _glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ZERO, GL_ONE);
      }
    }
    return;
  } else {
    if (_target._color_write != _state._color_write) {
      if (CLP(color_mask)) {
        glColorMask((color_channels & ColorWriteAttrib::C_red) != 0,
                       (color_channels & ColorWriteAttrib::C_green) != 0,
                       (color_channels & ColorWriteAttrib::C_blue) != 0,
                       (color_channels & ColorWriteAttrib::C_alpha) != 0);
      }
    }
  }

  CPT(ColorBlendAttrib) color_blend = _target._color_blend;
  ColorBlendAttrib::Mode color_blend_mode = _target._color_blend->get_mode();
  TransparencyAttrib::Mode transparency_mode = _target._transparency->get_mode();

  _color_blend_involves_color_scale = color_blend->involves_color_scale();

  // Is there a color blend set?
  if (color_blend_mode != ColorBlendAttrib::M_none) {
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    _glBlendEquation(get_blend_equation_type(color_blend_mode));
    glBlendFunc(get_blend_func(color_blend->get_operand_a()),
                   get_blend_func(color_blend->get_operand_b()));

    if (_color_blend_involves_color_scale) {
      // Apply the current color scale to the blend mode.
      _glBlendColor(_current_color_scale[0], _current_color_scale[1],
                    _current_color_scale[2], _current_color_scale[3]);

    } else {
      Colorf c = color_blend->get_color();
      _glBlendColor(c[0], c[1], c[2], c[3]);
    }
    return;
  }

  // No color blend; is there a transparency set?
  switch (transparency_mode) {
  case TransparencyAttrib::M_none:
  case TransparencyAttrib::M_binary:
    break;

  case TransparencyAttrib::M_alpha:
  case TransparencyAttrib::M_dual:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    _glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return;

  case TransparencyAttrib::M_multisample:
    // We need to enable *both* of these in M_multisample case.
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
    tinydisplay_cat.error()
      << "invalid transparency mode " << (int)transparency_mode << endl;
    break;
  }

  if (_line_smooth_enabled || _point_smooth_enabled) {
    // If we have either of these turned on, we also need to have
    // blend mode enabled in order to see it.
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    _glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return;
  }

  // For best polygon smoothing, we need:
  // (1) a frame buffer that supports alpha
  // (2) sort polygons front-to-back
  // (3) glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
  //
  // Since these modes have other implications for the application, we
  // don't attempt to do this by default.  If you really want good
  // polygon smoothing (and you don't have multisample support), do
  // all this yourself.

  // Nothing's set, so disable blending.
  enable_multisample_alpha_one(false);
  enable_multisample_alpha_mask(false);
  enable_blend(false);
  */
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::apply_texture
//       Access: Protected
//  Description: Updates TinyGL with the current information for this
//               texture, and makes it the current texture available
//               for rendering.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
apply_texture(TextureContext *tc) {
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  gtc->set_active(true);
  glBindTexture(GL_TEXTURE_2D, gtc->_index);

  if (gtc->was_image_modified()) {
    // If the texture image was modified, reload the texture.
    if (!upload_texture(gtc)) {
      glDisable(GL_TEXTURE_2D);
    }
    gtc->mark_loaded();

  } else if (gtc->was_properties_modified()) {
    // If only the properties have been modified, we don't need to
    // reload the texture.
    gtc->mark_loaded();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::upload_texture
//       Access: Protected
//  Description: Uploads the texture image to TinyGL.
//
//               The return value is true if successful, or false if
//               the texture has no image.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
upload_texture(TinyTextureContext *gtc) {
  Texture *tex = gtc->get_texture();
  PStatTimer timer(_load_texture_pcollector);

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "loading texture " << tex->get_name() << "\n";
  }
  CPTA_uchar image = tex->get_ram_image();
  nassertr(!image.is_null(), false)

  int width = tex->get_x_size();
  int height = tex->get_y_size();

  PTA_uchar new_image = restructure_image(tex);
  const unsigned char *image_ptr = new_image.p();

#ifdef DO_PSTATS
  _data_transferred_pcollector.add_level(new_image.size());
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
               width, height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, (void *)image_ptr);
  
#ifdef DO_PSTATS 
  gtc->update_data_size_bytes(256 * 256 * 3);
#endif
  
  tex->texture_uploaded();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_immediate_simple_primitives
//       Access: Private
//  Description: Uses the ImmediateModeSender to draw a series of
//               primitives of the indicated type.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
draw_immediate_simple_primitives(const GeomPrimitivePipelineReader *reader, GLenum mode) {
  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);
  glBegin(mode);

  if (reader->is_indexed()) {
    for (int v = 0; v < num_vertices; ++v) {
      _sender.set_vertex(reader->get_vertex(v));
      _sender.issue_vertex();
    }

  } else {
    _sender.set_vertex(reader->get_first_vertex());
    for (int v = 0; v < num_vertices; ++v) {
      _sender.issue_vertex();
    }
  }

  glEnd();
}
