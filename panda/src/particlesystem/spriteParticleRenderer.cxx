// Filename: spriteParticleRenderer.cxx
// Created by:  charles (13Jul00)
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

#include <boundingSphere.h>
#include <geom.h>

#include "spriteParticleRenderer.h"

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
SpriteParticleRenderer(Texture *tex) :
  BaseParticleRenderer(PR_ALPHA_NONE),
  _color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _initial_x_texel_ratio(0.02f),
  _final_x_texel_ratio(0.02f),
  _initial_y_texel_ratio(0.02f),
  _final_y_texel_ratio(0.02f),
  _theta(0.0f),
  _animate_x_ratio(false),
  _animate_y_ratio(false),
  _animate_theta(false),
  _blend_method(PP_BLEND_LINEAR),
  _pool_size(0)
{
  _sprite_primitive = new GeomSprite(tex);
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
SpriteParticleRenderer(const SpriteParticleRenderer& copy) :
  BaseParticleRenderer(copy), _pool_size(0) {
  _animate_x_ratio = copy._animate_x_ratio;
  _animate_y_ratio = copy._animate_y_ratio;
  _animate_theta = copy._animate_theta;
  _blend_method = copy._blend_method;
  _initial_x_texel_ratio = copy._initial_x_texel_ratio;
  _final_x_texel_ratio = copy._final_x_texel_ratio;
  _initial_y_texel_ratio = copy._initial_y_texel_ratio;
  _final_y_texel_ratio = copy._final_y_texel_ratio;
  _theta = copy._theta;
  _color = copy._color;
  _sprite_primitive = new GeomSprite(copy.get_texture());
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : ~SpriteParticleRenderer
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
~SpriteParticleRenderer(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : public
// Description : child dynamic copy
////////////////////////////////////////////////////////////////////
BaseParticleRenderer *SpriteParticleRenderer::
make_copy(void) {
  return new SpriteParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : private
// Description : reallocate the vertex pool.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
resize_pool(int new_size) {
  if (new_size == _pool_size)
    return;

  _pool_size = new_size;

  GeomBindType _x_bind, _y_bind, _theta_bind;

  // handle the x texel ratio
  if (_animate_x_ratio == true) {
    _x_texel_array = PTA_float(new_size);
    _x_bind = G_PER_PRIM;
  }
  else {
    _x_texel_array = PTA_float(1);
    _x_bind = G_OVERALL;
  }

  // handle the y texel ratio
  if (_animate_y_ratio == true) {
    _y_texel_array = PTA_float(new_size);
    _y_bind = G_PER_PRIM;
  }
  else {
    _y_texel_array = PTA_float(1);
    _y_bind = G_OVERALL;
  }

  // handle the theta vector
  if (_animate_theta == true) {
    _theta_array = PTA_float(new_size);
    _theta_bind = G_PER_PRIM;
  }
  else {
    _theta_array = PTA_float(1);
    _theta_bind = G_OVERALL;
  }

  _vertex_array = PTA_Vertexf(new_size);
  _color_array = PTA_Colorf(new_size);

  _sprite_primitive->set_coords(_vertex_array, G_PER_VERTEX);
  _sprite_primitive->set_colors(_color_array, G_PER_PRIM);
  _sprite_primitive->set_x_texel_ratio(_x_texel_array, _x_bind);
  _sprite_primitive->set_y_texel_ratio(_y_texel_array, _y_bind);
  _sprite_primitive->set_thetas(_theta_array, _theta_bind);

  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : private
// Description : initializes everything, called on traumatic events
//               such as construction and serious particlesystem
//               modifications
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
init_geoms(void) {
  _sprite_primitive->set_num_prims(0);

  _interface_node->clear();
  _interface_node->add_geom(_sprite_primitive);
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : private
// Description : child birth, one of those 'there-if-we-want-it'
//               things.  not really too useful here, so it turns
//               out we don't really want it.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : private
// Description : child death
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : private
// Description : big child render.  populates the geom node.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
render(vector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  Vertexf *cur_vert = &_vertex_array[0];
  Colorf *cur_color = &_color_array[0];
  float *cur_x_texel = &_x_texel_array[0];
  float *cur_y_texel = &_y_texel_array[0];
  float *cur_theta = &_theta_array[0];

  if (_animate_x_ratio == false)
    *cur_x_texel = _initial_x_texel_ratio;

  if (_animate_y_ratio == false)
    *cur_y_texel = _initial_y_texel_ratio;

  if (_animate_theta == false)
    *cur_theta = _theta;

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

    // put the current vertex into the array
    *cur_vert++ = cur_particle->get_position();

    // put the current color into the array
    Colorf c = _color;

        int alphamode=get_alpha_mode();
    if (alphamode != PR_ALPHA_NONE) {
      float t = cur_particle->get_parameterized_age();

      if (alphamode == PR_ALPHA_OUT)
        c[3] = 1.0f - t;
      else if (alphamode == PR_ALPHA_IN)
        c[3] = t;
          else {
                  assert(alphamode == PR_ALPHA_USER);
                  c[3] = get_user_alpha();
          }
    }

    *cur_color++ = c;

    // handle x scaling
    if (_animate_x_ratio == true) {
      float t = cur_particle->get_parameterized_age();

      if (_blend_method == PP_BLEND_CUBIC)
        t = CUBIC_T(t);

      *cur_x_texel++ = (_initial_x_texel_ratio +
        (t * (_final_x_texel_ratio - _initial_x_texel_ratio)));
    }

    // handle y scaling
    if (_animate_y_ratio == true) {
      float t = cur_particle->get_parameterized_age();

      if (_blend_method == PP_BLEND_CUBIC)
        t = CUBIC_T(t);

      *cur_y_texel++ = (_initial_y_texel_ratio +
        (t * (_final_y_texel_ratio - _initial_y_texel_ratio)));
    }

    // handle theta
    if (_animate_theta == true)
      *cur_theta++ = cur_particle->get_theta();

    // maybe jump out early?
    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  _sprite_primitive->set_num_prims(ttl_particles);

  // done filling geompoint node, now do the bb stuff
  LPoint3f aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  float radius = (aabb_center - _aabb_min).length();

  _interface_node->set_bound(BoundingSphere(aabb_center, radius));
}
