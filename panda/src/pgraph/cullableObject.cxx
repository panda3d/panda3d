/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullableObject.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "cullableObject.h"
#include "lightAttrib.h"
#include "nodePath.h"
#include "colorAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "shaderAttrib.h"
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
#include "lightMutexHolder.h"

CullableObject::FormatMap CullableObject::_format_map;
LightMutex CullableObject::_format_lock;

PStatCollector CullableObject::_munge_pcollector("*:Munge");
PStatCollector CullableObject::_munge_geom_pcollector("*:Munge:Geom");
PStatCollector CullableObject::_munge_sprites_pcollector("*:Munge:Sprites");
PStatCollector CullableObject::_munge_sprites_verts_pcollector("*:Munge:Sprites:Verts");
PStatCollector CullableObject::_munge_sprites_prims_pcollector("*:Munge:Sprites:Prims");
PStatCollector CullableObject::_sw_sprites_pcollector("SW Sprites");

TypeHandle CullableObject::_type_handle;

/**
 * Uses the indicated GeomMunger to transform the geom and/or its vertices.
 *
 * If force is false, this may do nothing and return false if the vertex data
 * is nonresident.  If force is true, this will always return true, but it may
 * have to block while the vertex data is paged in.
 */
bool CullableObject::
munge_geom(GraphicsStateGuardianBase *gsg, GeomMunger *munger,
           const CullTraverser *traverser, bool force) {
  nassertr(munger != nullptr, false);

  Thread *current_thread = traverser->get_current_thread();
  PStatTimer timer(_munge_pcollector, current_thread);
  if (_geom != nullptr) {
    GraphicsStateGuardianBase *gsg = traverser->get_gsg();
    int gsg_bits = gsg->get_supported_geom_rendering();
    if (!hardware_point_sprites) {
      // If support for hardware point sprites or perspective-scaled points is
      // disabled, we don't allow the GSG to tell us it supports them.
      gsg_bits &= ~(Geom::GR_point_perspective | Geom::GR_point_sprite);
    }
    if (!hardware_points) {
      // If hardware-points is off, we don't allow any kind of point
      // rendering, except plain old one-pixel points;
      gsg_bits &= ~(Geom::GR_point_bits & ~Geom::GR_point);
    }
    int geom_rendering;
    int unsupported_bits;

    {
      GeomPipelineReader geom_reader(_geom, current_thread);
      _munged_data = geom_reader.get_vertex_data();

#ifdef _DEBUG
      {
        GeomVertexDataPipelineReader data_reader(_munged_data, current_thread);
        data_reader.check_array_readers();
        nassertr(geom_reader.check_valid(&data_reader), false);
      }
#endif  // _DEBUG

      geom_rendering = geom_reader.get_geom_rendering();
      geom_rendering = _state->get_geom_rendering(geom_rendering);
      geom_rendering = _internal_transform->get_geom_rendering(geom_rendering);
      unsupported_bits = geom_rendering & ~gsg_bits;

      if (unsupported_bits & Geom::GR_per_point_size) {
        // If we have a shader that processes the point size, we can assume it
        // does the right thing.
        const ShaderAttrib *sattr;
        if (_state->get_attrib(sattr) && sattr->get_flag(ShaderAttrib::F_shader_point_size)) {
          unsupported_bits &= ~Geom::GR_per_point_size;
        }
      }

      if (geom_rendering & Geom::GR_point_bits) {
        if (geom_reader.get_primitive_type() != Geom::PT_points) {
          if (singular_points ||
              (unsupported_bits & Geom::GR_render_mode_point) != 0) {
            // Isolate the points so there's no unneeded overlap.
            _geom = _geom->make_points();
          }
        }
      }
      if (unsupported_bits & Geom::GR_render_mode_wireframe) {
        if (geom_reader.get_primitive_type() != Geom::PT_lines) {
          _geom = _geom->make_lines();
        }
      }
    }

    if ((unsupported_bits & Geom::GR_point_bits) != 0) {
      // The GSG doesn't support rendering these fancy points directly; we
      // have to render them in software instead.  Munge them into quads.
      // This will replace the _geom and _munged_data, and might also replace
      // _state.
      if (pgraph_cat.is_spam()) {
        pgraph_cat.spam()
          << "munge_points_to_quads() for geometry with bits: "
          << std::hex << geom_rendering << ", unsupported: "
          << (unsupported_bits & Geom::GR_point_bits) << std::dec << "\n";
      }
      if (!munge_points_to_quads(traverser, force)) {
        return false;
      }
    }

    // Now invoke the munger to ensure the resulting geometry is in a GSG-
    // friendly form.
    {
      PStatTimer timer(_munge_geom_pcollector, current_thread);
      if (!munger->munge_geom(_geom, _munged_data, force, current_thread)) {
        return false;
      }
    }

    // If we have prepared it for skinning via the shader generator, mark a
    // flag on the state so that the shader generator will do this.  We should
    // probably find a cleaner way to do this.
    const ShaderAttrib *sattr;
    if (_state->get_attrib(sattr) && sattr->auto_shader()) {
      GeomVertexDataPipelineReader data_reader(_munged_data, current_thread);
      if (data_reader.get_format()->get_animation().get_animation_type() == Geom::AT_hardware) {
        static CPT(RenderState) state = RenderState::make(
          DCAST(ShaderAttrib, ShaderAttrib::make())->set_flag(ShaderAttrib::F_hardware_skinning, true));
        _state = _state->compose(state);
      }

      gsg->ensure_generated_shader(_state);
    } else {
      // We may need to munge the state for the fixed-function pipeline.
      StateMunger *state_munger = (StateMunger *)munger;
      if (state_munger->should_munge_state()) {
        _state = state_munger->munge_state(_state);
      }
    }

    // If there is any animation left in the vertex data after it has been
    // munged--that is, we couldn't arrange to handle the animation in
    // hardware--then we have to calculate that animation now.
    bool cpu_animated = false;

    CPT(GeomVertexData) animated_vertices =
      _munged_data->animate_vertices(force, current_thread);
    if (animated_vertices != _munged_data) {
      cpu_animated = true;
      std::swap(_munged_data, animated_vertices);
    }

#ifndef NDEBUG
    if (show_vertex_animation) {
      GeomVertexDataPipelineReader data_reader(_munged_data, current_thread);
      bool hardware_animated = (data_reader.get_format()->get_animation().get_animation_type() == Geom::AT_hardware);
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

  return true;
}

/**
 *
 */
void CullableObject::
output(std::ostream &out) const {
  if (_geom != nullptr) {
    out << *_geom;
  } else {
    out << "(null)";
  }
}

/**
 * Converts a table of points to quads for rendering on systems that don't
 * support fancy points.
 *
 * This may replace _geom, _munged_data, and _state.
 */
bool CullableObject::
munge_points_to_quads(const CullTraverser *traverser, bool force) {
  Thread *current_thread = traverser->get_current_thread();

  // Better get the animated vertices, in case we're showing sprites on an
  // animated model for some reason.
  CPT(GeomVertexData) source_data =
    _munged_data->animate_vertices(force, current_thread);

  if (!force && !source_data->request_resident()) {
    return false;
  }

  PStatTimer timer(_munge_sprites_pcollector, current_thread);
  _sw_sprites_pcollector.add_level(source_data->get_num_rows());

  GraphicsStateGuardianBase *gsg = traverser->get_gsg();

  GeomVertexReader vertex(source_data, InternalName::get_vertex(),
                          current_thread);
  GeomVertexReader normal(source_data, InternalName::get_normal(),
                          current_thread);
  GeomVertexReader color(source_data, InternalName::get_color(),
                         current_thread);
  GeomVertexReader texcoord(source_data, InternalName::get_texcoord(),
                            current_thread);
  GeomVertexReader rotate(source_data, InternalName::get_rotate(),
                          current_thread);
  GeomVertexReader size(source_data, InternalName::get_size(),
                        current_thread);
  GeomVertexReader aspect_ratio(source_data, InternalName::get_aspect_ratio(),
                                current_thread);

  bool has_normal = (normal.has_column());
  bool has_color = (color.has_column());
  bool has_texcoord = (texcoord.has_column());
  bool has_rotate = (rotate.has_column());
  bool has_size = (size.has_column());
  bool has_aspect_ratio = (aspect_ratio.has_column());

  bool sprite_texcoord = false;
  const TexGenAttrib *tex_gen = DCAST(TexGenAttrib, _state->get_attrib(TexGenAttrib::get_class_slot()));
  if (tex_gen != nullptr) {
    if (tex_gen->get_mode(TextureStage::get_default()) == TexGenAttrib::M_point_sprite) {
      sprite_texcoord = true;

      // Turn off the TexGenAttrib, since we don't want it now.
      _state = _state->set_attrib(tex_gen->remove_stage(TextureStage::get_default()));
    }
  }

  PN_stdfloat point_size = 1;
  bool perspective = false;
  const RenderModeAttrib *render_mode = DCAST(RenderModeAttrib, _state->get_attrib(RenderModeAttrib::get_class_slot()));
  if (render_mode != nullptr) {
    point_size = render_mode->get_thickness();
    perspective = render_mode->get_perspective();

    if (render_mode->get_mode() != RenderModeAttrib::M_filled_flat) {
      // Render the new polygons with M_filled_flat, for a slight performance
      // advantage when software rendering.
      _state = _state->set_attrib(RenderModeAttrib::make(RenderModeAttrib::M_filled_flat));
    }
  }

  // Get the vertex format of the newly created geometry.
  CPT(GeomVertexFormat) new_format;

  {
    LightMutexHolder holder(_format_lock);
    SourceFormat sformat(source_data->get_format(), sprite_texcoord);
    FormatMap::iterator fmi = _format_map.find(sformat);
    if (fmi != _format_map.end()) {
      new_format = (*fmi).second;

    } else {
      // We have to construct the format now.
      PT(GeomVertexArrayFormat) new_array_format;
      if (sformat._retransform_sprites) {
        // With retransform_sprites in effect, we will be sending ordinary 3-D
        // points to the graphics API.
        new_array_format =
          new GeomVertexArrayFormat(InternalName::get_vertex(), 3,
                                    Geom::NT_stdfloat,
                                    Geom::C_point);
      } else {
        // Without retransform_sprites, we will be sending 4-component clip-
        // space points.
        new_array_format =
          new GeomVertexArrayFormat(InternalName::get_vertex(), 4,
                                    Geom::NT_stdfloat,
                                    Geom::C_clip_point);
      }
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
           Geom::NT_stdfloat,
           Geom::C_texcoord);

      } else if (has_texcoord) {
        const GeomVertexColumn *c = texcoord.get_column();
        new_array_format->add_column
          (InternalName::get_texcoord(), c->get_num_components(),
           c->get_numeric_type(), c->get_contents());
      }

      new_format = GeomVertexFormat::register_format(new_array_format);
      _format_map[sformat] = new_format;
    }
  }

  CoordinateSystem internal_cs = gsg->get_internal_coordinate_system();
  LMatrix4 internal = _internal_transform->get_mat();
  PN_stdfloat scale = _internal_transform->get_scale()[1];

  SceneSetup *scene = traverser->get_scene();
  const Lens *lens = scene->get_lens();
  LMatrix4 projection =
    LMatrix4::convert_mat(internal_cs, lens->get_coordinate_system()) *
                                       lens->get_projection_mat();

  int viewport_width = scene->get_viewport_width();
  int viewport_height = scene->get_viewport_height();

  // We need a standard projection matrix, in a known coordinate system, to
  // compute the perspective height.
  LMatrix4 height_projection;
  if (perspective) {
    height_projection =
      LMatrix4::convert_mat(CS_yup_right, lens->get_coordinate_system()) *
                                          lens->get_projection_mat();
  }

  LMatrix4 render_transform = internal * projection;
  LMatrix4 inv_render_transform;
  inv_render_transform.invert_from(render_transform);

  // Now convert all of the vertices in the GeomVertexData to quads.  We
  // always convert all the vertices, assuming all the vertices are referenced
  // by GeomPrimitives, because we want to optimize for the most common case.
  int orig_verts = source_data->get_num_rows();
  int new_verts = 4 * orig_verts;        // each vertex becomes four.

  PT(GeomVertexData) new_data = new GeomVertexData
    (source_data->get_name(), new_format, Geom::UH_stream);
  new_data->unclean_set_num_rows(new_verts);

  GeomVertexWriter new_vertex(new_data, InternalName::get_vertex());
  GeomVertexWriter new_normal(new_data, InternalName::get_normal());
  GeomVertexWriter new_color(new_data, InternalName::get_color());
  GeomVertexWriter new_texcoord(new_data, InternalName::get_texcoord());

  // We'll keep an array of all of the points' eye-space coordinates, and
  // their distance from the camera, so we can sort the points for each
  // primitive, below.
  PointData *points;
  {
    PStatTimer t2(_munge_sprites_verts_pcollector, current_thread);
    points = (PointData *)alloca(orig_verts * sizeof(PointData));
    int vi = 0;
    while (!vertex.is_at_end()) {
      // Get the point in eye-space coordinates.
      LPoint3 eye = internal.xform_point(vertex.get_data3());
      PN_stdfloat dist = gsg->compute_distance_to(eye);
      points[vi]._dist = dist;

      // The point in clip coordinates.
      LPoint4 p4 = LPoint4(eye[0], eye[1], eye[2], 1.0f) * projection;

      if (has_size) {
        point_size = size.get_data1();
      }

      PN_stdfloat scale_y = point_size;
      if (perspective) {
        // Perspective-sized points.  Here point_size is the point's height in
        // 3-d units.  To arrange that, we need to figure out the appropriate
        // scaling factor based on the current viewport and projection matrix.
        LVector3 height(0.0f, point_size * scale, scale);
        height = height * height_projection;
        scale_y = height[1] * viewport_height;

        // We should then divide the radius by the distance from the camera
        // plane, to emulate the glPointParameters() behavior.
        if (!lens->is_orthographic()) {
          scale_y /= dist;
        }
      }

      // Also factor in the homogeneous scale for being in clip coordinates
      // still.
      scale_y *= p4[3];

      PN_stdfloat scale_x = scale_y;
      if (has_aspect_ratio) {
        scale_x *= aspect_ratio.get_data1();
      }

      // Define the first two corners based on the scales in X and Y.
      LPoint2 c0(scale_x, scale_y);
      LPoint2 c1(-scale_x, scale_y);

      if (has_rotate) {
        // If we have a rotate factor, apply it to those two corners.
        PN_stdfloat r = rotate.get_data1();
        LMatrix3 mat = LMatrix3::rotate_mat(r);
        c0 = c0 * mat;
        c1 = c1 * mat;
      }

      // Finally, scale the corners in their newly-rotated position, to
      // compensate for the aspect ratio of the viewport.
      PN_stdfloat rx = 1.0f / viewport_width;
      PN_stdfloat ry = 1.0f / viewport_height;
      c0.set(c0[0] * rx, c0[1] * ry);
      c1.set(c1[0] * rx, c1[1] * ry);

      if (retransform_sprites) {
        // With retransform_sprites in effect, we must reconvert the resulting
        // quad back into the original 3-D space.
        new_vertex.set_data4(inv_render_transform.xform(LPoint4(p4[0] + c0[0], p4[1] + c0[1], p4[2], p4[3])));
        new_vertex.set_data4(inv_render_transform.xform(LPoint4(p4[0] + c1[0], p4[1] + c1[1], p4[2], p4[3])));
        new_vertex.set_data4(inv_render_transform.xform(LPoint4(p4[0] - c1[0], p4[1] - c1[1], p4[2], p4[3])));
        new_vertex.set_data4(inv_render_transform.xform(LPoint4(p4[0] - c0[0], p4[1] - c0[1], p4[2], p4[3])));

        if (has_normal) {
          const LNormal &c = normal.get_data3();
          new_normal.set_data3(c);
          new_normal.set_data3(c);
          new_normal.set_data3(c);
          new_normal.set_data3(c);
        }

      } else {
        // Without retransform_sprites, we can simply load the clip-space
        // coordinates.
        new_vertex.set_data4(p4[0] + c0[0], p4[1] + c0[1], p4[2], p4[3]);
        new_vertex.set_data4(p4[0] + c1[0], p4[1] + c1[1], p4[2], p4[3]);
        new_vertex.set_data4(p4[0] - c1[0], p4[1] - c1[1], p4[2], p4[3]);
        new_vertex.set_data4(p4[0] - c0[0], p4[1] - c0[1], p4[2], p4[3]);

        if (has_normal) {
          LNormal c = render_transform.xform_vec(normal.get_data3());
          new_normal.set_data3(c);
          new_normal.set_data3(c);
          new_normal.set_data3(c);
          new_normal.set_data3(c);
        }
      }
      if (has_color) {
        const LColor &c = color.get_data4();
        new_color.set_data4(c);
        new_color.set_data4(c);
        new_color.set_data4(c);
        new_color.set_data4(c);
      }
      if (sprite_texcoord) {
        new_texcoord.set_data2(1.0f, 0.0f);
        new_texcoord.set_data2(0.0f, 0.0f);
        new_texcoord.set_data2(1.0f, 1.0f);
        new_texcoord.set_data2(0.0f, 1.0f);
      } else if (has_texcoord) {
        const LVecBase4 &c = texcoord.get_data4();
        new_texcoord.set_data4(c);
        new_texcoord.set_data4(c);
        new_texcoord.set_data4(c);
        new_texcoord.set_data4(c);
      }

      ++vi;
    }

    nassertr(vi == orig_verts, false);
    nassertr(new_data->get_num_rows() == new_verts, false);
  }

  // Determine the format we should use to store the indices.  Don't choose
  // NT_uint8, as Direct3D 9 doesn't support it.
  const GeomVertexArrayFormat *new_prim_format = nullptr;
  if (new_verts < 0xffff) {
    new_prim_format = GeomPrimitive::get_index_format(GeomEnums::NT_uint16);

  } else {
    new_prim_format = GeomPrimitive::get_index_format(GeomEnums::NT_uint32);
  }

  PT(Geom) new_geom = new Geom(new_data);

  // Replace each primitive in the Geom (it's presumably a GeomPoints
  // primitive, although it might be some other kind of primitive if we got
  // here because RenderModeAttrib::M_point is enabled) with a new primitive
  // that replaces each vertex with a quad of the appropriate scale and
  // orientation.

  // BUG: if we're rendering polygons in M_point mode with a CullFaceAttrib in
  // effect, we won't actually apply the CullFaceAttrib but will always render
  // all of the vertices of the polygons.  This is certainly a bug, but a very
  // minor one; and in order to fix it we'd have to do the face culling
  // ourselves--not sure if it's worth it.

  {
    PStatTimer t3(_munge_sprites_prims_pcollector, current_thread);
    GeomPipelineReader geom_reader(_geom, current_thread);
    int num_primitives = geom_reader.get_num_primitives();
    for (int pi = 0; pi < num_primitives; ++pi) {
      const GeomPrimitive *primitive = geom_reader.get_primitive(pi);
      if (primitive->get_num_vertices() != 0) {
        // Extract out the list of vertices referenced by the primitive.
        int num_vertices = primitive->get_num_vertices();
        unsigned int *vertices = (unsigned int *)alloca(num_vertices * sizeof(unsigned int));
        unsigned int *vertices_end = vertices + num_vertices;

        if (primitive->is_indexed()) {
          // Indexed case.
          GeomVertexReader index(primitive->get_vertices(), 0, current_thread);
          for (unsigned int *vi = vertices; vi != vertices_end; ++vi) {
            unsigned int v = index.get_data1i();
            nassertr(v < (unsigned int)orig_verts, false);
            (*vi) = v;
          }
        } else {
          // Nonindexed case.
          unsigned int first_vertex = primitive->get_first_vertex();
          for (int i = 0; i < num_vertices; ++i) {
            unsigned int v = i + first_vertex;
            nassertr(v < (unsigned int)orig_verts, false);
            vertices[i] = v;
          }
        }

        // Now sort the points in order from back-to-front so they will render
        // properly with transparency, at least with each other.
        std::sort(vertices, vertices_end, SortPoints(points));

        // Go through the points, now in sorted order, and generate a pair of
        // triangles for each one.  We generate indexed triangles instead of
        // two-triangle strips, since this seems to be generally faster on PC
        // hardware (otherwise, we'd have to nearly double the vertices to
        // stitch all the little triangle strips together).
        PT(GeomPrimitive) new_primitive = new GeomTriangles(Geom::UH_stream);
        int new_prim_verts = 6 * num_vertices;  // two triangles per point.

        PT(GeomVertexArrayData) new_index
          = new GeomVertexArrayData(new_prim_format, GeomEnums::UH_stream);
        new_index->unclean_set_num_rows(new_prim_verts);

        GeomVertexWriter index(new_index, 0);
        nassertr(index.has_column(), false);
        for (unsigned int *vi = vertices; vi != vertices_end; ++vi) {
          int new_vi = (*vi) * 4;
          nassertr(index.get_write_row() + 6 <= new_prim_verts, false);
          index.set_data1i(new_vi);
          index.set_data1i(new_vi + 1);
          index.set_data1i(new_vi + 2);
          index.set_data1i(new_vi + 2);
          index.set_data1i(new_vi + 1);
          index.set_data1i(new_vi + 3);
        }
        new_primitive->set_vertices(new_index, new_prim_verts);

        int min_vi = primitive->get_min_vertex();
        int max_vi = primitive->get_max_vertex();
        new_primitive->set_minmax(min_vi * 4, max_vi * 4 + 3, nullptr, nullptr);

        new_geom->add_primitive(new_primitive);
      }
    }
  }

  _geom = new_geom.p();
  _munged_data = std::move(new_data);

  return true;
}

