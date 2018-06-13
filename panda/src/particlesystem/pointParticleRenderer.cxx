/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticleRenderer.cxx
 * @author charles
 * @date 2000-06-20
 */

#include "pointParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexWriter.h"
#include "indent.h"
#include "pStatTimer.h"

PStatCollector PointParticleRenderer::_render_collector("App:Particles:Point:Render");

/**
 * special constructor
 */
PointParticleRenderer::
PointParticleRenderer(ParticleRendererAlphaMode am,
                      PN_stdfloat point_size,
                      PointParticleBlendType bt,
                      ParticleRendererBlendMethod bm,
                      const LColor& sc, const LColor& ec) :
  BaseParticleRenderer(am),
  _start_color(sc), _end_color(ec),
  _blend_type(bt), _blend_method(bm)
{
  set_point_size(point_size);
  resize_pool(0);
}

/**
 * Copy constructor
 */
PointParticleRenderer::
PointParticleRenderer(const PointParticleRenderer& copy) :
  BaseParticleRenderer(copy)
{
  _blend_type = copy._blend_type;
  _blend_method = copy._blend_method;
  _start_color = copy._start_color;
  _end_color = copy._end_color;
  _point_size = copy._point_size;
  _thick = copy._thick;
  resize_pool(0);
}

/**
 * Simple destructor
 */
PointParticleRenderer::
~PointParticleRenderer() {
}

/**
 * for spawning systems from dead particles
 */
BaseParticleRenderer *PointParticleRenderer::
make_copy() {
  return new PointParticleRenderer(*this);
}

/**
 * reallocate the space for the vertex and color pools
 */
void PointParticleRenderer::
resize_pool(int new_size) {
  if (new_size == _max_pool_size)
    return;

  _max_pool_size = new_size;

  init_geoms();
}

/**
 * On-construction initialization
 */
void PointParticleRenderer::
init_geoms() {
  _vdata = new GeomVertexData
    ("point_particles", GeomVertexFormat::get_v3cp(),
     Geom::UH_stream);
  PT(Geom) geom = new Geom(_vdata);
  _point_primitive = geom;
  _points = new GeomPoints(Geom::UH_stream);
  geom->add_primitive(_points);

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_point_primitive, _render_state->add_attrib(_thick));
}

/**
 * child birth
 */
void PointParticleRenderer::
birth_particle(int) {
}

/**
 * child kill
 */
void PointParticleRenderer::
kill_particle(int) {
}

/**
 * Generates the point color based on the render_type
 */
LColor PointParticleRenderer::
create_color(const BaseParticle *p) {
  LColor color;
  PN_stdfloat life_t, vel_t;
  PN_stdfloat parameterized_age = 1.0f;
  bool have_alpha_t = false;

  switch (_blend_type) {
  case PP_ONE_COLOR:
    // Constant solid color
    color = _start_color;
    break;

  case PP_BLEND_LIFE:
    // Blending colors based on life
    parameterized_age = p->get_parameterized_age();
    life_t = parameterized_age;
    have_alpha_t = true;

    if (_blend_method == PP_BLEND_CUBIC) {
      life_t = CUBIC_T(life_t);
    }

    color = LERP(life_t, _start_color, _end_color);
    break;

  case PP_BLEND_VEL:
    // Blending colors based on vel
    vel_t = p->get_parameterized_vel();

    if (_blend_method == PP_BLEND_CUBIC) {
      vel_t = CUBIC_T(vel_t);
    }

    color = LERP(vel_t, _start_color, _end_color);
    break;
  }

  // Handle alpha channel
  if (_alpha_mode != PR_ALPHA_NONE) {
    if (_alpha_mode == PR_ALPHA_USER) {
      parameterized_age = 1.0;
    } else {
      if (!have_alpha_t) {
        parameterized_age = p->get_parameterized_age();
      }

      if (_alpha_mode == PR_ALPHA_OUT) {
        parameterized_age = 1.0f - parameterized_age;
      } else if (_alpha_mode == PR_ALPHA_IN_OUT) {
        parameterized_age = 2.0f * std::min(parameterized_age,
                                      1.0f - parameterized_age);
      }
    }
    color[3] = parameterized_age * get_user_alpha();
  }

  return color;
}

/**
 * renders the particle system out to a GeomNode
 */
void PointParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  PStatTimer t1(_render_collector);

  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  GeomVertexWriter vertex(_vdata, InternalName::get_vertex());
  GeomVertexWriter color(_vdata, InternalName::get_color());

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through every filled slot

  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (!cur_particle->get_alive())
      continue;

    LPoint3 position = cur_particle->get_position();

    // x aabb adjust

    if (position[0] > _aabb_max[0])
      _aabb_max[0] = position[0];
    else if (position[0] < _aabb_min[0])
      _aabb_min[0] = position[0];

    // y aabb adjust

    if (position[1] > _aabb_max[1])
      _aabb_max[1] = position[1];
    else if (position[1] < _aabb_min[1])
      _aabb_min[1] = position[1];

    // z aabb adjust

    if (position[2] > _aabb_max[2])
      _aabb_max[2] = position[2];
    else if (position[2] < _aabb_min[2])
      _aabb_min[2] = position[2];

    // stuff it into the arrays

    vertex.add_data3(position);
    color.add_data4(create_color(cur_particle));

    // maybe jump out early?

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  _points->clear_vertices();
  _points->add_next_vertices(ttl_particles);

  // done filling geompoint node, now do the bb stuff

  LPoint3 aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  PN_stdfloat radius = (aabb_center - _aabb_min).length();

  BoundingSphere sphere(aabb_center, radius);
  _point_primitive->set_bounds(&sphere);
  get_render_node()->mark_internal_bounds_stale();
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticleRenderer::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointParticleRenderer";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticleRenderer::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "PointParticleRenderer:\n";
  indent(out, indent_level + 2) << "_start_color "<<_start_color<<"\n";
  indent(out, indent_level + 2) << "_end_color "<<_end_color<<"\n";
  indent(out, indent_level + 2) << "_point_size "<<_point_size<<"\n";
  indent(out, indent_level + 2) << "_point_primitive "<<_point_primitive<<"\n";
  indent(out, indent_level + 2) << "_max_pool_size "<<_max_pool_size<<"\n";
  indent(out, indent_level + 2) << "_blend_type "<<_blend_type<<"\n";
  indent(out, indent_level + 2) << "_blend_method "<<_blend_method<<"\n";
  indent(out, indent_level + 2) << "_aabb_min "<<_aabb_min<<"\n";
  indent(out, indent_level + 2) << "_aabb_max "<<_aabb_max<<"\n";
  BaseParticleRenderer::write(out, indent_level + 2);
}
