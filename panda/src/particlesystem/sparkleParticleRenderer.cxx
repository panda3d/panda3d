/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparkleParticleRenderer.cxx
 * @author charles
 * @date 2000-06-27
 */

#include "sparkleParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexWriter.h"
#include "indent.h"
#include "pStatTimer.h"

PStatCollector SparkleParticleRenderer::_render_collector("App:Particles:Sparkle:Render");

/**
 * Default Constructor
 */
SparkleParticleRenderer::
SparkleParticleRenderer() :
  BaseParticleRenderer(PR_ALPHA_NONE),
  _center_color(LColor(1.0f, 1.0f, 1.0f, 1.0f)),
  _edge_color(LColor(1.0f, 1.0f, 1.0f, 1.0f)),
  _birth_radius(0.1f), _death_radius(0.1f)
{
  resize_pool(0);
}

/**
 * Constructor
 */
SparkleParticleRenderer::
SparkleParticleRenderer(const LColor& center, const LColor& edge,
                        PN_stdfloat birth_radius, PN_stdfloat death_radius,
                        SparkleParticleLifeScale life_scale,
                        ParticleRendererAlphaMode alpha_mode) :
  BaseParticleRenderer(alpha_mode),
  _center_color(center), _edge_color(edge), _birth_radius(birth_radius),
  _death_radius(death_radius), _life_scale(life_scale)
{
  resize_pool(0);
}

/**
 * Copy Constructor
 */
SparkleParticleRenderer::
SparkleParticleRenderer(const SparkleParticleRenderer& copy) :
  BaseParticleRenderer(copy) {
  _center_color = copy._center_color;
  _edge_color = copy._edge_color;
  _birth_radius = copy._birth_radius;
  _death_radius = copy._death_radius;
  _life_scale = copy._life_scale;

  resize_pool(0);
}

/**
 * Destructor
 */
SparkleParticleRenderer::
~SparkleParticleRenderer() {
}

/**
 * child virtual for spawning systems
 */
BaseParticleRenderer *SparkleParticleRenderer::
make_copy() {
  return new SparkleParticleRenderer(*this);
}

/**
 * child birth
 */
void SparkleParticleRenderer::
birth_particle(int) {
}

/**
 * child kill
 */
void SparkleParticleRenderer::
kill_particle(int) {
}

/**
 * resizes the render pool.  Reference counting makes this easy.
 */
void SparkleParticleRenderer::
resize_pool(int new_size) {
  _max_pool_size = new_size;

  init_geoms();
}

/**
 * initializes the geomnodes
 */
void SparkleParticleRenderer::
init_geoms() {
  _vdata = new GeomVertexData
    ("sparkle_particles", GeomVertexFormat::get_v3cp(),
     Geom::UH_stream);
  PT(Geom) geom = new Geom(_vdata);
  _line_primitive = geom;
  _lines = new GeomLines(Geom::UH_stream);
  geom->add_primitive(_lines);

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_line_primitive, _render_state);
}

/**
 * populates the GeomLine
 */
void SparkleParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  PStatTimer t1(_render_collector);
  if (!ttl_particles) {
    return;
  }

  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  GeomVertexWriter vertex(_vdata, InternalName::get_vertex());
  GeomVertexWriter color(_vdata, InternalName::get_color());
  _lines->clear_vertices();

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through the array

  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (cur_particle->get_alive() == false) {
      continue;
    }

    LPoint3 position = cur_particle->get_position();

    // adjust the aabb

    if (position[0] > _aabb_max[0])
      _aabb_max[0] = position[0];
    else if (position[0] < _aabb_min[0])
      _aabb_min[0] = position[0];

    if (position[1] > _aabb_max[1])
      _aabb_max[1] = position[1];
    else if (position[1] < _aabb_min[1])
      _aabb_min[1] = position[1];

    if (position[2] > _aabb_max[2])
      _aabb_max[2] = position[2];
    else if (position[2] < _aabb_min[2])
      _aabb_min[2] = position[2];

    // draw the particle.

    PN_stdfloat radius = get_radius(cur_particle);
    PN_stdfloat neg_radius = -radius;
    PN_stdfloat alpha;

    LColor center_color = _center_color;
    LColor edge_color = _edge_color;

    // handle alpha

    if (_alpha_mode != PR_ALPHA_NONE) {
      if(_alpha_mode == PR_ALPHA_USER) {
        alpha = get_user_alpha();
      } else {
        alpha = cur_particle->get_parameterized_age();
        if (_alpha_mode == PR_ALPHA_OUT)
          alpha = 1.0f - alpha;
        else if (_alpha_mode == PR_ALPHA_IN_OUT)
          alpha = 2.0f * std::min(alpha, 1.0f - alpha);

        alpha *= get_user_alpha();
      }

      center_color[3] = alpha;
      edge_color[3] = alpha;
    }

    // 6 lines coming from the center point.

    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(radius, 0.0f, 0.0f));
    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(neg_radius, 0.0f, 0.0f));
    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(0.0f, radius, 0.0f));
    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(0.0f, neg_radius, 0.0f));
    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(0.0f, 0.0f, radius));
    vertex.add_data3(position);
    vertex.add_data3(position + LVertex(0.0f, 0.0f, neg_radius));

    color.add_data4(center_color);
    color.add_data4(edge_color);
    color.add_data4(center_color);
    color.add_data4(edge_color);
    color.add_data4(center_color);
    color.add_data4(edge_color);
    color.add_data4(center_color);
    color.add_data4(edge_color);
    color.add_data4(center_color);
    color.add_data4(edge_color);
    color.add_data4(center_color);
    color.add_data4(edge_color);

    _lines->add_next_vertices(2);
    _lines->close_primitive();
    _lines->add_next_vertices(2);
    _lines->close_primitive();
    _lines->add_next_vertices(2);
    _lines->close_primitive();
    _lines->add_next_vertices(2);
    _lines->close_primitive();
    _lines->add_next_vertices(2);
    _lines->close_primitive();
    _lines->add_next_vertices(2);
    _lines->close_primitive();

    remaining_particles--;
    if (remaining_particles == 0) {
      break;
    }
  }

  // done filling geomline node, now do the bb stuff

  LPoint3 aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  PN_stdfloat radius = (aabb_center - _aabb_min).length();

  BoundingSphere sphere(aabb_center, radius);
  _line_primitive->set_bounds(&sphere);
  get_render_node()->mark_internal_bounds_stale();
}

/**
 * Write a string representation of this instance to <out>.
 */
void SparkleParticleRenderer::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SparkleParticleRenderer";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void SparkleParticleRenderer::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "SparkleParticleRenderer:\n";
  indent(out, indent_level + 2) << "_center_color "<<_center_color<<"\n";
  indent(out, indent_level + 2) << "_edge_color "<<_edge_color<<"\n";
  indent(out, indent_level + 2) << "_birth_radius "<<_birth_radius<<"\n";
  indent(out, indent_level + 2) << "_death_radius "<<_death_radius<<"\n";
  indent(out, indent_level + 2) << "_line_primitive "<<_line_primitive<<"\n";
  indent(out, indent_level + 2) << "_max_pool_size "<<_max_pool_size<<"\n";
  indent(out, indent_level + 2) << "_life_scale "<<_life_scale<<"\n";
  indent(out, indent_level + 2) << "_aabb_min "<<_aabb_min<<"\n";
  indent(out, indent_level + 2) << "_aabb_max "<<_aabb_max<<"\n";
  BaseParticleRenderer::write(out, indent_level + 2);
}
