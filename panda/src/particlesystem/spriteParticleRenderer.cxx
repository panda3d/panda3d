// Filename: spriteParticleRenderer.cxx
// Created by:  charles (13Jul00)
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

#include "spriteParticleRenderer.h"

#include "boundingSphere.h"
#include "geom.h"
#include "nodePath.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::SpriteParticleRenderer
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
  _pool_size(0),
  _source_type(ST_texture)
{
  _sprite_primitive = new GeomSprite(tex);
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::SpriteParticleRenderer
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
//    Function : SpriteParticleRenderer::~SpriteParticleRenderer
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
~SpriteParticleRenderer(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::make_copy
//      Access : public
// Description : child dynamic copy
////////////////////////////////////////////////////////////////////
BaseParticleRenderer *SpriteParticleRenderer::
make_copy(void) {
  return new SpriteParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::set_from_node
//      Access : public
// Description : Sets the properties on this render from the geometry
//               referenced by the indicated NodePath.  This should be
//               a reference to a GeomNode; it extracts out the
//               Texture and UV range from the GeomNode.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
set_from_node(const NodePath &node_path) {
  nassertv(!node_path.is_empty());

  // The bottom node must be a GeomNode.  If it is not, find the first
  // GeomNode beneath it.
  NodePath geom_node_path = node_path;
  if (!geom_node_path.node()->is_geom_node()) {
    geom_node_path = node_path.find("**/+GeomNode");
    if (geom_node_path.is_empty()) {
      particlesystem_cat.error()
        << node_path << " does not contain a GeomNode.\n";
      return;
    }
  }
  GeomNode *gnode = DCAST(GeomNode, geom_node_path.node());

  // Get the texture off the node.  We'll take just the first texture.
  Texture *tex = geom_node_path.find_texture("*");

  if (tex == (Texture *)NULL) {
    particlesystem_cat.error()
      << geom_node_path << " has no texture.\n";
    return;
  }

  // Now examine the UV's of the first Geom within the GeomNode.
  nassertv(gnode->get_num_geoms() > 0);
  Geom *geom = gnode->get_geom(0);

  PTA_TexCoordf texcoords;
  GeomBindType bind;
  PTA_ushort tindex;
  geom->get_texcoords(texcoords, bind, tindex);
  if (bind != G_PER_VERTEX) {
    particlesystem_cat.error()
      << geom_node_path << " has no UV's in its first Geom.\n";
    return;
  }

  int num_verts = geom->get_num_vertices();
  if (num_verts == 0) {
    particlesystem_cat.error()
      << geom_node_path << " has no vertices in its first Geom.\n";
    return;
  }

  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();

  const TexCoordf &first_texcoord = geom->get_next_texcoord(ti);
  TexCoordf min_uv = first_texcoord;
  TexCoordf max_uv = first_texcoord;

  for (int v = 1; v < num_verts; v++) {
    const TexCoordf &texcoord = geom->get_next_texcoord(ti);    

    min_uv[0] = min(min_uv[0], texcoord[0]);
    max_uv[0] = max(max_uv[0], texcoord[0]);
    min_uv[1] = min(min_uv[1], texcoord[1]);
    max_uv[1] = max(max_uv[1], texcoord[1]);
  }

  // We don't really pay attention to orientation of UV's here; a
  // minor flaw.  We assume the minimum is in the lower-left, and the
  // maximum is in the upper-right.
  set_texture(tex);
  set_ll_uv(min_uv);
  set_ur_uv(max_uv);
  _source_type = ST_from_node;
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::resize_pool
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
    _x_texel_array = PTA_float::empty_array(new_size);
    _x_bind = G_PER_PRIM;
  }
  else {
    _x_texel_array = PTA_float::empty_array(1);
    _x_bind = G_OVERALL;
  }

  // handle the y texel ratio
  if (_animate_y_ratio == true) {
    _y_texel_array = PTA_float::empty_array(new_size);
    _y_bind = G_PER_PRIM;
  }
  else {
    _y_texel_array = PTA_float::empty_array(1);
    _y_bind = G_OVERALL;
  }

  // handle the theta vector
  if (_animate_theta == true) {
    _theta_array = PTA_float::empty_array(new_size);
    _theta_bind = G_PER_PRIM;
  }
  else {
    _theta_array = PTA_float::empty_array(1);
    _theta_bind = G_OVERALL;
  }

  _vertex_array = PTA_Vertexf::empty_array(new_size);
  _color_array = PTA_Colorf::empty_array(new_size);

  _sprite_primitive->set_coords(_vertex_array);
  _sprite_primitive->set_colors(_color_array, G_PER_PRIM);
  _sprite_primitive->set_x_texel_ratio(_x_texel_array, _x_bind);
  _sprite_primitive->set_y_texel_ratio(_y_texel_array, _y_bind);
  _sprite_primitive->set_thetas(_theta_array, _theta_bind);

  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::init_geoms
//      Access : private
// Description : initializes everything, called on traumatic events
//               such as construction and serious particlesystem
//               modifications
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
init_geoms(void) {
  _sprite_primitive->set_num_prims(0);

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_sprite_primitive, _render_state);
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::birth_particle
//      Access : private
// Description : child birth, one of those 'there-if-we-want-it'
//               things.  not really too useful here, so it turns
//               out we don't really want it.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::kill_particle
//      Access : private
// Description : child death
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::render
//      Access : private
// Description : big child render.  populates the geom node.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
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
        c[3] = (1.0f - t) * get_user_alpha();
      else if (alphamode == PR_ALPHA_IN)
        c[3] = t * get_user_alpha();
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

  _sprite_primitive->set_bound(BoundingSphere(aabb_center, radius));
  get_render_node()->mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SpriteParticleRenderer";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"SpriteParticleRenderer:\n";
  out.width(indent+2); out<<""; out<<"_sprite_primitive "<<_sprite_primitive<<"\n";
  out.width(indent+2); out<<""; out<<"_vertex_array "<<_vertex_array<<"\n";
  out.width(indent+2); out<<""; out<<"_color_array "<<_color_array<<"\n";
  out.width(indent+2); out<<""; out<<"_x_texel_array "<<_x_texel_array<<"\n";
  out.width(indent+2); out<<""; out<<"_y_texel_array "<<_y_texel_array<<"\n";
  out.width(indent+2); out<<""; out<<"_theta_array "<<_theta_array<<"\n";
  out.width(indent+2); out<<""; out<<"_color "<<_color<<"\n";
  out.width(indent+2); out<<""; out<<"_initial_x_texel_ratio "<<_initial_x_texel_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_final_x_texel_ratio "<<_final_x_texel_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_initial_y_texel_ratio "<<_initial_y_texel_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_final_y_texel_ratio "<<_final_y_texel_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_theta "<<_theta<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_x_ratio "<<_animate_x_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_y_ratio "<<_animate_y_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_theta "<<_animate_theta<<"\n";
  out.width(indent+2); out<<""; out<<"_blend_method "<<_blend_method<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_min "<<_aabb_min<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_max "<<_aabb_max<<"\n";
  out.width(indent+2); out<<""; out<<"_pool_size "<<_pool_size<<"\n";
  out.width(indent+2); out<<""; out<<"_source_type "<<_source_type<<"\n";
  BaseParticleRenderer::write(out, indent+2);
  #endif //] NDEBUG
}
