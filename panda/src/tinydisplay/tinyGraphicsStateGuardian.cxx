// Filename: tinyGraphicsStateGuardian.cxx
// Created by:  drose (24Apr08)
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

#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "config_tinydisplay.h"
#include "pStatTimer.h"
#include "geomVertexReader.h"
#include "ambientLight.h"
#include "pointLight.h"
#include "directionalLight.h"
#include "spotlight.h"
#include "bitMask.h"
#include "zgl.h"
#include "zmath.h"
#include "ztriangle_table.h"
#include "store_pixel_table.h"

TypeHandle TinyGraphicsStateGuardian::_type_handle;

PStatCollector TinyGraphicsStateGuardian::_vertices_immediate_pcollector("Vertices:Immediate mode");
PStatCollector TinyGraphicsStateGuardian::_draw_transform_pcollector("Draw:Transform");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_white_untextured_pcollector("Pixels:White untextured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_flat_untextured_pcollector("Pixels:Flat untextured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_smooth_untextured_pcollector("Pixels:Smooth untextured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_white_textured_pcollector("Pixels:White textured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_flat_textured_pcollector("Pixels:Flat textured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_smooth_textured_pcollector("Pixels:Smooth textured");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_white_perspective_pcollector("Pixels:White perspective");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_flat_perspective_pcollector("Pixels:Flat perspective");
PStatCollector TinyGraphicsStateGuardian::_pixel_count_smooth_perspective_pcollector("Pixels:Smooth perspective");

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsStateGuardian::
TinyGraphicsStateGuardian(GraphicsPipe *pipe,
			 TinyGraphicsStateGuardian *share_with) :
  GraphicsStateGuardian(CS_yup_right, pipe),
  _textures_lru("textures_lru", td_texture_ram)
{
  _current_frame_buffer = NULL;
  _aux_frame_buffer = NULL;
  _c = NULL;
  _vertices = NULL;
  _vertices_size = 0;
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

  if (_c != (GLContext *)NULL) {
    glClose(_c);
    _c = NULL;
  }

  _c = (GLContext *)gl_zalloc(sizeof(GLContext));
  glInit(_c, _current_frame_buffer);

  _c->draw_triangle_front = gl_draw_triangle_fill;
  _c->draw_triangle_back = gl_draw_triangle_fill;

  _supported_geom_rendering =
    Geom::GR_point | 
    Geom::GR_indexed_other |
    Geom::GR_flat_last_vertex;

  _max_texture_dimension = (1 << ZB_POINT_ST_FRAC_BITS);
  _max_lights = MAX_LIGHTS;

  _color_scale_via_lighting = false;
  _alpha_scale_via_texture = false;
  _runtime_color_scale = true;

  // Now that the GSG has been initialized, make it available for
  // optimizations.
  add_gsg(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
free_pointers() {
  if (_aux_frame_buffer != (ZBuffer *)NULL) {
    ZB_close(_aux_frame_buffer);
    _aux_frame_buffer = NULL;
  }

  if (_vertices != (GLVertex *)NULL) {
    PANDA_FREE_ARRAY(_vertices);
    _vertices = NULL;
  }
  _vertices_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::close_gsg
//       Access: Protected, Virtual
//  Description: This is called by the associated GraphicsWindow when
//               close_window() is called.  It should null out the
//               _win pointer and possibly free any open resources
//               associated with the GSG.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
close_gsg() {
  GraphicsStateGuardian::close_gsg();

  if (_c != (GLContext *)NULL) {
    glClose(_c);
    _c = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
depth_offset_decals() {
  return false;
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

  bool clear_color = false;
  int r, g, b, a;
  if (clearable->get_clear_color_active()) {
    Colorf v = clearable->get_clear_color();
    r = (int)(v[0] * 0xffff);
    g = (int)(v[1] * 0xffff);
    b = (int)(v[2] * 0xffff);
    a = (int)(v[3] * 0xffff);
    clear_color = true;
  }
  
  bool clear_z = false;
  int z;
  if (clearable->get_clear_depth_active()) {
    // We ignore the specified depth clear value, since we don't
    // support alternate depth compare functions anyway.
    z = 0;
    clear_z = true;
  }

  ZB_clear_viewport(_c->zb, clear_z, z,
                    clear_color, r, g, b, a,
                    _c->viewport.xmin, _c->viewport.ymin,
                    _c->viewport.xsize, _c->viewport.ysize);
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

  int xmin, ymin, xsize, ysize;
  dr->get_region_pixels_i(xmin, ymin, xsize, ysize);

  float pixel_factor = _current_display_region->get_pixel_factor();
  if (pixel_factor != 1.0) {
    // Render into an aux buffer, and zoom it up into the main
    // frame buffer later.
    xmin = 0;
    ymin = 0;
    xsize = int(xsize * pixel_factor);
    ysize = int(ysize * pixel_factor);
    if (_aux_frame_buffer == (ZBuffer *)NULL) {
      // We add 3 to xsize, since ZB_open may resize the frame buffer
      // down by up to 3 pixels to make it fit within the
      // word-alignment rule.
      _aux_frame_buffer = ZB_open(xsize + 3, ysize, ZB_MODE_RGBA, 0, 0, 0, 0);
    } else if (_aux_frame_buffer->xsize < xsize || _aux_frame_buffer->ysize < ysize) {
      ZB_resize(_aux_frame_buffer, NULL, 
                max(_aux_frame_buffer->xsize, xsize) + 3,
                max(_aux_frame_buffer->ysize, ysize));
    }
    _c->zb = _aux_frame_buffer;

  } else {
    // Render directly into the main frame buffer.
    _c->zb = _current_frame_buffer;
  }

  _c->viewport.xmin = xmin;
  _c->viewport.ymin = ymin;
  _c->viewport.xsize = xsize;
  _c->viewport.ysize = ysize;
  set_scissor(0.0f, 1.0f, 0.0f, 1.0f);
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
  _transform_stale = true;
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

  _c->zb = _current_frame_buffer;

#ifdef DO_PSTATS
  _vertices_immediate_pcollector.clear_level();

  _pixel_count_white_untextured_pcollector.clear_level();
  _pixel_count_flat_untextured_pcollector.clear_level();
  _pixel_count_smooth_untextured_pcollector.clear_level();
  _pixel_count_white_textured_pcollector.clear_level();
  _pixel_count_flat_textured_pcollector.clear_level();
  _pixel_count_smooth_textured_pcollector.clear_level();
  _pixel_count_white_perspective_pcollector.clear_level();
  _pixel_count_flat_perspective_pcollector.clear_level();
  _pixel_count_smooth_perspective_pcollector.clear_level();
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
//     Function: TinyGraphicsStateGuardian::end_scene
//       Access: Protected, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_scene() {
  if (_c->zb == _aux_frame_buffer) {
    // Copy the aux frame buffer into the main scene now, zooming it
    // up to the appropriate size.
    int xmin, ymin, xsize, ysize;
    _current_display_region->get_region_pixels_i(xmin, ymin, xsize, ysize);
    float pixel_factor = _current_display_region->get_pixel_factor();

    int fb_xsize = int(xsize * pixel_factor);
    int fb_ysize = int(ysize * pixel_factor);

    int tyinc = _current_frame_buffer->linesize / PSZB;
    int fyinc = _aux_frame_buffer->linesize / PSZB;

    int fyt = 0;
    for (int ty = 0; ty < ysize; ++ty) {
      int fy = fyt / ysize;
      fyt += fb_ysize;

      PIXEL *tp = _current_frame_buffer->pbuf + xmin + (ymin + ty) * tyinc;
      PIXEL *fp = _aux_frame_buffer->pbuf + fy * fyinc;
      ZPOINT *tz = _current_frame_buffer->zbuf + xmin + (ymin + ty) * _current_frame_buffer->xsize;
      ZPOINT *fz = _aux_frame_buffer->zbuf + fy * _aux_frame_buffer->xsize;
      int fxt = 0;
      for (int tx = 0; tx < xsize; ++tx) {
        int fx = fxt / xsize;
        fxt += fb_xsize;

        tp[tx] = fp[fx];
        tz[tx] = fz[fx];
      }
    }
    _c->zb = _current_frame_buffer;
  }
  GraphicsStateGuardian::end_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  GraphicsStateGuardian::end_frame(current_thread);

#ifdef DO_PSTATS
  // Flush any PCollectors specific to this kind of GSG.
  _vertices_immediate_pcollector.flush_level();

  _pixel_count_white_untextured_pcollector.flush_level();
  _pixel_count_flat_untextured_pcollector.flush_level();
  _pixel_count_smooth_untextured_pcollector.flush_level();
  _pixel_count_white_textured_pcollector.flush_level();
  _pixel_count_flat_textured_pcollector.flush_level();
  _pixel_count_smooth_textured_pcollector.flush_level();
  _pixel_count_white_perspective_pcollector.flush_level();
  _pixel_count_flat_perspective_pcollector.flush_level();
  _pixel_count_smooth_perspective_pcollector.flush_level();
#endif  // DO_PSTATS

  // Evict any textures that exceed our texture memory.
  _textures_lru.begin_epoch();
}


////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::begin_draw_primitives
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

  PStatTimer timer(_draw_transform_pcollector);

  // Set up the proper transform.
  if (_data_reader->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).
    const TransformState *ident = TransformState::make_identity();
    load_matrix(&_c->matrix_model_view, ident);
    load_matrix(&_c->matrix_projection, _scissor_mat);
    load_matrix(&_c->matrix_model_view_inv, ident);
    load_matrix(&_c->matrix_model_projection, _scissor_mat);
    _c->matrix_model_projection_no_w_transform = 1;
    _transform_stale = true;

  } else if (_transform_stale) {
    // Load the actual transform.

    CPT(TransformState) scissor_proj_mat = _scissor_mat->compose(_projection_mat);

    if (_c->lighting_enabled) {
      // With the lighting equation, we need to keep the modelview and
      // projection matrices separate.

      load_matrix(&_c->matrix_model_view, _internal_transform);
      load_matrix(&_c->matrix_projection, scissor_proj_mat);

      /* precompute inverse modelview */
      M4 tmp;
      gl_M4_Inv(&tmp, &_c->matrix_model_view);
      gl_M4_Transpose(&_c->matrix_model_view_inv, &tmp);

    }

    // Compose the modelview and projection matrices.
    load_matrix(&_c->matrix_model_projection, 
                scissor_proj_mat->compose(_internal_transform));

    /* test to accelerate computation */
    _c->matrix_model_projection_no_w_transform = 0;
    float *m = &_c->matrix_model_projection.m[0][0];
    if (m[12] == 0.0 && m[13] == 0.0 && m[14] == 0.0) {
      _c->matrix_model_projection_no_w_transform = 1;
    }
    _transform_stale = false;
  }

  // Figure out the subset of vertices we will be using in this
  // operation.
  int num_vertices = data_reader->get_num_rows();
  _min_vertex = num_vertices;
  _max_vertex = 0;
  int num_prims = geom_reader->get_num_primitives();
  int i;
  for (i = 0; i < num_prims; ++i) {
    CPT(GeomPrimitive) prim = geom_reader->get_primitive(i);
    int nv = prim->get_min_vertex();
    _min_vertex = min(_min_vertex, nv);
    int xv = prim->get_max_vertex();
    _max_vertex = max(_max_vertex, xv);
  }
  if (_min_vertex > _max_vertex) {
    return false;
  }

  // Now copy all of those vertices into our working table,
  // transforming into screen space them as we go.
  int num_used_vertices = _max_vertex - _min_vertex + 1;
  if (_vertices_size < num_used_vertices) {
    if (_vertices_size == 0) {
      _vertices_size = 1;
    }
    while (_vertices_size < num_used_vertices) {
      _vertices_size *= 2;
    }
    if (_vertices != (GLVertex *)NULL) {
      PANDA_FREE_ARRAY(_vertices);
    }
    _vertices = (GLVertex *)PANDA_MALLOC_ARRAY(_vertices_size * sizeof(GLVertex));
  }

  GeomVertexReader rtexcoord, rcolor, rnormal;

  // We only support single-texturing, so only bother with the first
  // texture stage.
  bool needs_texcoord = false;
  bool needs_texmat = false;
  LMatrix4f texmat;
  const InternalName *texcoord_name = InternalName::get_texcoord();
  int max_stage_index = _effective_texture->get_num_on_ff_stages();
  if (max_stage_index > 0) {
    TextureStage *stage = _effective_texture->get_on_ff_stage(0);
    rtexcoord = GeomVertexReader(data_reader, stage->get_texcoord_name(),
                                 force);
    rtexcoord.set_row(_min_vertex);
    needs_texcoord = rtexcoord.has_column();

    if (needs_texcoord && _target._tex_matrix->has_stage(stage)) {
      needs_texmat = true;
      texmat = _target._tex_matrix->get_mat(stage);
    }
  }

  bool needs_color = false;
  if (_vertex_colors_enabled) {
    rcolor = GeomVertexReader(data_reader, InternalName::get_color(), force);
    rcolor.set_row(_min_vertex);
    needs_color = rcolor.has_column();
  }

  if (!needs_color) {
    const Colorf &d = _scene_graph_color;
    const Colorf &s = _current_color_scale;
    _c->current_color.X = d[0] * s[0];
    _c->current_color.Y = d[1] * s[1];
    _c->current_color.Z = d[2] * s[2];
    _c->current_color.W = d[3] * s[3];
  }

  bool needs_normal = false;
  if (_c->lighting_enabled) {
    rnormal = GeomVertexReader(data_reader, InternalName::get_normal(), force);
    rnormal.set_row(_min_vertex);
    needs_normal = rnormal.has_column();
  }

  GeomVertexReader rvertex(data_reader, InternalName::get_vertex(), force); 
  rvertex.set_row(_min_vertex);

  if (!rvertex.has_column()) {
    // Whoops, guess the vertex data isn't resident.
    return false;
  }

  if (!needs_color && _color_material_flags) {
    if (_color_material_flags & CMF_ambient) {
      _c->materials[0].ambient = _c->current_color;
      _c->materials[1].ambient = _c->current_color;
    }
    if (_color_material_flags & CMF_diffuse) {
      _c->materials[0].diffuse = _c->current_color;
      _c->materials[1].diffuse = _c->current_color;
    }
  }

  if (_texturing_state != 0 && _texture_replace) {
    // We don't need the vertex color or lighting calculation after
    // all, since the current texture will just hide all of that.
    needs_color = false;
    needs_normal = false;
  }

  bool lighting_enabled = (needs_normal && _c->lighting_enabled);

  for (i = 0; i < num_used_vertices; ++i) {
    GLVertex *v = &_vertices[i];
    const LVecBase4f &d = rvertex.get_data4f();
    
    v->coord.X = d[0];
    v->coord.Y = d[1];
    v->coord.Z = d[2];
    v->coord.W = d[3];

    if (needs_texmat) {
      // Transform texcoords as a four-component vector for most generality.
      LVecBase4f d = rtexcoord.get_data4f() * texmat;
      v->tex_coord.X = d[0];
      v->tex_coord.Y = d[1];

    } else if (needs_texcoord) {
      // No need to transform, so just extract as two-component.
      const LVecBase2f &d = rtexcoord.get_data2f();
      v->tex_coord.X = d[0];
      v->tex_coord.Y = d[1];
    }

    if (needs_color) {
      const Colorf &d = rcolor.get_data4f();
      const Colorf &s = _current_color_scale;
      _c->current_color.X = d[0] * s[0];
      _c->current_color.Y = d[1] * s[1];
      _c->current_color.Z = d[2] * s[2];
      _c->current_color.W = d[3] * s[3];
      
      if (_color_material_flags) {
        if (_color_material_flags & CMF_ambient) {
          _c->materials[0].ambient = _c->current_color;
          _c->materials[1].ambient = _c->current_color;
        }
        if (_color_material_flags & CMF_diffuse) {
          _c->materials[0].diffuse = _c->current_color;
          _c->materials[1].diffuse = _c->current_color;
        }
      }
    }

    v->color = _c->current_color;

    if (lighting_enabled) {
      const LVecBase3f &d = rnormal.get_data3f();
      _c->current_normal.X = d[0];
      _c->current_normal.Y = d[1];
      _c->current_normal.Z = d[2];
      _c->current_normal.W = 0.0f;

      gl_vertex_transform(_c, v);
      gl_shade_vertex(_c, v);

    } else {
      gl_vertex_transform(_c, v);
    }

    if (v->clip_code == 0) {
      gl_transform_to_viewport(_c, v);
    }

    v->edge_flag = 1;
  }

  // Set up the appropriate function callback for filling triangles,
  // according to the current state.

  int depth_write_state = 0;  // zon
  if (_target._depth_write->get_mode() != DepthWriteAttrib::M_on) {
    depth_write_state = 1;  // zoff
  }

  int color_write_state = 0;  // cstore
  switch (_target._transparency->get_mode()) {
  case TransparencyAttrib::M_alpha:
  case TransparencyAttrib::M_dual:
    color_write_state = 1;    // cblend
    break;

  default:
    break;
  }

  if (_target._color_blend->get_mode() == ColorBlendAttrib::M_add) {
    // If we have a color blend set that we can support, it overrides
    // the transparency set.
    int op_a = get_color_blend_op(_target._color_blend->get_operand_a());
    int op_b = get_color_blend_op(_target._color_blend->get_operand_b());
    _c->zb->store_pix_func = store_pixel_funcs[op_a][op_b];
    Colorf c = _target._color_blend->get_color();
    _c->zb->blend_r = (int)(c[0] * ZB_POINT_RED_MAX);
    _c->zb->blend_g = (int)(c[1] * ZB_POINT_GREEN_MAX);
    _c->zb->blend_b = (int)(c[2] * ZB_POINT_BLUE_MAX);
    _c->zb->blend_a = (int)(c[3] * ZB_POINT_ALPHA_MAX);

    color_write_state = 2;     // cgeneral
  }

  unsigned int color_channels =
    _target._color_write->get_channels() & _color_write_mask;
  if (color_channels == ColorWriteAttrib::C_off) {
    color_write_state = 3;    // coff
  }

  int alpha_test_state = 0;   // anone
  switch (_target._alpha_test->get_mode()) {
  case AlphaTestAttrib::M_none:
  case AlphaTestAttrib::M_never:
  case AlphaTestAttrib::M_always:
  case AlphaTestAttrib::M_equal:
  case AlphaTestAttrib::M_not_equal:
    alpha_test_state = 0;    // anone
    break;

  case AlphaTestAttrib::M_less:
  case AlphaTestAttrib::M_less_equal:
    alpha_test_state = 1;    // aless
    _c->zb->reference_alpha = (int)_target._alpha_test->get_reference_alpha() * ZB_POINT_ALPHA_MAX;
    break;

  case AlphaTestAttrib::M_greater:
  case AlphaTestAttrib::M_greater_equal:
    alpha_test_state = 2;    // amore
    _c->zb->reference_alpha = (int)_target._alpha_test->get_reference_alpha() * ZB_POINT_ALPHA_MAX;
    break;
  }

  int depth_test_state = 1;    // zless
  _c->depth_test = 1;  // set this for ZB_line
  if (_target._depth_test->get_mode() == DepthTestAttrib::M_none) {
    depth_test_state = 0;      // zless
    _c->depth_test = 0;
  }
  
  ShadeModelAttrib::Mode shade_model = _target._shade_model->get_mode();
  if (!needs_normal && !needs_color) {
    // With no per-vertex lighting, and no per-vertex colors, we might
    // as well use the flat shading model.
    shade_model = ShadeModelAttrib::M_flat;
  }
  int shade_model_state = 2;  // smooth
  _c->smooth_shade_model = true;

  if (shade_model == ShadeModelAttrib::M_flat) {
    _c->smooth_shade_model = false;
    shade_model_state = 1;  // flat
    if (_c->current_color.X == 1.0f &&
        _c->current_color.Y == 1.0f &&
        _c->current_color.Z == 1.0f &&
        _c->current_color.W == 1.0f) {
      shade_model_state = 0;  // white
    }
  }

  int texturing_state = _texturing_state;
  int texfilter_state = 0;  // tnearest
  if (texturing_state > 0) {
    texfilter_state = _texfilter_state;
    if (_c->matrix_model_projection_no_w_transform) {
      // Don't bother with the perspective-correct algorithm if we're
      // under an orthonormal lens, e.g. render2d.
      texturing_state = 1;    // textured (not perspective correct)
    }

    if (_texture_replace) {
      // If we're completely replacing the underlying color, then it
      // doesn't matter what the color is.
      shade_model_state = 0;
    }
  }

  _c->zb_fill_tri = fill_tri_funcs[depth_write_state][color_write_state][alpha_test_state][depth_test_state][texfilter_state][shade_model_state][texturing_state];

#ifdef DO_PSTATS
  pixel_count_white_untextured = 0;
  pixel_count_flat_untextured = 0;
  pixel_count_smooth_untextured = 0;
  pixel_count_white_textured = 0;
  pixel_count_flat_textured = 0;
  pixel_count_smooth_textured = 0;
  pixel_count_white_perspective = 0;
  pixel_count_flat_perspective = 0;
  pixel_count_smooth_perspective = 0;
#endif  // DO_PSTATS
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_triangles
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

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; vi += 3) {
      GLVertex *v0 = &_vertices[vi + delta];
      GLVertex *v1 = &_vertices[vi + delta + 1];
      GLVertex *v2 = &_vertices[vi + delta + 2];
      gl_draw_triangle(_c, v0, v1, v2);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_lines
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

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; vi += 2) {
      GLVertex *v0 = &_vertices[vi + delta];
      GLVertex *v1 = &_vertices[vi + delta + 1];
      gl_draw_line(_c, v0, v1);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_points
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

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(force);
        if (index == NULL) {
          return false;
        }
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; ++vi) {
      GLVertex *v0 = &_vertices[vi + delta];
      gl_draw_point(_c, v0);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_draw_primitives() {

#ifdef DO_PSTATS
  _pixel_count_white_untextured_pcollector.add_level(pixel_count_white_untextured);
  _pixel_count_flat_untextured_pcollector.add_level(pixel_count_flat_untextured);
  _pixel_count_smooth_untextured_pcollector.add_level(pixel_count_smooth_untextured);
  _pixel_count_white_textured_pcollector.add_level(pixel_count_white_textured);
  _pixel_count_flat_textured_pcollector.add_level(pixel_count_flat_textured);
  _pixel_count_smooth_textured_pcollector.add_level(pixel_count_smooth_textured);
  _pixel_count_white_perspective_pcollector.add_level(pixel_count_white_perspective);
  _pixel_count_flat_perspective_pcollector.add_level(pixel_count_flat_perspective);
  _pixel_count_smooth_perspective_pcollector.add_level(pixel_count_smooth_perspective);
#endif  // DO_PSTATS

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
  nassertv(tex != NULL && dr != NULL);
  
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  tex->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgba);

  TextureContext *tc = tex->prepare_now(get_prepared_objects(), this);
  nassertv(tc != (TextureContext *)NULL);
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  GLTexture *gltex = &gtc->_gltex;
  if (!setup_gltex(gltex, tex->get_x_size(), tex->get_y_size(), 1)) {
    return;
  }

  PIXEL *ip = gltex->levels[0].pixmap + gltex->xsize * gltex->ysize;
  PIXEL *fo = _c->zb->pbuf + xo + yo * _c->zb->linesize / PSZB;
  for (int y = 0; y < gltex->ysize; ++y) {
    ip -= gltex->xsize;
    memcpy(ip, fo, gltex->xsize * PSZB);
    fo += _c->zb->linesize / PSZB;
  }

  gtc->update_data_size_bytes(gltex->xsize * gltex->ysize * 4);
  gtc->mark_loaded();
  gtc->enqueue_lru(&_textures_lru);
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
  nassertr(tex != NULL && dr != NULL, false);
  
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  Texture::TextureType texture_type;
  int z_size;
  if (z >= 0) {
    texture_type = Texture::TT_cube_map;
    z_size = 6;
  } else {
    texture_type = Texture::TT_2d_texture;
    z_size = 1;
  }

  Texture::ComponentType component_type = Texture::T_unsigned_byte;
  Texture::Format format = Texture::F_rgba;

  if (tex->get_x_size() != w || tex->get_y_size() != h ||
      tex->get_z_size() != z_size ||
      tex->get_component_type() != component_type ||
      tex->get_format() != format ||
      tex->get_texture_type() != texture_type) {
    // Re-setup the texture; its properties have changed.
    tex->setup_texture(texture_type, w, h, z_size,
                       component_type, format);
  }

  unsigned char *image_ptr = tex->modify_ram_image();
  size_t image_size = tex->get_ram_image_size();
  if (z >= 0) {
    nassertr(z < tex->get_z_size(), false);
    image_size = tex->get_expected_ram_page_size();
    image_ptr += z * image_size;
  }

  PIXEL *ip = (PIXEL *)(image_ptr + image_size);
  PIXEL *fo = _c->zb->pbuf + xo + yo * _c->zb->linesize / PSZB;
  for (int y = 0; y < h; ++y) {
    ip -= w;
    memcpy(ip, fo, w * PSZB);
    fo += _c->zb->linesize / PSZB;
  }

  return true;
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

  if (_target._scissor != _state._scissor) {
    do_issue_scissor();
    _state._scissor = _target._scissor;
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

  TinyTextureContext *gtc = new TinyTextureContext(_prepared_objects, tex);

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

  _texturing_state = 0;  // just in case

  GLTexture *gltex = &gtc->_gltex;
  if (gltex->allocated_buffer != NULL) {
    nassertv(gltex->num_levels != 0);
    TinyTextureContext::get_class_type().dec_memory_usage(TypeHandle::MC_array, gltex->total_bytecount);
    PANDA_FREE_ARRAY(gltex->allocated_buffer);
    gltex->allocated_buffer = NULL;
    gltex->total_bytecount = 0;
    gltex->num_levels = 0;
  } else {
    nassertv(gltex->num_levels == 0);
  }

  gtc->dequeue_lru();

  delete gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_light
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_light() {
  // Initialize the current ambient light total and newly enabled
  // light list
  Colorf cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);

  int num_enabled = 0;
  int num_on_lights = 0;

  if (display_cat.is_spam()) {
    display_cat.spam()
      << "do_issue_light: " << _target._light << "\n";
  }

  // First, release all of the previously-assigned lights.
  _c->lighting_enabled = false;

  GLLight *gl_light = _c->first_light;
  while (gl_light != (GLLight *)NULL) {
    GLLight *next = gl_light->next;
    gl_light->next = NULL;
    gl_light = next;
  }
  _c->first_light = NULL;

  // Now, assign new lights.
  if (_target._light != (LightAttrib *)NULL) {
    CPT(LightAttrib) new_light = _target._light->filter_to_max(_max_lights);
    if (display_cat.is_spam()) {
      new_light->write(display_cat.spam(false), 2);
    }

    num_on_lights = new_light->get_num_on_lights();
    for (int li = 0; li < num_on_lights; li++) {
      NodePath light = new_light->get_on_light(li);
      nassertv(!light.is_empty());
      Light *light_obj = light.node()->as_light();
      nassertv(light_obj != (Light *)NULL);

      _lighting_enabled = true;
      _c->lighting_enabled = true;

      if (light_obj->get_type() == AmbientLight::get_class_type()) {
        // Accumulate all of the ambient lights together into one.
        cur_ambient_light += light_obj->get_color();

      } else {
        // Other kinds of lights each get their own GLLight object.
        nassertv(num_enabled < MAX_LIGHTS);
        GLLight *gl_light = &_c->lights[num_enabled];
        memset(gl_light, 0, sizeof(GLLight));

        gl_light->next = _c->first_light;
        _c->first_light = gl_light;

        const Colorf &diffuse = light_obj->get_color();
        gl_light->diffuse.X = diffuse[0];
        gl_light->diffuse.Y = diffuse[1];
        gl_light->diffuse.Z = diffuse[2];
        gl_light->diffuse.W = diffuse[3];

        light_obj->bind(this, light, num_enabled);
        num_enabled++;
      }
    }
  }

  _c->ambient_light_model.X = cur_ambient_light[0];
  _c->ambient_light_model.Y = cur_ambient_light[1];
  _c->ambient_light_model.Z = cur_ambient_light[2];
  _c->ambient_light_model.W = cur_ambient_light[3];
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  LPoint3f pos = light_obj->get_point() * net_transform->get_mat();
  gl_light->position.X = pos[0];
  gl_light->position.Y = pos[1];
  gl_light->position.Z = pos[2];
  gl_light->position.W = 1.0f;

  // Exponent == 0 implies uniform light distribution
  gl_light->spot_exponent = 0.0f;

  // Cutoff == 180 means uniform point light source
  gl_light->spot_cutoff = 180.0f;

  const LVecBase3f &att = light_obj->get_attenuation();
  gl_light->attenuation[0] = att[0];
  gl_light->attenuation[1] = att[1];
  gl_light->attenuation[2] = att[2];
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];

  // Position needs to specify x, y, z, and w
  // w == 0 implies light is at infinity
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  LVector3f dir = light_obj->get_direction() * net_transform->get_mat();
  gl_light->position.X = -dir[0];
  gl_light->position.Y = -dir[1];
  gl_light->position.Z = -dir[2];
  gl_light->position.W = 0.0f;

  gl_light->norm_position.X = -dir[0];
  gl_light->norm_position.Y = -dir[1];
  gl_light->norm_position.Z = -dir[2];
  gl_V3_Norm(&gl_light->norm_position);

  // Exponent == 0 implies uniform light distribution
  gl_light->spot_exponent = 0.0f;

  // Cutoff == 180 means uniform point light source
  gl_light->spot_cutoff = 180.0f;

  // Default attenuation values (only spotlight and point light can
  // modify these)
  gl_light->attenuation[0] = 1.0f;
  gl_light->attenuation[1] = 0.0f;
  gl_light->attenuation[2] = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];
  
  Lens *lens = light_obj->get_lens();
  nassertv(lens != (Lens *)NULL);

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  const LMatrix4f &light_mat = net_transform->get_mat();
  LPoint3f pos = lens->get_nodal_point() * light_mat;
  LVector3f dir = lens->get_view_vector() * light_mat;

  gl_light->position.X = pos[0];
  gl_light->position.Y = pos[1];
  gl_light->position.Z = pos[2];
  gl_light->position.W = 1.0f;

  gl_light->spot_direction.X = dir[0];
  gl_light->spot_direction.Y = dir[1];
  gl_light->spot_direction.Z = dir[2];

  gl_light->norm_spot_direction.X = dir[0];
  gl_light->norm_spot_direction.Y = dir[1];
  gl_light->norm_spot_direction.Z = dir[2];
  gl_V3_Norm(&gl_light->norm_spot_direction);

  gl_light->spot_exponent = light_obj->get_exponent();
  gl_light->spot_cutoff = lens->get_hfov() * 0.5f;

  const LVecBase3f &att = light_obj->get_attenuation();
  gl_light->attenuation[0] = att[0];
  gl_light->attenuation[1] = att[1];
  gl_light->attenuation[2] = att[2];
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
  _transform_state_pcollector.add_level(1);
  _transform_stale = true;
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
    _c->draw_triangle_front = gl_draw_triangle_fill;
    _c->draw_triangle_back = gl_draw_triangle_fill;
    break;

  case RenderModeAttrib::M_wireframe:
    _c->draw_triangle_front = gl_draw_triangle_line;
    _c->draw_triangle_back = gl_draw_triangle_line;
    break;

  case RenderModeAttrib::M_point:
    _c->draw_triangle_front = gl_draw_triangle_point;
    _c->draw_triangle_back = gl_draw_triangle_point;
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
    _c->cull_face_enabled = false;
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _c->cull_face_enabled = true;
    _c->cull_clockwise = true;
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _c->cull_face_enabled = true;
    _c->cull_clockwise = false;
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

  // Apply the material parameters to the front face.
  setup_material(&_c->materials[0], material);

  if (material->get_twoside()) {
    // Also apply the material parameters to the back face.
    setup_material(&_c->materials[1], material);
  }

  _c->local_light_model = material->get_local();
  _c->light_model_two_side = material->get_twoside();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_texture() {
  _texturing_state = 0;   // untextured
  _c->texture_2d_enabled = false;

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
  if (!apply_texture(tc)) {
    return;
  }

  // Set a few state cache values.
  _c->texture_2d_enabled = true;

  _texturing_state = 2;   // perspective (perspective-correct texturing)
  if (!td_perspective_textures) {
    _texturing_state = 1;    // textured (not perspective correct)
  }

  Texture::QualityLevel quality_level = _texture_quality_override;
  if (quality_level == Texture::QL_default) {
    quality_level = texture->get_quality_level();
    if (quality_level == Texture::QL_default) {
      quality_level = texture_quality_level;
    }
  }

  if (quality_level == Texture::QL_best) {
    // This is the most generic texture filter.  Slow, but pretty.
    _texfilter_state = 2;  // tgeneral
    _c->zb->tex_minfilter_func = get_tex_filter_func(texture->get_minfilter());
    _c->zb->tex_magfilter_func = get_tex_filter_func(texture->get_magfilter());

    if (texture->get_minfilter() == Texture::FT_nearest &&
        texture->get_magfilter() == Texture::FT_nearest) {
      // This case is inlined.
      _texfilter_state = 0;    // tnearest
    } else if (texture->get_minfilter() == Texture::FT_nearest_mipmap_nearest &&
               texture->get_magfilter() == Texture::FT_nearest) {
      // So is this case.
      _texfilter_state = 1;  // tmipmap
    }

  } else if (quality_level == Texture::QL_fastest) {
    // This is the cheapest texture filter.  We disallow mipmaps and
    // perspective correctness.
    _texfilter_state = 0;    // tnearest
    _texturing_state = 1;    // textured (not perspective correct)
  
  } else {
    // This is the default texture filter.  We use nearest sampling if
    // there are no mipmaps, and mipmap_nearest if there are any
    // mipmaps--these are the two inlined filters.
    _texfilter_state = 0;    // tnearest
    if (texture->uses_mipmaps() && !td_ignore_mipmaps) {
      _texfilter_state = 1;  // tmipmap
    }
  }

  // M_replace means M_replace; anything else is treated the same as
  // M_modulate.
  _texture_replace = (stage->get_mode() == TextureStage::M_replace);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_scissor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_scissor() {
  const LVecBase4f &frame = _target._scissor->get_frame();
  set_scissor(frame[0], frame[1], frame[2], frame[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::set_scissor
//       Access: Private
//  Description: Sets up the scissor region, as a set of coordinates
//               relative to the current viewport.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
set_scissor(float left, float right, float bottom, float top) {
  _c->scissor.left = left;
  _c->scissor.right = right;
  _c->scissor.bottom = bottom;
  _c->scissor.top = top;
  gl_eval_viewport(_c);

  float xsize = right - left;
  float ysize = top - bottom;
  float xcenter = (left + right) - 1.0f;
  float ycenter = (bottom + top) - 1.0f;
  _scissor_mat = TransformState::make_scale(LVecBase3f(1.0f / xsize, 1.0f / ysize, 1.0f))->compose(TransformState::make_pos(LPoint3f(-xcenter, -ycenter, 0.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::apply_texture
//       Access: Protected
//  Description: Updates TinyGL with the current information for this
//               texture, and makes it the current texture available
//               for rendering.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
apply_texture(TextureContext *tc) {
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  gtc->set_active(true);

  GLTexture *gltex = &gtc->_gltex;

  if (gtc->was_image_modified() || gltex->num_levels == 0) {
    // If the texture image was modified, reload the texture.
    bool okflag = upload_texture(gtc);
    if (!okflag) {
      tinydisplay_cat.error()
        << "Could not load " << *gtc->get_texture() << "\n";
    }
    gtc->mark_loaded();
    if (!okflag) {
      return false;
    }
  }
  gtc->enqueue_lru(&_textures_lru);

  _c->current_texture = gltex;
  _c->zb->current_texture = gltex->levels;
  return true;
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

  CPTA_uchar src_image = tex->get_ram_image();
  if (src_image.is_null()) {
    return false;
  }

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "loading texture " << tex->get_name() << "\n";
  }
#ifdef DO_PSTATS
  _data_transferred_pcollector.add_level(tex->get_ram_image_size());
#endif
  GLTexture *gltex = &gtc->_gltex;

  int num_levels = 1;
  if (tex->uses_mipmaps()) {
    if (!tex->has_all_ram_mipmap_images()) {
      tex->generate_ram_mipmap_images();
    }
    num_levels = tex->get_num_ram_mipmap_images();
  }

  if (!setup_gltex(gltex, tex->get_x_size(), tex->get_y_size(), num_levels)) {
    return false;
  }

  int bytecount = 0;
  int xsize = gltex->xsize;
  int ysize = gltex->ysize;

  for (int level = 0; level < gltex->num_levels; ++level) {
    ZTextureLevel *dest = &gltex->levels[level];

    switch (tex->get_format()) {
    case Texture::F_rgb:
    case Texture::F_rgb5:
    case Texture::F_rgb8:
    case Texture::F_rgb12:
    case Texture::F_rgb332:
      copy_rgb_image(dest, xsize, ysize, tex, level);
      break;

    case Texture::F_rgba:
    case Texture::F_rgbm:
    case Texture::F_rgba4:
    case Texture::F_rgba5:
    case Texture::F_rgba8:
    case Texture::F_rgba12:
    case Texture::F_rgba16:
    case Texture::F_rgba32:
      copy_rgba_image(dest, xsize, ysize, tex, level);
      break;

    case Texture::F_luminance:
      copy_lum_image(dest, xsize, ysize, tex, level);
      break;

    case Texture::F_red:
      copy_one_channel_image(dest, xsize, ysize, tex, level, 0);
      break;

    case Texture::F_green:
      copy_one_channel_image(dest, xsize, ysize, tex, level, 1);
      break;

    case Texture::F_blue:
      copy_one_channel_image(dest, xsize, ysize, tex, level, 2);
      break;

    case Texture::F_alpha:
      copy_alpha_image(dest, xsize, ysize, tex, level);
      break;

    case Texture::F_luminance_alphamask:
    case Texture::F_luminance_alpha:
      copy_la_image(dest, xsize, ysize, tex, level);
      break;
    }

    bytecount += xsize * ysize * 4;
    xsize = max(xsize >> 1, 1);
    ysize = max(ysize >> 1, 1);
  }

  gtc->update_data_size_bytes(bytecount);
  
  tex->texture_uploaded();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::setup_gltex
//       Access: Private
//  Description: Sets the GLTexture size, bits, and masks appropriate,
//               and allocates space for a pixmap.  Does not fill the
//               pixmap contents.  Returns true if the texture is a
//               valid size, false otherwise.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
setup_gltex(GLTexture *gltex, int x_size, int y_size, int num_levels) {
  int s_bits = get_tex_shift(x_size);
  int t_bits = get_tex_shift(y_size);
  
  if (s_bits < 0 || t_bits < 0) {
    return false;
  }

  num_levels = min(num_levels, MAX_MIPMAP_LEVELS);

  gltex->xsize = x_size;
  gltex->ysize = y_size;

  gltex->s_max = 1 << (s_bits + ZB_POINT_ST_FRAC_BITS);
  gltex->t_max = 1 << (t_bits + ZB_POINT_ST_FRAC_BITS);

  gltex->num_levels = num_levels;
  
  // We allocate one big buffer, large enough to include all the
  // mipmap levels, and index into that buffer for each level.  This
  // cuts down on the number of individual alloc calls we have to make
  // for each texture.
  int total_bytecount = 0;

  // Count up the total bytes required for all mipmap levels.
  {
    int x = x_size;
    int y = y_size;
    for (int level = 0; level < num_levels; ++level) {
      int bytecount = x * y * 4;
      total_bytecount += bytecount;
      x = max((x >> 1), 1);
      y = max((y >> 1), 1);
    }
  }

  if (gltex->total_bytecount != total_bytecount) {
    if (gltex->allocated_buffer != NULL) {
      PANDA_FREE_ARRAY(gltex->allocated_buffer);
      TinyTextureContext::get_class_type().dec_memory_usage(TypeHandle::MC_array, gltex->total_bytecount);
    }
    gltex->allocated_buffer = PANDA_MALLOC_ARRAY(total_bytecount);
    gltex->total_bytecount = total_bytecount;
    TinyTextureContext::get_class_type().inc_memory_usage(TypeHandle::MC_array, total_bytecount);
  }

  char *next_buffer = (char *)gltex->allocated_buffer;
  char *end_of_buffer = next_buffer + total_bytecount;

  int level = 0;
  ZTextureLevel *dest = NULL;
  while (level < num_levels) {
    dest = &gltex->levels[level];
    int bytecount = x_size * y_size * 4;
    dest->pixmap = (PIXEL *)next_buffer;
    next_buffer += bytecount;
    nassertr(next_buffer <= end_of_buffer, false);

    dest->s_mask = (1 << (s_bits + ZB_POINT_ST_FRAC_BITS)) - (1 << ZB_POINT_ST_FRAC_BITS);
    dest->t_mask = (1 << (t_bits + ZB_POINT_ST_FRAC_BITS)) - (1 << ZB_POINT_ST_FRAC_BITS);
    dest->t_shift = (ZB_POINT_ST_FRAC_BITS - s_bits);
    
    x_size = max((x_size >> 1), 1);
    y_size = max((y_size >> 1), 1);
    s_bits = max(s_bits - 1, 0);
    t_bits = max(t_bits - 1, 0);

    ++level;
  }

  // Fill out the remaining mipmap arrays with copies of the last
  // level, so we don't have to be concerned with running off the end
  // of this array while scanning out triangles.
  while (level < MAX_MIPMAP_LEVELS) {
    gltex->levels[level] = *dest;
    ++level;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::get_tex_shift
//       Access: Private
//  Description: Calculates the bit shift count, such that (1 << shift)
//               == size.  Returns -1 if the size is not a power of 2
//               or is larger than our largest allowable size.
////////////////////////////////////////////////////////////////////
int TinyGraphicsStateGuardian::
get_tex_shift(int orig_size) {
  if ((orig_size & (orig_size - 1)) != 0) {
    // Not a power of 2.
    return -1;
  }
  if (orig_size > _max_texture_dimension) {
    return -1;
  }

  return count_bits_in_word((unsigned int)orig_size - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_lum_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel luminance image
//               from the texture into the indicated ZTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_lum_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level) {
  nassertv(tex->get_num_components() == 1);
  nassertv(tex->get_expected_mipmap_x_size(level) == xsize &&
           tex->get_expected_mipmap_y_size(level) == ysize);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = spix[co];
    dpix[1] = spix[co];
    dpix[2] = spix[co];
    dpix[3] = 0xff;
      
    dpix += 4;
    spix += cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_alpha_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel alpha image
//               from the texture into the indicated ZTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_alpha_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level) {
  nassertv(tex->get_num_components() == 1);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = 0xff;
    dpix[1] = 0xff;
    dpix[2] = 0xff;
    dpix[3] = spix[co];
      
    dpix += 4;
    spix += cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_one_channel_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel image (with a
//               single channel, e.g. red, green, or blue) from
//               the texture into the indicated ZTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_one_channel_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level, int channel) {
  nassertv(tex->get_num_components() == 1);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = 0;
    dpix[1] = 0;
    dpix[2] = 0;
    dpix[3] = 0xff;
    dpix[channel] = spix[co];
      
    dpix += 4;
    spix += cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_la_image
//       Access: Private, Static
//  Description: Copies and scales the two-channel luminance-alpha
//               image from the texture into the indicated ZTexture
//               pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_la_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level) {
  nassertv(tex->get_num_components() == 2);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = spix[co];
    dpix[1] = spix[co];
    dpix[2] = spix[co];
    dpix[3] = spix[cw + co];
      
    dpix += 4;
    spix += 2 * cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgb_image
//       Access: Private, Static
//  Description: Copies and scales the three-channel RGB image from
//               the texture into the indicated ZTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgb_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level) {
  nassertv(tex->get_num_components() == 3);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = spix[co];
    dpix[1] = spix[cw + co];
    dpix[2] = spix[cw + cw + co];
    dpix[3] = 0xff;
      
    dpix += 4;
    spix += 3 * cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgba_image
//       Access: Private, Static
//  Description: Copies and scales the four-channel RGBA image from
//               the texture into the indicated ZTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgba_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level) {
  nassertv(tex->get_num_components() == 4);

  CPTA_uchar src_image = tex->get_ram_mipmap_image(level);
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  unsigned char *dpix = (unsigned char *)dest->pixmap;
  nassertv(dpix != NULL);
  const unsigned char *spix = src;
  int pixel_count = xsize * ysize;
  while (pixel_count-- > 0) {
    dpix[0] = spix[co];
    dpix[1] = spix[cw + co];
    dpix[2] = spix[cw + cw + co];
    dpix[3] = spix[cw + cw + cw + co];
      
    dpix += 4;
    spix += 4 * cw;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::setup_material
//       Access: Private
//  Description: Applies the desired parametesr to the indicated
//               GLMaterial object.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
setup_material(GLMaterial *gl_material, const Material *material) {
  const Colorf &specular = material->get_specular();
  gl_material->specular.X = specular[0];
  gl_material->specular.Y = specular[1];
  gl_material->specular.Z = specular[2];
  gl_material->specular.W = specular[3];

  const Colorf &emission = material->get_emission();
  gl_material->emission.X = emission[0];
  gl_material->emission.Y = emission[1];
  gl_material->emission.Z = emission[2];
  gl_material->emission.W = emission[3];

  gl_material->shininess = material->get_shininess();
  gl_material->shininess_i = (int)((material->get_shininess() / 128.0f) * SPECULAR_BUFFER_RESOLUTION);

  _color_material_flags = CMF_ambient | CMF_diffuse;

  if (material->has_ambient()) {
    const Colorf &ambient = material->get_ambient();
    gl_material->ambient.X = ambient[0];
    gl_material->ambient.Y = ambient[1];
    gl_material->ambient.Z = ambient[2];
    gl_material->ambient.W = ambient[3];

    _color_material_flags &= ~CMF_ambient;
  }

  if (material->has_diffuse()) {
    const Colorf &diffuse = material->get_diffuse();
    gl_material->diffuse.X = diffuse[0];
    gl_material->diffuse.Y = diffuse[1];
    gl_material->diffuse.Z = diffuse[2];
    gl_material->diffuse.W = diffuse[3];

    _color_material_flags &= ~CMF_diffuse;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::load_matrix
//       Access: Private, Static
//  Description: Copies the Panda matrix stored in the indicated
//               TransformState object into the indicated TinyGL
//               matrix.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
load_matrix(M4 *matrix, const TransformState *transform) {
  const LMatrix4f &pm = transform->get_mat();
  for (int i = 0; i < 4; ++i) {
    matrix->m[0][i] = pm.get_cell(i, 0);
    matrix->m[1][i] = pm.get_cell(i, 1);
    matrix->m[2][i] = pm.get_cell(i, 2);
    matrix->m[3][i] = pm.get_cell(i, 3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::get_color_blend_op
//       Access: Private, Static
//  Description: Returns the integer element of store_pixel_funcs (as
//               defined by store_pixel.py) that corresponds to the
//               indicated ColorBlendAttrib operand code.
////////////////////////////////////////////////////////////////////
int TinyGraphicsStateGuardian::
get_color_blend_op(ColorBlendAttrib::Operand operand) {
  switch (operand) {
  case ColorBlendAttrib::O_zero:
    return 0;
  case ColorBlendAttrib::O_one:
    return 1;
  case ColorBlendAttrib::O_incoming_color:
    return 2;
  case ColorBlendAttrib::O_one_minus_incoming_color:
    return 3;
  case ColorBlendAttrib::O_fbuffer_color:
    return 4;
  case ColorBlendAttrib::O_one_minus_fbuffer_color:
    return 5;
  case ColorBlendAttrib::O_incoming_alpha:
    return 6;
  case ColorBlendAttrib::O_one_minus_incoming_alpha:
    return 7;
  case ColorBlendAttrib::O_fbuffer_alpha:
    return 8;
  case ColorBlendAttrib::O_one_minus_fbuffer_alpha:
    return 9;
  case ColorBlendAttrib::O_constant_color:
    return 10;
  case ColorBlendAttrib::O_one_minus_constant_color:
    return 11;
  case ColorBlendAttrib::O_constant_alpha:
    return 12;
  case ColorBlendAttrib::O_one_minus_constant_alpha:
    return 13;

  case ColorBlendAttrib::O_incoming_color_saturate:
    return 1;

  case ColorBlendAttrib::O_color_scale:
    return 10;
  case ColorBlendAttrib::O_one_minus_color_scale:
    return 11;
  case ColorBlendAttrib::O_alpha_scale:
    return 12;
  case ColorBlendAttrib::O_one_minus_alpha_scale:
    return 13;
  }
  return 0;
}
 
////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::get_tex_filter_func
//       Access: Private, Static
//  Description: Returns the pointer to the appropriate filter
//               function according to the texture's filter type.
////////////////////////////////////////////////////////////////////
ZB_lookupTextureFunc TinyGraphicsStateGuardian::
get_tex_filter_func(Texture::FilterType filter) {
  switch (filter) {
  case Texture::FT_nearest:
    return &lookup_texture_nearest;

  case Texture::FT_linear:
    return &lookup_texture_bilinear;

  case Texture::FT_nearest_mipmap_nearest:
    if (td_ignore_mipmaps) {
      return &lookup_texture_nearest;
    }
    return &lookup_texture_mipmap_nearest;

  case Texture::FT_nearest_mipmap_linear:
    if (td_ignore_mipmaps) {
      return &lookup_texture_nearest;
    }
    return &lookup_texture_mipmap_linear;
      
  case Texture::FT_linear_mipmap_nearest:
    if (td_ignore_mipmaps) {
      return &lookup_texture_bilinear;
    }
    return &lookup_texture_mipmap_bilinear;

  case Texture::FT_linear_mipmap_linear:
    if (td_ignore_mipmaps) {
      return &lookup_texture_bilinear;
    }
    return &lookup_texture_mipmap_trilinear;

  default:
    return &lookup_texture_nearest;
  }
}

