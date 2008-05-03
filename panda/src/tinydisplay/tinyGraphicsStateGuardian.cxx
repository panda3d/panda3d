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

extern "C" {
#include "zgl.h"
#include "zmath.h"
}

TypeHandle TinyGraphicsStateGuardian::_type_handle;

PStatCollector TinyGraphicsStateGuardian::_vertices_immediate_pcollector("Vertices:Immediate mode");


static const ZB_fillTriangleFunc fill_tri_funcs
[2 /* alpha test: anone, abin (binary) */]
[2 /* ztest: znone, zless */]
[3 /* white, flat, smooth */]
[3 /* untextured, textured, perspective textured */] = {
  { // alpha test anone
    {
      { ZB_fillTriangleFlat_anone_znone,
        ZB_fillTriangleMapping_anone_znone,
        ZB_fillTriangleMappingPerspective_anone_znone },
      { ZB_fillTriangleFlat_anone_znone,
        ZB_fillTriangleMappingFlat_anone_znone,
        ZB_fillTriangleMappingPerspectiveFlat_anone_znone },
      { ZB_fillTriangleSmooth_anone_znone,
        ZB_fillTriangleMappingSmooth_anone_znone,
        ZB_fillTriangleMappingPerspectiveSmooth_anone_znone },
    },
    {
      { ZB_fillTriangleFlat_anone_zless,
        ZB_fillTriangleMapping_anone_zless,
        ZB_fillTriangleMappingPerspective_anone_zless },
      { ZB_fillTriangleFlat_anone_zless,
        ZB_fillTriangleMappingFlat_anone_zless,
        ZB_fillTriangleMappingPerspectiveFlat_anone_zless },
      { ZB_fillTriangleSmooth_anone_zless,
        ZB_fillTriangleMappingSmooth_anone_zless,
        ZB_fillTriangleMappingPerspectiveSmooth_anone_zless },
    },
  },
  { // alpha test abin
    {
      { ZB_fillTriangleFlat_abin_znone,
        ZB_fillTriangleMapping_abin_znone,
        ZB_fillTriangleMappingPerspective_abin_znone },
      { ZB_fillTriangleFlat_abin_znone,
        ZB_fillTriangleMappingFlat_abin_znone,
        ZB_fillTriangleMappingPerspectiveFlat_abin_znone },
      { ZB_fillTriangleSmooth_abin_znone,
        ZB_fillTriangleMappingSmooth_abin_znone,
        ZB_fillTriangleMappingPerspectiveSmooth_abin_znone },
    },
    {
      { ZB_fillTriangleFlat_abin_zless,
        ZB_fillTriangleMapping_abin_zless,
        ZB_fillTriangleMappingPerspective_abin_zless },
      { ZB_fillTriangleFlat_abin_zless,
        ZB_fillTriangleMappingFlat_abin_zless,
        ZB_fillTriangleMappingPerspectiveFlat_abin_zless },
      { ZB_fillTriangleSmooth_abin_zless,
        ZB_fillTriangleMappingSmooth_abin_zless,
        ZB_fillTriangleMappingPerspectiveSmooth_abin_zless },
    },
  },
};

