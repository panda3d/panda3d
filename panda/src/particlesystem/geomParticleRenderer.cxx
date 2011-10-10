// Filename: geomParticleRenderer.cxx
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "geomParticleRenderer.h"
#include "baseParticle.h"

#include "transformState.h"
#include "colorScaleAttrib.h"
#include "colorAttrib.h"
#include "pStatTimer.h"

PStatCollector GeomParticleRenderer::_render_collector("App:Particles:Geom:Render");

////////////////////////////////////////////////////////////////////
//    Function : GeomParticleRenderer
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
GeomParticleRenderer(ParticleRendererAlphaMode am, PandaNode *geom_node) :
  BaseParticleRenderer(am),
  _geom_node(geom_node),
  _color_interpolation_manager(new ColorInterpolationManager(LColor(1.0f,1.0f,1.0f,1.0f))),
  _pool_size(0),
  _initial_x_scale(1.0f),
  _final_x_scale(1.0f),
  _initial_y_scale(1.0f),
  _final_y_scale(1.0f),
  _initial_z_scale(1.0f),
  _final_z_scale(1.0f),
  _animate_x_ratio(false),
  _animate_y_ratio(false),
  _animate_z_ratio(false)
{
  if (_geom_node.is_null())
    _geom_node = new PandaNode("empty");
}

////////////////////////////////////////////////////////////////////
//    Function : GeomParticleRenderer
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
GeomParticleRenderer(const GeomParticleRenderer& copy) :
  BaseParticleRenderer(copy), 
  _pool_size(0),
  _initial_x_scale(copy._initial_x_scale),
  _final_x_scale(copy._final_x_scale),
  _initial_y_scale(copy._initial_y_scale),
  _final_y_scale(copy._final_y_scale),
  _initial_z_scale(copy._initial_z_scale),
  _final_z_scale(copy._final_z_scale),
  _animate_x_ratio(copy._animate_x_ratio),
  _animate_y_ratio(copy._animate_y_ratio),
  _animate_z_ratio(copy._animate_z_ratio)
{
  _geom_node = copy._geom_node;
}

