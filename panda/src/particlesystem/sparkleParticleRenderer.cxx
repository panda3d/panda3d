// Filename: sparkleParticleRenderer.cxx
// Created by:  charles (27Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "sparkleParticleRenderer.h"

#include <boundingSphere.h>

////////////////////////////////////////////////////////////////////
//    Function : SparkleParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////

SparkleParticleRenderer::
SparkleParticleRenderer(void) :
  _center_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _edge_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _birth_mag(0.1f), _death_mag(0.1f),
  BaseParticleRenderer(PR_NO_ALPHA) {
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
			float birth_mag, float death_mag,
			SparkleParticleLifeScale life_scale,
			ParticleRendererAlphaDecay alpha_decay) :
  _center_color(center), _edge_color(edge), _birth_mag(birth_mag),
  _death_mag(death_mag), _life_scale(life_scale), 
  BaseParticleRenderer(alpha_decay) {
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
  _birth_mag = copy._birth_mag;
  _death_mag = copy._death_mag;
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
  _vertex_array = PTA_Vertexf(new_size * 12);
  _color_array = PTA_Colorf(new_size * 12);

  _line_primitive->set_coords(_vertex_array, G_PER_VERTEX);
  _line_primitive->set_colors(_color_array, G_PER_VERTEX);

  _max_pool_size = new_size;
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : private
// Description : initializes the geomnodes
////////////////////////////////////////////////////////////////////

void SparkleParticleRenderer::
init_geoms(void) {
  _line_primitive->set_num_prims(0);

  _interface_node->clear();
  _interface_node->add_geom(_line_primitive);
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : private
// Description : populates the GeomLine
////////////////////////////////////////////////////////////////////

void SparkleParticleRenderer::
render(vector< PT(PhysicsObject) >& po_vector, int ttl_particles) {

  if (!ttl_particles)
    return;

  BaseParticle *cur_particle;

  int cur_index = 0;
  int remaining_particles = ttl_particles;
  int i;

  Vertexf *cur_vert = &_vertex_array[0];
  Colorf *cur_color = &_color_array[0];

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through the array

  for (i = 0; i < po_vector.size(); i++) {
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

    float mag = get_magnitude(cur_particle);
    float neg_mag = -mag;
    float alpha;

    LPoint3f pos = cur_particle->get_position();
    Colorf center_color = _center_color;
    Colorf edge_color = _edge_color;

    // handle alpha

    if (_alpha_decay != PR_NO_ALPHA) {
      alpha = cur_particle->get_parameterized_age();

      if (_alpha_decay == PR_ALPHA_OUT)
	alpha = 1.0f - alpha;

      center_color[3] = alpha;
      edge_color[3] = alpha;
    }

    // 6 lines coming from the center point.

    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(mag, 0.0f, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(neg_mag, 0.0f, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, mag, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, neg_mag, 0.0f);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, 0.0f, mag);
    *cur_vert++ = pos;
    *cur_vert++ = pos + Vertexf(0.0f, 0.0f, neg_mag);

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

  _interface_node->set_bound(BoundingSphere(aabb_center, radius));
}
