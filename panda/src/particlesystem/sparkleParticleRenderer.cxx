// Filename: sparkleParticleRenderer.cxx
// Created by:  charles (27Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "sparkleParticleRenderer.h"

#include "boundingSphere.h"
#include "geomNode.h"

////////////////////////////////////////////////////////////////////
//    Function : SparkleParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
SparkleParticleRenderer::
SparkleParticleRenderer(void) :
  BaseParticleRenderer(PR_ALPHA_NONE),
  _center_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _edge_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _birth_radius(0.1f), _death_radius(0.1f)
{
  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SparkleParticleRenderer
//      Access : Public
// Description : Constructor
////////////////////////////////////////////////////////////////////
SparkleParticleRenderer::
SparkleParticleRenderer(const Colorf& center, const Colorf& edge,
                        float birth_radius, float death_radius,
                        SparkleParticleLifeScale life_scale,
                        ParticleRendererAlphaMode alpha_mode) :
  BaseParticleRenderer(alpha_mode),
  _center_color(center), _edge_color(edge), _birth_radius(birth_radius),
  _death_radius(death_radius), _life_scale(life_scale)
{
  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SparkleParticleRenderer
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
SparkleParticleRenderer::
SparkleParticleRenderer(const SparkleParticleRenderer& copy) :
  BaseParticleRenderer(copy) {
  _center_color = copy._center_color;
  _edge_color = copy._edge_color;
  _birth_radius = copy._birth_radius;
  _death_radius = copy._death_radius;
  _life_scale = copy._life_scale;

  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : ~SparkleParticleRenderer
//      Access : Public
// Description : Destructor
////////////////////////////////////////////////////////////////////
SparkleParticleRenderer::
~SparkleParticleRenderer(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make copy
//      Access : Public
// Description : child virtual for spawning systems
////////////////////////////////////////////////////////////////////
BaseParticleRenderer *SparkleParticleRenderer::
make_copy(void) {
  return new SparkleParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private, virtual
// Description : child birth
////////////////////////////////////////////////////////////////////
void SparkleParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private, virtual
// Description : child kill
////////////////////////////////////////////////////////////////////
void SparkleParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : private
// Description : resizes the render pool.  Reference counting
//               makes this easy.
////////////////////////////////////////////////////////////////////
void SparkleParticleRenderer::
resize_pool(int new_size) {
  _vertex_array = PTA_Vertexf::empty_array(new_size * 12);
  _color_array = PTA_Colorf::empty_array(new_size * 12);

  _line_primitive->set_coords(_vertex_array);
  _line_primitive->set_colors(_color_array, G_PER_VERTEX);

  _max_pool_size = new_size;

  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : private
// Description : initializes the geomnodes
////////////////////////////////////////////////////////////////////
void SparkleParticleRenderer::
init_geoms(void) {
  _line_primitive->set_num_prims(0);

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_line_primitive, _render_state);
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : private
// Description : populates the GeomLine
////////////////////////////////////////////////////////////////////
void SparkleParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {

  if (!ttl_particles)
    return;

  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  Vertexf *cur_vert = &_vertex_array[0];
  Colorf *cur_color = &_color_array[0];

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through the array

  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (cur_particle->get_alive() == false)
      continue;

    // adjust the aabb

    if (cur_particle->get_position().get_x() > _aabb_max.get_x())
      _aabb_max[0] = cur_particle->get_position().get_x();
    else if (cur_particle->get_position().get_x() < _aabb_min.get_x())
      _aabb_min[0] = cur_particle->get_position().get_x();

    if (cur_particle->get_position().get_y() > _aabb_max.get_y())
      _aabb_max[1] = cur_particle->get_position().get_y();
    else if (cur_particle->get_position().get_y() < _aabb_min.get_y())
      _aabb_min[1] = cur_particle->get_position().get_y();

    if (cur_particle->get_position().get_z() > _aabb_max.get_z())
      _aabb_max[2] = cur_particle->get_position().get_z();
    else if (cur_particle->get_position().get_z() < _aabb_min.get_z())
      _aabb_min[2] = cur_particle->get_position().get_z();

    // draw the particle.

    float radius = get_radius(cur_particle);
    float neg_radius = -radius;
    float alpha;

    LPoint3f pos = cur_particle->get_position();
    Colorf center_color = _center_color;
    Colorf edge_color = _edge_color;

    // handle alpha

    if (_alpha_mode != PR_ALPHA_NONE) {
      if(_alpha_mode == PR_ALPHA_USER) {
        alpha = get_user_alpha();
      } else {
        alpha = cur_particle->get_parameterized_age();
        if (_alpha_mode == PR_ALPHA_OUT)
          alpha = 1.0f - alpha;

        alpha *= get_user_alpha();
      }

      center_color[3] = alpha;
      edge_color[3] = alpha;
    }

    // 6 lines coming from the center point.

    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(radius, 0.0f, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(neg_radius, 0.0f, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, radius, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, neg_radius, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, 0.0f, radius);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, 0.0f, neg_radius);

    *cur_color++ = center_color;
    *cur_color++ = edge_color;
    *cur_color++ = center_color;
    *cur_color++ = edge_color;
    *cur_color++ = center_color;
    *cur_color++ = edge_color;
    *cur_color++ = center_color;
    *cur_color++ = edge_color;
    *cur_color++ = center_color;
    *cur_color++ = edge_color;
    *cur_color++ = center_color;
    *cur_color++ = edge_color;

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  _line_primitive->set_num_prims(6 * ttl_particles);

  // done filling geomline node, now do the bb stuff

  LPoint3f aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  float radius = (aabb_center - _aabb_min).length();

  _line_primitive->set_bound(BoundingSphere(aabb_center, radius));
  get_render_node()->mark_bound_stale();
}
