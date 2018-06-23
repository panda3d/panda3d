/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineParticleRenderer.cxx
 * @author darren
 * @date 2000-10-06
 */

#include "lineParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexWriter.h"
#include "indent.h"
#include "pStatTimer.h"

PStatCollector LineParticleRenderer::_render_collector("App:Particles:Line:Render");

/**
 * Default Constructor
 */

LineParticleRenderer::
LineParticleRenderer() :
  _head_color(LColor(1.0f, 1.0f, 1.0f, 1.0f)),
  _tail_color(LColor(1.0f, 1.0f, 1.0f, 1.0f)) {

  _line_scale_factor = 1.0f;

  resize_pool(0);
}

/**
 * Constructor
 */

LineParticleRenderer::
LineParticleRenderer(const LColor& head,
                     const LColor& tail,
                     ParticleRendererAlphaMode alpha_mode) :
  BaseParticleRenderer(alpha_mode),
  _head_color(head), _tail_color(tail)
{
  resize_pool(0);
}

/**
 * Copy Constructor
 */

LineParticleRenderer::
LineParticleRenderer(const LineParticleRenderer& copy) :
  BaseParticleRenderer(copy) {
  _head_color = copy._head_color;
  _tail_color = copy._tail_color;

  resize_pool(0);
}

/**
 * Destructor
 */

LineParticleRenderer::
~LineParticleRenderer() {
}

/**
 * child virtual for spawning systems
 */

BaseParticleRenderer *LineParticleRenderer::
make_copy() {
  return new LineParticleRenderer(*this);
}

/**
 * child birth
 */

void LineParticleRenderer::
birth_particle(int) {
}

/**
 * child kill
 */

void LineParticleRenderer::
kill_particle(int) {
}

/**
 * resizes the render pool.  Reference counting makes this easy.
 */

void LineParticleRenderer::
resize_pool(int new_size) {
  _max_pool_size = new_size;

  init_geoms();
}

/**
 * initializes the geomnodes
 */

void LineParticleRenderer::
init_geoms() {
  _vdata = new GeomVertexData
    ("line_particles", GeomVertexFormat::get_v3cp(),
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

void LineParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  PStatTimer t1(_render_collector);

  if (!ttl_particles)
    return;

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

    if (cur_particle->get_alive() == false)
      continue;

    LPoint3 position = cur_particle->get_position();

    // adjust the aabb

    if (position[0] > _aabb_max[0])
      _aabb_max[0] = position[0];
    if (position[0] < _aabb_min[0])
      _aabb_min[0] = position[0];

    if (position[1] > _aabb_max[1])
      _aabb_max[1] = position[1];
    if (position[1] < _aabb_min[1])
      _aabb_min[1] = position[1];

    if (position[2] > _aabb_max[2])
      _aabb_max[2] = position[2];
    if (position[2] < _aabb_min[2])
      _aabb_min[2] = position[2];

    // draw the particle.

    LColor head_color = _head_color;
    LColor tail_color = _tail_color;

    // handle alpha

    if (_alpha_mode != PR_ALPHA_NONE) {

          PN_stdfloat alpha;

      if (_alpha_mode == PR_ALPHA_USER) {
                  alpha = get_user_alpha();
          } else {
                  alpha = cur_particle->get_parameterized_age();
                  if (_alpha_mode == PR_ALPHA_OUT)
                          alpha = 1.0f - alpha;
                  else if (_alpha_mode == PR_ALPHA_IN_OUT)
                    alpha = 2.0f * std::min(alpha, 1.0f - alpha);
          }

      head_color[3] = alpha;
      tail_color[3] = alpha;
    }

    // one line from current position to last position

    vertex.add_data3(position);
    LPoint3 last_position = position +
      (cur_particle->get_last_position() - position) * _line_scale_factor;
    vertex.add_data3(last_position);
    color.add_data4(head_color);
    color.add_data4(tail_color);
    _lines->add_next_vertices(2);
    _lines->close_primitive();

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  // done filling geomline node, now do the bb stuff

  LPoint3 aabb_center = (_aabb_min + _aabb_max) * 0.5f;
  PN_stdfloat radius = (aabb_center - _aabb_min).length();

  BoundingSphere sphere(aabb_center, radius);
  _line_primitive->set_bounds(&sphere);
  get_render_node()->mark_internal_bounds_stale();
}

/**
 * Write a string representation of this instance to <out>.
 */
void LineParticleRenderer::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LineParticleRenderer";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LineParticleRenderer::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "LineParticleRenderer:\n";
  indent(out, indent_level + 2) << "_head_color "<<_head_color<<"\n";
  indent(out, indent_level + 2) << "_tail_color "<<_tail_color<<"\n";
  indent(out, indent_level + 2) << "_line_primitive "<<_line_primitive<<"\n";
  indent(out, indent_level + 2) << "_max_pool_size "<<_max_pool_size<<"\n";
  indent(out, indent_level + 2) << "_aabb_min "<<_aabb_min<<"\n";
  indent(out, indent_level + 2) << "_aabb_max "<<_aabb_max<<"\n";
  BaseParticleRenderer::write(out, indent_level + 2);
}