////////////////////////////////////////////////////////////////////
//    Function : ~GeomParticleRenderer
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
~GeomParticleRenderer() {
  kill_nodes();
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : public
// Description : dynamic copying
////////////////////////////////////////////////////////////////////

BaseParticleRenderer *GeomParticleRenderer::
make_copy() {
  return new GeomParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : private
// Description : links the child nodes to the parent stuff
////////////////////////////////////////////////////////////////////
void GeomParticleRenderer::
init_geoms() {

}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : private
// Description : handles renderer-size resizing.
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
resize_pool(int new_size) {
  kill_nodes();

  // now repopulate the vector with a bunch of NULLS, representing
  // potential instances of the _geom_node.

  int i;
  for (i = 0; i < new_size; i++) {
    _node_vector.push_back(NULL);
  }

  _pool_size = new_size;
}

////////////////////////////////////////////////////////////////////
//  Function : kill_nodes
//    Access : private
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
kill_nodes() {
  pvector< PT(PandaNode) >::iterator vec_iter = _node_vector.begin();

  PandaNode *render_node = get_render_node();
  for (; vec_iter != _node_vector.end(); vec_iter++) {
    PandaNode *node = *vec_iter;
    if (node != (PandaNode *)NULL) {
      render_node->remove_child(node);
    }
  }

  _node_vector.erase(_node_vector.begin(), _node_vector.end());
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private, virtual
// Description : child birth
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
birth_particle(int index) {
  if (_node_vector[index] == (PandaNode *)NULL) {
    PandaNode *node = new PandaNode("");
    get_render_node()->add_child(node);
    node->add_child(_geom_node);
    _node_vector[index] = node;
  }
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private, virtual
// Description : child kill
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
kill_particle(int index) {
  if (_node_vector[index] != (PandaNode *)NULL) {
    get_render_node()->remove_child(_node_vector[index]);
    _node_vector[index] = (PandaNode *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : private
// Description : sets the transitions on each arc
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  PStatTimer t1(_render_collector);

  BaseParticle *cur_particle;
  int i, remaining_particles = ttl_particles;

  pvector< PT(PandaNode) >::iterator cur_node_iter = _node_vector.begin();

  // run through the particle vector

  for (i = 0; i < (int)po_vector.size(); i++) {
    PandaNode *cur_node;

    cur_particle = (BaseParticle *) po_vector[i].p();
    cur_node = *cur_node_iter;

    if (cur_particle->get_alive()) {
      // living particle
      if (cur_node == (PandaNode *)NULL) {
        birth_particle(i);
        cur_node = *cur_node_iter;
      }
      nassertv(cur_node != (PandaNode *)NULL);

      cur_node->set_state(_render_state);

      PN_stdfloat t = cur_particle->get_parameterized_age();
      LColor c = _color_interpolation_manager->generateColor(t);

      if ((_alpha_mode != PR_ALPHA_NONE)) {
        PN_stdfloat alpha_scalar;

        if(_alpha_mode == PR_ALPHA_USER) {
          alpha_scalar = get_user_alpha();
        } else {
          alpha_scalar = t;
          if (_alpha_mode == PR_ALPHA_OUT)
            alpha_scalar = 1.0f - alpha_scalar;
          else if (_alpha_mode == PR_ALPHA_IN_OUT)
            alpha_scalar = 2.0f * min(alpha_scalar, 1.0f - alpha_scalar);
          alpha_scalar *= get_user_alpha();
        }
        
        c[3] *= alpha_scalar;
        cur_node->set_attrib(ColorScaleAttrib::make
                             (LColor(1.0f, 1.0f, 1.0f, c[3])));
      }

      cur_node->set_attrib(ColorAttrib::make_flat(c), 0);

      // animate scale
      PN_stdfloat current_x_scale = _initial_x_scale;
      PN_stdfloat current_y_scale = _initial_y_scale;
      PN_stdfloat current_z_scale = _initial_z_scale;

      if (_animate_x_ratio || _animate_y_ratio || _animate_z_ratio) {
        if (_animate_x_ratio) {
          current_x_scale = (_initial_x_scale + 
                             (t * (_final_x_scale - _initial_x_scale)));
        }
        if (_animate_y_ratio) {
          current_y_scale = (_initial_y_scale + 
                             (t * (_final_y_scale - _initial_y_scale)));
        }
        if (_animate_z_ratio) {
          current_z_scale = (_initial_z_scale + 
                             (t * (_final_z_scale - _initial_z_scale)));
        }
      }

      cur_node->set_transform(TransformState::make_pos_quat_scale
                              (cur_particle->get_position(),
                               cur_particle->get_orientation(),
                               LVecBase3(current_x_scale, current_y_scale, current_z_scale)));

      // maybe get out early if possible.

      remaining_particles--;

      if (remaining_particles == 0)
        break;
    }

    cur_node_iter++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void GeomParticleRenderer::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"GeomParticleRenderer";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write_linear_forces
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void GeomParticleRenderer::
write_linear_forces(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_node_vector ("<<_node_vector.size()<<" forces)\n";
  for (pvector< PT(PandaNode) >::const_iterator i=_node_vector.begin();
       i != _node_vector.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void GeomParticleRenderer::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"GeomParticleRenderer:\n";
  out.width(indent+2); out<<""; out<<"_geom_node "<<_geom_node<<"\n";
  out.width(indent+2); out<<""; out<<"_pool_size "<<_pool_size<<"\n";

  out.width(indent+2); out<<""; out<<"_initial_x_scale "<<_initial_x_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_final_x_scale "<<_final_x_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_initial_y_scale "<<_initial_y_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_final_y_scale "<<_final_y_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_initial_z_scale "<<_initial_z_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_final_z_scale "<<_final_z_scale<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_x_ratio "<<_animate_x_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_y_ratio "<<_animate_y_ratio<<"\n";
  out.width(indent+2); out<<""; out<<"_animate_z_ratio "<<_animate_z_ratio<<"\n";

  write_linear_forces(out, indent+2);
  BaseParticleRenderer::write(out, indent+2);
  #endif //] NDEBUG
}
