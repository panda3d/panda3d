// Filename: geomParticleRenderer.C
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "geomParticleRenderer.h"
#include "baseParticle.h"

#include <transformTransition.h>
#include <colorTransition.h>

////////////////////////////////////////////////////////////////////
//    Function : GeomParticleRenderer
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
GeomParticleRenderer(ParticleRendererAlphaMode am, Node *geom_node) :
  _geom_node(geom_node), _pool_size(0), BaseParticleRenderer(am) {

  _dead_particle_parent_node = new Node;

  if (_geom_node.is_null())
    _geom_node = _dead_particle_parent_node;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomParticleRenderer
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
GeomParticleRenderer(const GeomParticleRenderer& copy) :
  _pool_size(0), BaseParticleRenderer(copy) {

  _dead_particle_parent_node = new Node;

  _geom_node = copy._geom_node;
}

////////////////////////////////////////////////////////////////////
//    Function : ~GeomParticleRenderer
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

GeomParticleRenderer::
~GeomParticleRenderer(void) {
  kill_arcs();
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : public
// Description : dynamic copying
////////////////////////////////////////////////////////////////////

BaseParticleRenderer *GeomParticleRenderer::
make_copy(void) {
  return new GeomParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : init_geoms
//      Access : private
// Description : links the child nodes to the parent stuff
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
init_geoms(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : private
// Description : handles renderer-size resizing.
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
resize_pool(int new_size) {
  kill_arcs();

  // now repopulate the vector

  int i;
  RenderRelation *rr;

  for (i = 0; i < new_size; i++) {
    rr = new RenderRelation(_dead_particle_parent_node, _geom_node);
    _arc_vector.push_back(rr);
  }

  _pool_size = new_size;
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private, virtual
// Description : child birth
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
birth_particle(int index) {
  _arc_vector[index]->change_parent(_interface_node);
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private, virtual
// Description : child kill
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
kill_particle(int index) {
  _arc_vector[index]->change_parent(_dead_particle_parent_node);
}

////////////////////////////////////////////////////////////////////
//    Function : render
//      Access : private
// Description : sets the transitions on each arc
////////////////////////////////////////////////////////////////////

void GeomParticleRenderer::
render(vector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  BaseParticle *cur_particle;
  LPoint3f pos;
  int i, remaining_particles = ttl_particles;

  vector< PT(RenderRelation) >::iterator cur_arc_iter = _arc_vector.begin();

  // run through the particle vector

  for (i = 0; i < po_vector.size(); i++) {
    RenderRelation *cur_arc;

    cur_particle = (BaseParticle *) po_vector[i].p();
    cur_arc = *cur_arc_iter;

    if (cur_particle->get_alive() == true) {
      // living particle

      pos = cur_particle->get_position();

      PT(TransformTransition) xform;
      PT(ColorTransition) alpha;
      xform = new TransformTransition(LMatrix4f::translate_mat(pos));

      if ((_alpha_mode != PR_ALPHA_NONE) && (_alpha_mode != PR_ALPHA_USER)) {

	float alpha_scalar = cur_particle->get_parameterized_age();

	if (_alpha_mode == PR_ALPHA_IN) {
	  alpha = new ColorTransition(1.0f, 1.0f, 1.0f, alpha_scalar);
	}
	else if (_alpha_mode == PR_ALPHA_OUT) {
	  alpha = new ColorTransition(1.0f, 1.0f, 1.0f, 1.0f - alpha_scalar);
	}

	cur_arc->set_transition(alpha);
      }

      cur_arc->set_transition(xform);

      // maybe get out early if possible.

      remaining_particles--;

      if (remaining_particles == 0)
	break;
    }

    cur_arc_iter++;
  }
}