static inline void tgl_vertex_transform(GLContext * c, GLVertex * v)
{
    float *m;
    V4 *n;

    if (c->lighting_enabled) {
	/* eye coordinates needed for lighting */

	m = &c->matrix_stack_ptr[0]->m[0][0];
	v->ec.X = (v->coord.X * m[0] + v->coord.Y * m[1] +
		   v->coord.Z * m[2] + m[3]);
	v->ec.Y = (v->coord.X * m[4] + v->coord.Y * m[5] +
		   v->coord.Z * m[6] + m[7]);
	v->ec.Z = (v->coord.X * m[8] + v->coord.Y * m[9] +
		   v->coord.Z * m[10] + m[11]);
	v->ec.W = (v->coord.X * m[12] + v->coord.Y * m[13] +
		   v->coord.Z * m[14] + m[15]);

	/* projection coordinates */
	m = &c->matrix_stack_ptr[1]->m[0][0];
	v->pc.X = (v->ec.X * m[0] + v->ec.Y * m[1] +
		   v->ec.Z * m[2] + v->ec.W * m[3]);
	v->pc.Y = (v->ec.X * m[4] + v->ec.Y * m[5] +
		   v->ec.Z * m[6] + v->ec.W * m[7]);
	v->pc.Z = (v->ec.X * m[8] + v->ec.Y * m[9] +
		   v->ec.Z * m[10] + v->ec.W * m[11]);
	v->pc.W = (v->ec.X * m[12] + v->ec.Y * m[13] +
		   v->ec.Z * m[14] + v->ec.W * m[15]);

	m = &c->matrix_model_view_inv.m[0][0];
	n = &c->current_normal;

	v->normal.X = (n->X * m[0] + n->Y * m[1] + n->Z * m[2]);
	v->normal.Y = (n->X * m[4] + n->Y * m[5] + n->Z * m[6]);
	v->normal.Z = (n->X * m[8] + n->Y * m[9] + n->Z * m[10]);

	if (c->normalize_enabled) {
	    gl_V3_Norm(&v->normal);
	}
    } else {
	/* no eye coordinates needed, no normal */
	/* NOTE: W = 1 is assumed */
	m = &c->matrix_model_projection.m[0][0];

	v->pc.X = (v->coord.X * m[0] + v->coord.Y * m[1] +
		   v->coord.Z * m[2] + m[3]);
	v->pc.Y = (v->coord.X * m[4] + v->coord.Y * m[5] +
		   v->coord.Z * m[6] + m[7]);
	v->pc.Z = (v->coord.X * m[8] + v->coord.Y * m[9] +
		   v->coord.Z * m[10] + m[11]);
	if (c->matrix_model_projection_no_w_transform) {
	    v->pc.W = m[15];
	} else {
	    v->pc.W = (v->coord.X * m[12] + v->coord.Y * m[13] +
		       v->coord.Z * m[14] + m[15]);
	}
    }

    v->clip_code = gl_clipcode(v->pc.X, v->pc.Y, v->pc.Z, v->pc.W);
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

  _c = gl_get_context();

  _c->draw_triangle_front = gl_draw_triangle_fill;
  _c->draw_triangle_back = gl_draw_triangle_fill;

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
//     Function: TinyGraphicsStateGuardian::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
free_pointers() {
  if (_vertices != (GLVertex *)NULL) {
    PANDA_FREE_ARRAY(_vertices);
    _vertices = NULL;
  }
  _vertices_size = 0;
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
  int xmin = GLint(l);
  int ymin = GLint(b);
  int xsize = GLsizei(w);
  int ysize = GLsizei(h);
  
  int xsize_req=xmin+xsize;
  int ysize_req=ymin+ysize;
  
  if (_c->gl_resize_viewport && 
      _c->gl_resize_viewport(_c,&xsize_req,&ysize_req) != 0) {
    gl_fatal_error("glViewport: error while resizing display");
  }
  
  xsize=xsize_req-xmin;
  ysize=ysize_req-ymin;
  if (xsize <= 0 || ysize <= 0) {
    gl_fatal_error("glViewport: size too small");
  }
  
  _c->viewport.xmin=xmin;
  _c->viewport.ymin=ymin;
  _c->viewport.xsize=xsize;
  _c->viewport.ysize=ysize;
  
  gl_eval_viewport(_c);
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

  // Flush any PCollectors specific to this kind of GSG.
  _vertices_immediate_pcollector.flush_level();
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

  // Set up the proper transform.
  if (_data_reader->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).
    const TransformState *ident = TransformState::make_identity();
    load_matrix(_c->matrix_stack_ptr[0], ident);
    load_matrix(_c->matrix_stack_ptr[1], ident);
    load_matrix(&_c->matrix_model_view_inv, ident);
    load_matrix(&_c->matrix_model_projection, ident);
    _c->matrix_model_projection_no_w_transform = 1;
    _transform_stale = true;

  } else if (_transform_stale) {
    // Load the actual transform.

    if (_c->lighting_enabled) {
      // With the lighting equation, we need to keep the modelview and
      // projection matrices separate.

      load_matrix(_c->matrix_stack_ptr[0], _internal_transform);
      load_matrix(_c->matrix_stack_ptr[1], _projection_mat);

      /* precompute inverse modelview */
      M4 tmp;
      gl_M4_Inv(&tmp, _c->matrix_stack_ptr[0]);
      gl_M4_Transpose(&_c->matrix_model_view_inv, &tmp);

    }

    // Compose the modelview and projection matrices.
    load_matrix(&_c->matrix_model_projection, 
                _projection_mat->compose(_internal_transform));

    /* test to accelerate computation */
    _c->matrix_model_projection_no_w_transform = 0;
    float *m = &_c->matrix_model_projection.m[0][0];
    if (m[12] == 0.0 && m[13] == 0.0 && m[14] == 0.0) {
      _c->matrix_model_projection_no_w_transform = 1;
    }
    _transform_stale = false;
  }
   
  /* test if the texture matrix is not Identity */
  //  _c->apply_texture_matrix = !gl_M4_IsId(_c->matrix_stack_ptr[2]);

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
  const InternalName *texcoord_name = InternalName::get_texcoord();
  int max_stage_index = _effective_texture->get_num_on_ff_stages();
  if (max_stage_index > 0) {
    TextureStage *stage = _effective_texture->get_on_ff_stage(0);
    rtexcoord = GeomVertexReader(data_reader, stage->get_texcoord_name());
    rtexcoord.set_row(_min_vertex);
    needs_texcoord = rtexcoord.has_column();
  }

  bool needs_color = false;
  if (_vertex_colors_enabled) {
    rcolor = GeomVertexReader(data_reader, InternalName::get_color());
    rcolor.set_row(_min_vertex);
    needs_color = rcolor.has_column();
  }

  if (!needs_color) {
    if (_has_scene_graph_color) {
      cerr << "sg color\n";
      const Colorf &d = _scene_graph_color;
      _c->current_color.X = d[0];
      _c->current_color.Y = d[1];
      _c->current_color.Z = d[2];
      _c->current_color.W = d[3];
      
    } else {
      _c->current_color.X = 1.0f;
      _c->current_color.Y = 1.0f;
      _c->current_color.Z = 1.0f;
      _c->current_color.W = 1.0f;
    }
  }

  bool needs_normal = false;
  if (_c->lighting_enabled) {
    rnormal = GeomVertexReader(data_reader, InternalName::get_normal());
    rnormal.set_row(_min_vertex);
    needs_normal = rnormal.has_column();
  }

  GeomVertexReader rvertex(data_reader, InternalName::get_vertex()); 
  rvertex.set_row(_min_vertex);

  for (i = 0; i < num_used_vertices; ++i) {
    GLVertex *v = &_vertices[i];
    const LVecBase4f &d = rvertex.get_data4f();
    
    v->coord.X = d[0];
    v->coord.Y = d[1];
    v->coord.Z = d[2];
    v->coord.W = d[3];

    if (needs_texcoord) {
      const LVecBase2f &d = rtexcoord.get_data2f();
      v->tex_coord.X = d[0];
      v->tex_coord.Y = d[1];
    }

    if (needs_color) {
      const Colorf &d = rcolor.get_data4f();
      _c->current_color.X = d[0];
      _c->current_color.Y = d[1];
      _c->current_color.Z = d[2];
      _c->current_color.W = d[3];

      if (_c->color_material_enabled) {
	GLParam q[7];
	q[0].op = OP_Material;
	q[1].i = _c->current_color_material_mode;
	q[2].i = _c->current_color_material_type;
	q[3].f = d[0];
	q[4].f = d[1];
	q[5].f = d[2];
	q[6].f = d[3];
	glopMaterial(_c, q);
      }
    }

    v->color = _c->current_color;

    if (needs_normal) {
      const LVecBase3f &d = rnormal.get_data3f();
      _c->current_normal.X = d[0];
      _c->current_normal.Y = d[1];
      _c->current_normal.Z = d[2];
      _c->current_normal.W = 0.0f;
    }

    tgl_vertex_transform(_c, v);

    if (_c->lighting_enabled) {
      gl_shade_vertex(_c, v);
    }

    if (v->clip_code == 0) {
      gl_transform_to_viewport(_c, v);
    }

    v->edge_flag = 0;
  }

  // Set up the appropriate function callback for filling triangles,
  // according to the current state.

  int alpha_test_state = 0;
  if (_target._transparency->get_mode() != TransparencyAttrib::M_none) {
    alpha_test_state = 1;
  }

  int depth_test_state = 1;
  if (_target._depth_test->get_mode() == DepthTestAttrib::M_none) {
    depth_test_state = 0;
  }
  
  ShadeModelAttrib::Mode shade_model = _target._shade_model->get_mode();
  if (!needs_normal && !needs_color) {
    // With no per-vertex lighting, and no per-vertex colors, we might
    // as well use the flat shading model.
    shade_model = ShadeModelAttrib::M_flat;
  }
  int color_state = 2;  // smooth
  if (shade_model == ShadeModelAttrib::M_flat) {
    color_state = 1;  // flat
    if (_c->current_color.X == 1.0f &&
        _c->current_color.Y == 1.0f &&
        _c->current_color.Z == 1.0f &&
        _c->current_color.W == 1.0f) {
      color_state = 0;  // white
    }
  }

  int texture_state = 0;  // untextured
  if (_c->texture_2d_enabled) {
    texture_state = 2;  // perspective-correct textures
    if (_c->matrix_model_projection_no_w_transform) {
      texture_state = 1;  // non-perspective-correct textures
    }
  }

  _c->zb_fill_tri = fill_tri_funcs[alpha_test_state][depth_test_state][color_state][texture_state];
  //_c->zb_fill_tri = ZB_fillTriangleFlat_zless;
  
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
    for (int i = 0; i < num_vertices; i += 3) {
      GLVertex *v0 = &_vertices[reader->get_vertex(i) - _min_vertex];
      GLVertex *v1 = &_vertices[reader->get_vertex(i + 1) - _min_vertex];
      GLVertex *v2 = &_vertices[reader->get_vertex(i + 2) - _min_vertex];
      gl_draw_triangle(_c, v0, v1, v2);
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
    for (int i = 0; i < num_vertices; i += 2) {
      GLVertex *v0 = &_vertices[reader->get_vertex(i) - _min_vertex];
      GLVertex *v1 = &_vertices[reader->get_vertex(i + 1) - _min_vertex];
      gl_draw_line(_c, v0, v1);
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
    for (int i = 0; i < num_vertices; ++i) {
      GLVertex *v0 = &_vertices[reader->get_vertex(i) - _min_vertex];
      gl_draw_point(_c, v0);
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
  gtc->_gltex = (GLTexture *)gl_zalloc(sizeof(GLTexture));

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

  if (_c->current_texture == gtc->_gltex) {
    _c->current_texture = NULL;
    _c->zb->current_texture = NULL;
    _c->texture_2d_enabled = false;
  }

  for (int i = 0; i < MAX_TEXTURE_LEVELS; i++) {
    GLImage *im = &gtc->_gltex->images[i];
    if (im->pixmap != NULL) {
      gl_free(im->pixmap);
    }
  }

  gl_free(gtc->_gltex);
  gtc->_gltex = NULL;

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

  /*
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(render_transform->get_mat().get_data());
  */
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

  /*
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  */
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
  _c->current_texture = gtc->_gltex;
  _c->texture_2d_enabled = true;

  if (gtc->was_image_modified()) {
    // If the texture image was modified, reload the texture.
    if (!upload_texture(gtc)) {
      _c->texture_2d_enabled = false;
    }
    gtc->mark_loaded();

  } else if (gtc->was_properties_modified()) {
    // If only the properties have been modified, we don't need to
    // reload the texture.
    gtc->mark_loaded();
  }

  _c->zb->current_texture = (PIXEL *)gtc->_gltex->images[0].pixmap;
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
#ifdef DO_PSTATS
  _data_transferred_pcollector.add_level(tex->get_ram_image_size());
#endif

  // Internal texture size is always 256 x 256 x 4.
  static const int iwidth = 256;
  static const int iheight = 256;
  static const int ibytecount = iwidth * iheight * 4;

  GLImage *im = &gtc->_gltex->images[0];
  im->xsize = iwidth;
  im->ysize = iheight;
  if (im->pixmap == NULL) {
    im->pixmap = gl_malloc(ibytecount);
  }

  switch (tex->get_format()) {
  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgb8:
  case Texture::F_rgb12:
  case Texture::F_rgb332:
    copy_rgb_image(im, tex);
    break;

  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgba4:
  case Texture::F_rgba5:
  case Texture::F_rgba8:
  case Texture::F_rgba12:
  case Texture::F_rgba16:
  case Texture::F_rgba32:
    copy_rgba_image(im, tex);
    break;

  case Texture::F_luminance:
    copy_lum_image(im, tex);
    break;

  case Texture::F_red:
    copy_one_channel_image(im, tex, 0);
    break;

  case Texture::F_green:
    copy_one_channel_image(im, tex, 1);
    break;

  case Texture::F_blue:
    copy_one_channel_image(im, tex, 2);
    break;

  case Texture::F_alpha:
    copy_alpha_image(im, tex);
    break;

  case Texture::F_luminance_alphamask:
  case Texture::F_luminance_alpha:
    copy_la_image(im, tex);
    break;
  }
  
#ifdef DO_PSTATS 
  gtc->update_data_size_bytes(ibytecount);
#endif
  
  tex->texture_uploaded();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_lum_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel luminance image
//               from the texture into the indicated GLImage pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_lum_image(GLImage *im, Texture *tex) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[co];
      dpix[2] = spix[co];
      dpix[3] = 0xff;
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_alpha_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel alpha image
//               from the texture into the indicated GLImage pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_alpha_image(GLImage *im, Texture *tex) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;
      
      dpix[0] = 0xff;
      dpix[1] = 0xff;
      dpix[2] = 0xff;
      dpix[3] = spix[co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_one_channel_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel image (with a
//               single channel, e.g. red, green, or blue) from
//               the texture into the indicated GLImage pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_one_channel_image(GLImage *im, Texture *tex, int channel) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;

      dpix[0] = 0;
      dpix[1] = 0;
      dpix[2] = 0;
      dpix[3] = 0xff;
      dpix[channel] = spix[co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_la_image
//       Access: Private, Static
//  Description: Copies and scales the two-channel luminance-alpha
//               image from the texture into the indicated GLImage
//               pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_la_image(GLImage *im, Texture *tex) {
  nassertv(tex->get_num_components() == 2);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 2 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[co];
      dpix[2] = spix[co];
      dpix[3] = spix[cw + co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgb_image
//       Access: Private, Static
//  Description: Copies and scales the three-channel RGB image from
//               the texture into the indicated GLImage pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgb_image(GLImage *im, Texture *tex) {
  nassertv(tex->get_num_components() == 3);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 3 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[cw + co];
      dpix[2] = spix[cw + cw + co];
      dpix[3] = 0xff;
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgba_image
//       Access: Private, Static
//  Description: Copies and scales the four-channel RGBA image from
//               the texture into the indicated GLImage pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgba_image(GLImage *im, Texture *tex) {
  nassertv(tex->get_num_components() == 4);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
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

  int xsize_dest = im->xsize;
  int ysize_dest = im->xsize;
  unsigned char *dest = (unsigned char *)im->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 4 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[cw + co];
      dpix[2] = spix[cw + cw + co];
      dpix[3] = spix[cw + cw + cw + co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
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
