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
#include "geomNode.h"
#include "qpgeom.h"
#include "qpgeomVertexWriter.h"
#include "geomLine.h"

////////////////////////////////////////////////////////////////////
//    Function : LineParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////

LineParticleRenderer::
LineParticleRenderer() :
  _head_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _tail_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)) {

  _line_scale_factor = 1.0f;

  resize_pool(0);
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
  resize_pool(0);
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

  resize_pool(0);
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
  if (!use_qpgeom) {
    _vertex_array = PTA_Vertexf::empty_array(new_size * 2);
    _color_array = PTA_Colorf::empty_array(new_size * 2);
  }

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
  if (use_qpgeom) {
    PT(qpGeom) qpgeom = new qpGeom; 
    _line_primitive = qpgeom;
    _vdata = new qpGeomVertexData
      ("particles", qpGeomVertexFormat::get_v3cp(),
       qpGeomUsageHint::UH_dynamic);
    qpgeom->set_vertex_data(_vdata);
    _lines = new qpGeomLines(qpGeomUsageHint::UH_dynamic);
    qpgeom->add_primitive(_lines);

  } else {
    _line_primitive = new GeomLine;
    _line_primitive->set_coords(_vertex_array);
    _line_primitive->set_colors(_color_array, G_PER_VERTEX);
  }

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
  qpGeomVertexWriter vertex(_vdata, InternalName::get_vertex());
  qpGeomVertexWriter color(_vdata, InternalName::get_color());
  if (use_qpgeom) {
    _lines->clear_vertices();
  }

  // init the aabb

  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through the array

  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (cur_particle->get_alive() == false)
      continue;

    LPoint3f position = cur_particle->get_position();

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
                  else if (_alpha_mode == PR_ALPHA_IN_OUT)
                    alpha = 2.0f * min(alpha, 1.0f - alpha);
          }

      head_color[3] = alpha;
      tail_color[3] = alpha;
    }

    // one line from current position to last position

    if (use_qpgeom) {
      vertex.add_data3f(position);
      LPoint3f last_position = position + 
        (cur_particle->get_last_position() - position) * _line_scale_factor;
      vertex.add_data3f(last_position);
      color.add_data4f(head_color);
      color.add_data4f(tail_color);
      _lines->add_next_vertices(2);
      _lines->close_primitive();
    } else {
      LPoint3f last_position = position + 
        (cur_particle->get_last_position() - position) * _line_scale_factor;

      *cur_vert++ = position;
      *cur_vert++ = last_position;

      *cur_color++ = head_color;
      *cur_color++ = tail_color;
    }

    remaining_particles--;
    if (remaining_particles == 0)
      break;
  }

  if (!use_qpgeom) {
    _line_primitive->set_num_prims(ttl_particles);
  }

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
