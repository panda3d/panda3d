// Filename: cullableObject.cxx
// Created by:  drose (04Mar02)
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

#include "cullableObject.h"
#include "ambientLight.h"
#include "lightAttrib.h"
#include "nodePath.h"
#include "material.h"
#include "materialAttrib.h"
#include "texGenAttrib.h"
#include "renderState.h"
#include "clockObject.h"
#include "cullTraverser.h"
#include "sceneSetup.h"
#include "lens.h"
#include "stateMunger.h"
#include "pStatTimer.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"
#include "geomTriangles.h"
#include "light.h"

PStatCollector CullableObject::_munge_points_pcollector("*:Munge:Points");
PStatCollector CullableObject::_munge_light_vector_pcollector("*:Munge:Light Vector");

CullableObject *CullableObject::_deleted_chain = (CullableObject *)NULL;
int CullableObject::_num_ever_allocated = 0;
TypeHandle CullableObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::munge_geom
//       Access: Public
//  Description: Uses the indicated GeomMunger to transform the geom
//               and/or its vertices.
////////////////////////////////////////////////////////////////////
void CullableObject::
munge_geom(GraphicsStateGuardianBase *gsg,
           GeomMunger *munger, const CullTraverser *traverser) {
  if (_geom != (Geom *)NULL) {
    _munger = munger;
    _munged_data = _geom->get_vertex_data();

    int geom_rendering = _geom->get_geom_rendering();
    geom_rendering = _state->get_geom_rendering(geom_rendering);
    geom_rendering = _modelview_transform->get_geom_rendering(geom_rendering);
    
    GraphicsStateGuardianBase *gsg = traverser->get_gsg();
    int gsg_bits = gsg->get_supported_geom_rendering();
    if (!hardware_point_sprites) {
      // If support for hardware point sprites or perspective-scaled
      // points is disabled, we don't allow the GSG to tell us it
      // supports them.
      gsg_bits &= ~(Geom::GR_point_perspective | Geom::GR_point_sprite);
    }
    int unsupported_bits = geom_rendering & ~gsg_bits;

    if ((unsupported_bits & Geom::GR_point_bits) != 0) {
      // The GSG doesn't support rendering these fancy points
      // directly; we have to render them in software instead.
      // Munge them into quads.  This will replace the _geom and
      // _munged_data, and might also replace _state.
      if (pgraph_cat.is_spam()) {
        pgraph_cat.spam()
          << "munge_points_to_quads() for geometry with bits: " 
          << hex << geom_rendering << ", unsupported: "
          << (unsupported_bits & Geom::GR_point_bits) << dec << "\n";
      }
      munge_points_to_quads(traverser);
    }

    bool cpu_animated = false;

    if (unsupported_bits & Geom::GR_texcoord_light_vector) {
      // If we have to compute the light vector, we have to animate
      // the vertices in the CPU--and we have to do it before we call
      // munge_geom(), which might lose the tangent and binormal.
      CPT(GeomVertexData) animated_vertices = 
	_munged_data->animate_vertices();
      if (animated_vertices != _munged_data) {
	cpu_animated = true;
	_munged_data = animated_vertices;
      }
      munge_texcoord_light_vector(traverser);
    }

    // Now invoke the munger to ensure the resulting geometry is in
    // a GSG-friendly form.
    munger->munge_geom(_geom, _munged_data);
    
    StateMunger *state_munger;
    DCAST_INTO_V(state_munger, munger);
    _state = state_munger->munge_state(_state);

    if (!cpu_animated) {
      // If there is any animation left in the vertex data after it
      // has been munged--that is, we couldn't arrange to handle the
      // animation in hardware--then we have to calculate that
      // animation now.
      CPT(GeomVertexData) animated_vertices = 
	_munged_data->animate_vertices();
      if (animated_vertices != _munged_data) {
	cpu_animated = true;
	_munged_data = animated_vertices;
      }
    }

#ifndef NDEBUG
    if (show_vertex_animation) {
      bool hardware_animated = (_munged_data->get_format()->get_animation().get_animation_type() == Geom::AT_hardware);
      if (cpu_animated || hardware_animated) {
        // These vertices were animated, so flash them red or blue.
        static const double flash_rate = 1.0;  // 1 state change per second
        int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * flash_rate);
        if ((cycle & 1) == 0) {
          _state = cpu_animated ? get_flash_cpu_state() : get_flash_hardware_state();
        }
      }
    }
#endif
  }
  if (_next != (CullableObject *)NULL) {
    if (_next->_state != (RenderState *)NULL) {
      _next->munge_geom(gsg, gsg->get_geom_munger(_next->_state), traverser);
    } else {
      _next->munge_geom(gsg, munger, traverser);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Destructor
//       Access: Public
//  Description: Automatically deletes the whole chain of these things.
////////////////////////////////////////////////////////////////////
CullableObject::
~CullableObject() {
  if (_next != (CullableObject *)NULL) {
    delete _next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullableObject::
output(ostream &out) const {
  if (_geom != (Geom *)NULL) {
    out << *_geom;
  } else {
    out << "(null)";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CullableObject::munge_points_to_quads
//       Access: Private
//  Description: Converts a table of points to quads for rendering on
//               systems that don't support fancy points.
//
//               This may replace _geom, _munged_data, and _state.
////////////////////////////////////////////////////////////////////
void CullableObject::
munge_points_to_quads(const CullTraverser *traverser) {
  PStatTimer timer(_munge_points_pcollector);

  GraphicsStateGuardianBase *gsg = traverser->get_gsg();

  GeomVertexReader vertex(_munged_data, InternalName::get_vertex());
  GeomVertexReader normal(_munged_data, InternalName::get_normal());
  GeomVertexReader color(_munged_data, InternalName::get_color());
  GeomVertexReader texcoord(_munged_data, InternalName::get_texcoord());
  GeomVertexReader rotate(_munged_data, InternalName::get_rotate());
  GeomVertexReader size(_munged_data, InternalName::get_size());
  GeomVertexReader aspect_ratio(_munged_data, InternalName::get_aspect_ratio());

  bool has_normal = (normal.has_column());
  bool has_color = (color.has_column());
  bool has_texcoord = (texcoord.has_column());
  bool has_rotate = (rotate.has_column());
  bool has_size = (size.has_column());
  bool has_aspect_ratio = (aspect_ratio.has_column());

  bool sprite_texcoord = false;
  const TexGenAttrib *tex_gen = _state->get_tex_gen();
  if (tex_gen != (TexGenAttrib *)NULL) {
    if (tex_gen->get_mode(TextureStage::get_default()) == TexGenAttrib::M_point_sprite) {
      sprite_texcoord = true;

      // Turn off the TexGenAttrib, since we don't want it now.
      _state = _state->add_attrib(tex_gen->remove_stage(TextureStage::get_default()));
    }
  }

  PT(GeomVertexArrayFormat) new_array_format =
    new GeomVertexArrayFormat(InternalName::get_vertex(), 4, 
			      Geom::NT_float32,
			      Geom::C_clip_point);
  if (has_normal) {
    const GeomVertexColumn *c = normal.get_column();
    new_array_format->add_column
      (InternalName::get_normal(), c->get_num_components(),
       c->get_numeric_type(), c->get_contents());
  }
  if (has_color) {
    const GeomVertexColumn *c = color.get_column();
    new_array_format->add_column
      (InternalName::get_color(), c->get_num_components(),
       c->get_numeric_type(), c->get_contents());
  }
  if (sprite_texcoord) {
    new_array_format->add_column
      (InternalName::get_texcoord(), 2,
       Geom::NT_float32,
       Geom::C_texcoord);

  } else if (has_texcoord) {
    const GeomVertexColumn *c = texcoord.get_column();
    new_array_format->add_column
      (InternalName::get_texcoord(), c->get_num_components(),
       c->get_numeric_type(), c->get_contents());
  }

  CPT(GeomVertexFormat) new_format = 
    GeomVertexFormat::register_format(new_array_format);

  PT(GeomVertexData) new_data = new GeomVertexData
    (_munged_data->get_name(), new_format, Geom::UH_client);

  GeomVertexWriter new_vertex(new_data, InternalName::get_vertex());
  GeomVertexWriter new_normal(new_data, InternalName::get_normal());
  GeomVertexWriter new_color(new_data, InternalName::get_color());
  GeomVertexWriter new_texcoord(new_data, InternalName::get_texcoord());
  int new_vi = 0;

  PT(Geom) new_geom = new Geom(new_data);

  const LMatrix4f &modelview = _modelview_transform->get_mat();

  SceneSetup *scene = traverser->get_scene();
  const Lens *lens = scene->get_lens();
  const LMatrix4f &projection = lens->get_projection_mat();

  int viewport_width = scene->get_viewport_width();
  int viewport_height = scene->get_viewport_height();

  float point_size = 1.0f;
  bool perspective = false;
  const RenderModeAttrib *render_mode = _state->get_render_mode();
  if (render_mode != (RenderModeAttrib *)NULL) {
    point_size = render_mode->get_thickness();
    perspective = render_mode->get_perspective();

    if (render_mode->get_mode() != RenderModeAttrib::M_filled) {
      // Be sure to turn on polygon render mode, since we're actually
      // rendering polygons, not points any more.
      _state = _state->add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_filled));
    }
  }

  // We need a standard projection matrix, in a known coordinate
  // system, to compute the perspective height.
  LMatrix4f height_projection;
  if (perspective) {
    height_projection =
      LMatrix4f::convert_mat(CS_yup_right, lens->get_coordinate_system()) *
      projection;
  }

  // We will need the composite render transform to compute the
  // lighting normal.
  LMatrix4f render_transform;
  if (has_normal) {
    render_transform = modelview * projection;
  }

  // Replace each primitive in the Geom (it's presumably a GeomPoints
  // primitive, although it might be some other kind of primitive if
  // we got here because RenderModeAttrib::M_point is enabled) with a
  // new primitive that replaces each vertex with a quad of the
  // appropriate scale and orientation.

  // BUG: if we're rendering polygons in M_point mode with a
  // CullFaceAttrib in effect, we won't actually apply the
  // CullFaceAttrib but will always render all of the vertices of the
  // polygons.  This is certainly a bug, but in order to fix it we'd
  // have to do the face culling ourselves--not sure if it's worth it.
  int num_primitives = _geom->get_num_primitives();
  for (int pi = 0; pi < num_primitives; ++pi) {
    const GeomPrimitive *primitive = _geom->get_primitive(pi);

    // We must first convert all of the points to eye space.
    int num_points = primitive->get_max_vertex() + 1;
    int num_vertices = primitive->get_num_vertices();
    PointData *points = (PointData *)alloca(num_points * sizeof(PointData));
    unsigned int *vertices = (unsigned int *)alloca(num_vertices * sizeof(unsigned int));
    unsigned int *vertices_end = vertices + num_vertices;

    if (primitive->is_indexed()) {
      GeomVertexReader index(primitive->get_vertices(), 0);
      for (unsigned int *vi = vertices; vi != vertices_end; ++vi) {
        // Get the point in eye-space coordinates.
        unsigned int v = index.get_data1i();
        nassertv(v < (unsigned int)num_points);
        (*vi) = v;
        vertex.set_row(v);
        points[v]._eye = modelview.xform_point(vertex.get_data3f());
        points[v]._dist = gsg->compute_distance_to(points[v]._eye);
      }
    } else {
      // Nonindexed case.
      unsigned int first_vertex = primitive->get_first_vertex();
      for (int i = 0; i < num_vertices; ++i) {
        unsigned int v = i + first_vertex;
        nassertv(v < (unsigned int)num_points);
        vertices[i] = v;
        vertex.set_row(i + first_vertex);
        points[v]._eye = modelview.xform_point(vertex.get_data3f());
        points[v]._dist = gsg->compute_distance_to(points[v]._eye);
      }
    }
  
    // Now sort the points in order from back-to-front so they will
    // render properly with transparency, at least with each other.
    sort(vertices, vertices_end, SortPoints(points));
  
    // Go through the points, now in sorted order, and generate a pair
    // of triangles for each one.  We generate indexed triangles
    // instead of two-triangle strips, since this seems to be
    // generally faster on PC hardware (otherwise, we'd have to nearly
    // double the vertices to stitch all the little triangle strips
    // together).
    PT(GeomPrimitive) new_primitive = new GeomTriangles(Geom::UH_client);

    for (unsigned int *vi = vertices; vi != vertices_end; ++vi) {
      // The point in eye coordinates.
      const LPoint3f &eye = points[*vi]._eye;
    
      // The point in clip coordinates.
      LPoint4f p4 = LPoint4f(eye[0], eye[1], eye[2], 1.0f) * projection;

      if (has_size) {
        size.set_row(*vi);
        point_size = size.get_data1f();
      }

      float scale_y = point_size;
      if (perspective) {
        // Perspective-sized points.  Here point_size is the point's
        // height in 3-d units.  To arrange that, we need to figure
        // out the appropriate scaling factor based on the current
        // viewport and projection matrix.
        float scale = _modelview_transform->get_scale()[1];
        LVector3f height(0.0f, point_size * scale, scale);
        height = height * height_projection;
        scale_y = height[1] * viewport_height;

        // We should then divide the radius by the distance from the
        // camera plane, to emulate the glPointParameters() behavior.
        if (!lens->is_orthographic()) {
          scale_y /= gsg->compute_distance_to(eye);
        }
      }
      
      // Also factor in the homogeneous scale for being in clip
      // coordinates still.
      scale_y *= p4[3];

      float scale_x = scale_y;
      if (has_aspect_ratio) {
        aspect_ratio.set_row(*vi);
        scale_x *= aspect_ratio.get_data1f();
      }

      // Define the first two corners based on the scales in X and Y.
      LPoint2f c0(scale_x, scale_y);
      LPoint2f c1(-scale_x, scale_y);

      if (has_rotate) { 
       // If we have a rotate factor, apply it to those two corners.
        rotate.set_row(*vi);
        float r = rotate.get_data1f();
        LMatrix3f mat = LMatrix3f::rotate_mat(r);
        c0 = c0 * mat;
        c1 = c1 * mat;
      }

      // Finally, scale the corners in their newly-rotated position,
      // to compensate for the aspect ratio of the viewport.
      float rx = 1.0f / viewport_width;
      float ry = 1.0f / viewport_height;
      c0.set(c0[0] * rx, c0[1] * ry);
      c1.set(c1[0] * rx, c1[1] * ry);

      new_vertex.add_data4f(p4[0] + c0[0], p4[1] + c0[1], p4[2], p4[3]);
      new_vertex.add_data4f(p4[0] + c1[0], p4[1] + c1[1], p4[2], p4[3]);
      new_vertex.add_data4f(p4[0] - c1[0], p4[1] - c1[1], p4[2], p4[3]);
      new_vertex.add_data4f(p4[0] - c0[0], p4[1] - c0[1], p4[2], p4[3]);

      if (has_normal) {
        normal.set_row(*vi);
        Normalf c = render_transform.xform_vec(normal.get_data3f());
        new_normal.add_data3f(c);
        new_normal.add_data3f(c);
        new_normal.add_data3f(c);
        new_normal.add_data3f(c);
      }
      if (has_color) {
        color.set_row(*vi);
        const Colorf &c = color.get_data4f();
        new_color.add_data4f(c);
        new_color.add_data4f(c);
        new_color.add_data4f(c);
        new_color.add_data4f(c);
      }
      if (sprite_texcoord) {
        new_texcoord.add_data2f(1.0f, 0.0f);
        new_texcoord.add_data2f(0.0f, 0.0f);
        new_texcoord.add_data2f(1.0f, 1.0f);
        new_texcoord.add_data2f(0.0f, 1.0f);
      } else if (has_texcoord) {
        texcoord.set_row(*vi);
        const LVecBase4f &c = texcoord.get_data4f();
        new_texcoord.add_data4f(c);
        new_texcoord.add_data4f(c);
        new_texcoord.add_data4f(c);
        new_texcoord.add_data4f(c);
      }

      new_primitive->add_vertex(new_vi);
      new_primitive->add_vertex(new_vi + 1);
      new_primitive->add_vertex(new_vi + 2);
      new_primitive->close_primitive();

      new_primitive->add_vertex(new_vi + 2);
      new_primitive->add_vertex(new_vi + 1);
      new_primitive->add_vertex(new_vi + 3);
      new_primitive->close_primitive();

      new_vi += 4;
    }

    new_geom->add_primitive(new_primitive);
  }

  _geom = new_geom.p();
  _munged_data = new_data;
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::munge_texcoord_light_vector
//       Access: Private
//  Description: Generates the vector from each vertex to the
//               indicated light as a 3-d texture coordinate.
//
//               This may replace _geom, _munged_data, and _state.
////////////////////////////////////////////////////////////////////
void CullableObject::
munge_texcoord_light_vector(const CullTraverser *traverser) {
  PStatTimer timer(_munge_light_vector_pcollector);

  if (_modelview_transform->is_singular()) {
    // If we're under a singular transform, never mind.
    return;
  }

  CPT(TransformState) net_transform =
    traverser->get_camera_transform()->compose(_modelview_transform);

  if (!_munged_data->has_column(InternalName::get_vertex()) || 
      !_munged_data->has_column(InternalName::get_normal())) {
    // No vertex or normal; can't compute light vector.
    return;
  }

  CPT(TexGenAttrib) tex_gen = _state->get_tex_gen();
  nassertv(tex_gen != (TexGenAttrib *)NULL);

  const TexGenAttrib::LightVectors &light_vectors = tex_gen->get_light_vectors();
  TexGenAttrib::LightVectors::const_iterator lvi;
  for (lvi = light_vectors.begin();
       lvi != light_vectors.end();
       ++lvi) {
    TextureStage *stage = (*lvi);
    NodePath light = tex_gen->get_light(stage);
    if (light.is_empty()) {
      // If a particular light isn't specified in the TexGenAttrib,
      // use the most important light in the current state.
      CPT(RenderAttrib) attrib = _state->get_attrib(LightAttrib::get_class_type());
      if (attrib != (RenderAttrib *)NULL) {
	CPT(LightAttrib) la = DCAST(LightAttrib, attrib);
	light = la->get_most_important_light();
	/*
	if (!light.is_empty()) {
	  // Remove that light, now that we're accounting for it in
	  // the normal map.
	  _state->set_attrib(la->remove_on_light(light));
	}
	*/
      }
    }
    if (!light.is_empty()) {
      string source_name = tex_gen->get_source_name(stage);
      Light *light_obj = light.node()->as_light();
      nassertv(light_obj != (Light *)NULL);
      
      // Determine the names of the tangent and binormal columns
      // associated with the stage's texcoord name.
      CPT(InternalName) tangent_name = InternalName::get_tangent_name(source_name);
      CPT(InternalName) binormal_name = InternalName::get_binormal_name(source_name);
      
      CPT(InternalName) texcoord_name = stage->get_texcoord_name();
      
      if (_munged_data->has_column(tangent_name) &&
	  _munged_data->has_column(binormal_name)) {
	// Create a new column for the new texcoords.
	PT(GeomVertexData) new_data = _munged_data->replace_column
	  (texcoord_name, 3, Geom::NT_float32, Geom::C_texcoord);
	_munged_data = new_data;
	
	// Remove this TexGen stage from the state, since we're handling
	// it now.
	_state = _state->add_attrib(tex_gen->remove_stage(stage));
	
	// Get the transform from the light to the object.
	CPT(TransformState) light_transform =
	  net_transform->invert_compose(light.get_net_transform());
	const LMatrix4f &light_mat = light_transform->get_mat();
	
	GeomVertexWriter texcoord(new_data, texcoord_name);
	GeomVertexReader vertex(new_data, InternalName::get_vertex());
	GeomVertexReader tangent(new_data, tangent_name);
	GeomVertexReader binormal(new_data, binormal_name);
	GeomVertexReader normal(new_data, InternalName::get_normal());
	
	while (!vertex.is_at_end()) {
	  LPoint3f p = vertex.get_data3f();
	  LVector3f t = tangent.get_data3f();
	  LVector3f b = binormal.get_data3f();
	  LVector3f n = normal.get_data3f();
	  
	  LVector3f lv;
	  if (light_obj->get_vector_to_light(lv, p, light_mat)) {
	    texcoord.add_data3f(lv.dot(t), lv.dot(b), lv.dot(n));
	  }
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::get_flash_cpu_state
//       Access: Private, Static
//  Description: Returns a RenderState for flashing the object red, to
//               show it is animated by the CPU when
//               show-vertex-animation is on.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullableObject::
get_flash_cpu_state() {
  static const Colorf flash_cpu_color(0.8f, 0.2f, 0.2f, 1.0f);

  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) flash_cpu_state = (const RenderState *)NULL;
  if (flash_cpu_state == (const RenderState *)NULL) {
    PT(AmbientLight) ambient_light = new AmbientLight("alight");
    ambient_light->set_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
    NodePath alight(ambient_light);

    CPT(LightAttrib) light_attrib = DCAST(LightAttrib, LightAttrib::make_all_off());
    light_attrib = DCAST(LightAttrib, light_attrib->add_on_light(alight));

    PT(Material) material = new Material;
    material->set_ambient(flash_cpu_color);
    material->set_diffuse(flash_cpu_color);

    flash_cpu_state = RenderState::make
      (light_attrib, 
       MaterialAttrib::make(material));
  }

  return flash_cpu_state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::get_flash_hardware_state
//       Access: Private, Static
//  Description: Returns a RenderState for flashing the object blue,
//               to show it is animated by the hardware when
//               show-vertex-animation is on.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullableObject::
get_flash_hardware_state() {
  static const Colorf flash_hardware_color(0.2f, 0.2f, 0.8f, 1.0f);

  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) flash_hardware_state = (const RenderState *)NULL;
  if (flash_hardware_state == (const RenderState *)NULL) {
    PT(AmbientLight) ambient_light = new AmbientLight("alight");
    ambient_light->set_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
    NodePath alight(ambient_light);

    CPT(LightAttrib) light_attrib = DCAST(LightAttrib, LightAttrib::make_all_off());
    light_attrib = DCAST(LightAttrib, light_attrib->add_on_light(alight));

    PT(Material) material = new Material;
    material->set_ambient(flash_hardware_color);
    material->set_diffuse(flash_hardware_color);

    flash_hardware_state = RenderState::make
      (light_attrib, 
       MaterialAttrib::make(material));
  }

  return flash_hardware_state;
}
