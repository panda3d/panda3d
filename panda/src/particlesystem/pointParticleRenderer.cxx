// Filename: pointParticleRenderer.cxx
// Created by:  charles (20Jun00)
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

#include "pointParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"

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
  BaseParticleRenderer(am),
  _start_color(sc), _end_color(ec),
  _point_size(point_size),
  _blend_type(bt), _blend_method(bm)
{
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
  BaseParticleRenderer(copy),
  _max_pool_size(0)
{
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

  _vertex_array = PTA_Vertexf::empty_array(new_size);
  _color_array = PTA_Colorf::empty_array(new_size);

  _point_primitive->set_coords(_vertex_array);
  _point_primitive->set_colors(_color_array, G_PER_VERTEX);

  init_geoms();
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
  
  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_point_primitive, _render_state);
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
  float life_t, vel_t;
  float parameterized_age = 1.0f;
  bool have_alpha_t = false;

  switch (_blend_type) {

    //// Constant solid color

  case PP_ONE_COLOR:
    color = _start_color;
    break;

    //// Blending colors based on life

  case PP_BLEND_LIFE:
    parameterized_age = p->get_parameterized_age();
    life_t = parameterized_age;
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

  if(_alpha_mode != PR_ALPHA_NONE) {
    if(_alpha_mode == PR_ALPHA_USER) {
      parameterized_age = 1.0;
    } else {
      if(!have_alpha_t)
        parameterized_age = p->get_parameterized_age();

      if(_alpha_mode==PR_ALPHA_OUT) {
        parameterized_age = 1.0f - parameterized_age;
      }
    }
    color[3] = parameterized_age * get_user_alpha();
  }

  return color;
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : Public
// Description : renders the particle system out to a GeomNode
////////////////////////////////////////////////////////////////////

void PointParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {

  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  Vertexf *cur_vert = &_vertex_array[0];
  Colorf *cur_color = &_color_array[0];

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through every filled slot

  for (i = 0; i < (int)po_vector.size(); i++) {
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

  _point_primitive->set_bound(BoundingSphere(aabb_center, radius));
  get_render_node()->mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointParticleRenderer::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointParticleRenderer";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointParticleRenderer::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointParticleRenderer:\n";
  out.width(indent+2); out<<""; out<<"_start_color "<<_start_color<<"\n";
  out.width(indent+2); out<<""; out<<"_end_color "<<_end_color<<"\n";
  out.width(indent+2); out<<""; out<<"_point_size "<<_point_size<<"\n";
  out.width(indent+2); out<<""; out<<"_point_primitive "<<_point_primitive<<"\n";
  out.width(indent+2); out<<""; out<<"_vertex_array "<<_vertex_array<<"\n";
  out.width(indent+2); out<<""; out<<"_color_array "<<_color_array<<"\n";
  out.width(indent+2); out<<""; out<<"_max_pool_size "<<_max_pool_size<<"\n";
  out.width(indent+2); out<<""; out<<"_blend_type "<<_blend_type<<"\n";
  out.width(indent+2); out<<""; out<<"_blend_method "<<_blend_method<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_min "<<_aabb_min<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_max "<<_aabb_max<<"\n";
  BaseParticleRenderer::write(out, indent+2);
  #endif //] NDEBUG
}
