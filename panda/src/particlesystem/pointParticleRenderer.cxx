// Filename: pointParticleRenderer.C
// Created by:  charles (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <boundingSphere.h>
#include "pointParticleRenderer.h"

////////////////////////////////////////////////////////////////////
//    Function : PointParticleRenderer
//      Access : Public
// Description : special constructor
////////////////////////////////////////////////////////////////////

PointParticleRenderer::
PointParticleRenderer(ParticleRendererAlphaMode am,
		      float point_size,
		      PointParticleBlendType bt,
		      ParticleRendererBlendMethod bm,
		      const Colorf& sc, const Colorf& ec) :
  _point_size(point_size),
  _blend_type(bt), _blend_method(bm),
  _start_color(sc), _end_color(ec),
  BaseParticleRenderer(am) {

  _point_primitive = new GeomPoint;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : PointParticleRenderer
//      Access : Public
// Description : Copy constructor
////////////////////////////////////////////////////////////////////

PointParticleRenderer::
PointParticleRenderer(const PointParticleRenderer& copy) :
  _max_pool_size(0),
  BaseParticleRenderer(copy) {

  _blend_type = copy._blend_type;
  _blend_method = copy._blend_method;
  _start_color = copy._start_color;
  _end_color = copy._end_color;
  _point_primitive = new GeomPoint;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : ~PointParticleRenderer
//      Access : Public
// Description : Simple destructor
////////////////////////////////////////////////////////////////////

PointParticleRenderer::
~PointParticleRenderer(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : for spawning systems from dead particles
////////////////////////////////////////////////////////////////////

BaseParticleRenderer *PointParticleRenderer::
make_copy(void) {
  return new PointParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : Public
// Description : reallocate the space for the vertex and color
//               pools
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
resize_pool(int new_size) {
  if (new_size == _max_pool_size)
    return;

  _max_pool_size = new_size;

  _vertex_array = PTA_Vertexf(new_size);
  _color_array = PTA_Colorf(new_size);

  _point_primitive->set_coords(_vertex_array, G_PER_VERTEX);
  _point_primitive->set_colors(_color_array, G_PER_VERTEX);
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : Private
// Description : On-construction initialization
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
init_geoms(void) {

  _point_primitive->set_num_prims(0);
  _point_primitive->set_size(_point_size);

  _interface_node->clear();
  _interface_node->add_geom(_point_primitive);
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private, virtual
// Description : child birth
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private, virtual
// Description : child kill
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : create_color
//      Access : Private
// Description : Generates the point color based on the render_type
////////////////////////////////////////////////////////////////////

Colorf PointParticleRenderer::
create_color(const BaseParticle *p) {
  Colorf color;
  float life_t, vel_t, alpha_linear_t;
  bool have_alpha_t = false;

  switch (_blend_type) {

    //// Constant solid color

  case PP_ONE_COLOR:
    color = _start_color;
    break;

    //// Blending colors based on life

  case PP_BLEND_LIFE:
    life_t = p->get_parameterized_age();
    alpha_linear_t = life_t;
    have_alpha_t = true;

    if (_blend_method == PP_BLEND_CUBIC)
      life_t = CUBIC_T(life_t);

    color = LERP(life_t, _start_color, _end_color);

    break;

    //// Blending colors based on vel

  case PP_BLEND_VEL:
    vel_t = p->get_parameterized_vel();

    if (_blend_method == PP_BLEND_CUBIC)
      vel_t = CUBIC_T(vel_t);

    color = LERP(vel_t, _start_color, _end_color);

    break;
  }

  // handle alpha channel

  if (!((_alpha_mode == PR_ALPHA_NONE) || (_alpha_mode == PR_ALPHA_USER))) {
    if (have_alpha_t == false)
      alpha_linear_t = p->get_parameterized_age();

    if (_alpha_mode == PR_ALPHA_OUT)
      color[3] = 1.0f - alpha_linear_t;
    else
      color[3] = alpha_linear_t;
  }

  return color;
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : Public
// Description : renders the particle system out to a GeomNode
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
render(vector< PT(PhysicsObject) >& po_vector, int ttl_particles) {

  BaseParticle *cur_particle;

  int cur_index = 0;
  int remaining_particles = ttl_particles;
  int i;

  Vertexf *cur_vert = &_vertex_array[0];
  Colorf *cur_color = &_color_array[0];

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through every filled slot

  for (i = 0; i < po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (cur_particle->get_alive() == false)
      continue;

    // x aabb adjust

    if (cur_particle->get_position().get_x() > _aabb_max.get_x())
      _aabb_max[0] = cur_particle->get_position().get_x();
    else if (cur_particle->get_position().get_x() < _aabb_min.get_x())
      _aabb_min[0] = cur_particle->get_position().get_x();

    // y aabb adjust

    if (cur_particle->get_position().get_y() > _aabb_max.get_y())
      _aabb_max[1] = cur_particle->get_position().get_y();
    else if (cur_particle->get_position().get_y() < _aabb_min.get_y())
      _aabb_min[1] = cur_particle->get_position().get_y();

    // z aabb adjust

    if (cur_particle->get_position().get_z() > _aabb_max.get_z())
      _aabb_max[2] = cur_particle->get_position().get_z();
    else if (cur_particle->get_position().get_z() < _aabb_min.get_z())
      _aabb_min[2] = cur_particle->get_position().get_z();

    // stuff it into the arrays

    *cur_vert++ = cur_particle->get_position();
    *cur_color++ = create_color(cur_particle);

    // maybe jump out early?

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  _point_primitive->set_num_prims(ttl_particles);

  // done filling geompoint node, now do the bb stuff

  LPoint3f aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  float radius = (aabb_center - _aabb_min).length();

  _interface_node->set_bound(BoundingSphere(aabb_center, radius));
}