/**
 * Returns a RenderState for flashing the object red, to show it is animated
 * by the CPU when show-vertex-animation is on.
 */
CPT(RenderState) CullableObject::
get_flash_cpu_state() {
  static const LColor flash_cpu_color(0.8f, 0.2, 0.2, 1.0f);

  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) flash_cpu_state = nullptr;
  if (flash_cpu_state == nullptr) {
    flash_cpu_state = RenderState::make
      (LightAttrib::make_all_off(),
       TextureAttrib::make_off(),
       ColorAttrib::make_flat(flash_cpu_color));
  }

  return flash_cpu_state;
}

/**
 * Returns a RenderState for flashing the object blue, to show it is animated
 * by the hardware when show-vertex-animation is on.
 */
CPT(RenderState) CullableObject::
get_flash_hardware_state() {
  static const LColor flash_hardware_color(0.2, 0.2, 0.8, 1.0);

  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) flash_hardware_state = nullptr;
  if (flash_hardware_state == nullptr) {
    flash_hardware_state = RenderState::make
      (LightAttrib::make_all_off(),
       TextureAttrib::make_off(),
       ColorAttrib::make_flat(flash_hardware_color));
  }

  return flash_hardware_state;
}

/**
 *
 */
CullableObject::SourceFormat::
SourceFormat(const GeomVertexFormat *format, bool sprite_texcoord) :
  _format(format),
  _sprite_texcoord(sprite_texcoord)
{
  _retransform_sprites = retransform_sprites;
}
