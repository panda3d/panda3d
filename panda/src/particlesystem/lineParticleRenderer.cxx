// Filename: lineParticleRenderer.cxx
// Created by:  darren (06Oct00)
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

#include "lineParticleRenderer.h"

#include "boundingSphere.h"

////////////////////////////////////////////////////////////////////
//    Function : LineParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////

LineParticleRenderer::
LineParticleRenderer() :
  _head_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _tail_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)) {
  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : LineParticleRenderer
//      Access : Public
// Description : Constructor
////////////////////////////////////////////////////////////////////

LineParticleRenderer::
LineParticleRenderer(const Colorf& head,
                     const Colorf& tail,
                     ParticleRendererAlphaMode alpha_mode) :
  BaseParticleRenderer(alpha_mode),
  _head_color(head), _tail_color(tail)
{
  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : LineParticleRenderer
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////

LineParticleRenderer::
LineParticleRenderer(const LineParticleRenderer& copy) :
  BaseParticleRenderer(copy) {
  _head_color = copy._head_color;
  _tail_color = copy._tail_color;

  _line_primitive = new GeomLine;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : ~LineParticleRenderer
//      Access : Public
// Description : Destructor
////////////////////////////////////////////////////////////////////

LineParticleRenderer::
~LineParticleRenderer() {
}

////////////////////////////////////////////////////////////////////
//    Function : make copy
//      Access : Public
// Description : child virtual for spawning systems
////////////////////////////////////////////////////////////////////

BaseParticleRenderer *LineParticleRenderer::
make_copy() {
  return new LineParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private, virtual
// Description : child birth
////////////////////////////////////////////////////////////////////

void LineParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private, virtual
// Description : child kill
////////////////////////////////////////////////////////////////////

void LineParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : private
// Description : resizes the render pool.  Reference counting
//               makes this easy.
////////////////////////////////////////////////////////////////////

void LineParticleRenderer::
resize_pool(int new_size) {
  _vertex_array = PTA_Vertexf::empty_array(new_size * 2);
  _color_array = PTA_Colorf::empty_array(new_size * 2);

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

void LineParticleRenderer::
init_geoms() {
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

void LineParticleRenderer::
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

    LPoint3f pos = cur_particle->get_position();

    // adjust the aabb

    if (pos.get_x() > _aabb_max.get_x())
      _aabb_max[0] = pos.get_x();
    if (pos.get_x() < _aabb_min.get_x())
      _aabb_min[0] = pos.get_x();

    if (pos.get_y() > _aabb_max.get_y())
      _aabb_max[1] = pos.get_y();
    if (pos.get_y() < _aabb_min.get_y())
      _aabb_min[1] = pos.get_y();

    if (pos.get_z() > _aabb_max.get_z())
      _aabb_max[2] = pos.get_z();
    if (pos.get_z() < _aabb_min.get_z())
      _aabb_min[2] = pos.get_z();

    // draw the particle.

    Colorf head_color = _head_color;
    Colorf tail_color = _tail_color;

    // handle alpha

    if (_alpha_mode != PR_ALPHA_NONE) {

          float alpha;

      if (_alpha_mode == PR_ALPHA_USER) {
                  alpha = get_user_alpha();
          } else {
                  alpha = cur_particle->get_parameterized_age();
                  if (_alpha_mode == PR_ALPHA_OUT)
                          alpha = 1.0f - alpha;
          }

      head_color[3] = alpha;
      tail_color[3] = alpha;
    }

    // one line from current position to last position

    *cur_vert++ = pos;
    *cur_vert++ = cur_particle->get_last_position();

    *cur_color++ = head_color;
    *cur_color++ = tail_color;

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  _line_primitive->set_num_prims(ttl_particles);

  // done filling geomline node, now do the bb stuff

  LPoint3f aabb_center = (_aabb_min + _aabb_max) * 0.5f;
  float radius = (aabb_center - _aabb_min).length();

  _line_primitive->set_bound(BoundingSphere(aabb_center, radius));
  get_render_node()->mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LineParticleRenderer::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LineParticleRenderer";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LineParticleRenderer::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LineParticleRenderer:\n";
  out.width(indent+2); out<<""; out<<"_head_color "<<_head_color<<"\n";
  out.width(indent+2); out<<""; out<<"_tail_color "<<_tail_color<<"\n";
  out.width(indent+2); out<<""; out<<"_line_primitive "<<_line_primitive<<"\n";
  out.width(indent+2); out<<""; out<<"_vertex_array "<<_vertex_array<<"\n";
  out.width(indent+2); out<<""; out<<"_color_array "<<_color_array<<"\n";
  out.width(indent+2); out<<""; out<<"_max_pool_size "<<_max_pool_size<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_min "<<_aabb_min<<"\n";
  out.width(indent+2); out<<""; out<<"_aabb_max "<<_aabb_max<<"\n";
  BaseParticleRenderer::write(out, indent+2);
  #endif //] NDEBUG
}
