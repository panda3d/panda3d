// Filename: renderRelation.cxx
// Created by:  drose (26Oct98)
// 

#include "renderRelation.h"
#include "transformTransition.h"

#include <geometricBoundingVolume.h>

TypeHandle RenderRelation::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::changed_transition
//       Access: Public, Virtual
//  Description: This is called by set_transition() or
//               clear_transition() whenever a transition is added,
//               updated, or removed from the arc.  It is just a 
//               callback to the arc so it can decide whether
//               it needs to update any internal data as a response to
//               this adjustment (for instance, by marking the
//               bounding sphere stale).
////////////////////////////////////////////////////////////////////
void RenderRelation::
changed_transition(TypeHandle trans_type) {
  if (trans_type == TransformTransition::get_class_type()) {
    // Changing the transform means changing our bounding volume.
    mark_bound_stale();

    // And we also might want to tell our node below that the
    // transform has changed.
    nassertv(get_child() != (Node *)NULL);
    get_child()->transform_changed(this);
  }
  NodeRelation::changed_transition(trans_type);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this arc
//               (and all of its descendants).
////////////////////////////////////////////////////////////////////
void RenderRelation::
recompute_bound() {
  // First, compute the bounding volume around all of our children.
  NodeRelation::recompute_bound();
  nassertv(_bound != (BoundingVolume *)NULL);

  // Now, if we have a transform transition on the arc, apply it to
  // the bounding volume.
  GeometricBoundingVolume *gbv;
  DCAST_INTO_V(gbv, _bound);

  const TransformTransition *tt;
  if (get_transition_into(tt, this)) {
    gbv->xform(tt->get_matrix());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::make_arc
//       Access: Public, Static
//  Description: This function is passed to the Factory to make a new
//               RenderRelation by type.  Don't try to call this
//               function directly.
////////////////////////////////////////////////////////////////////
NodeRelation *RenderRelation::
make_arc(const FactoryParams &) {
  return new RenderRelation;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::make_RenderRelation
//       Access: Protected
//  Description: Factory method to generate a RenderRelation object
////////////////////////////////////////////////////////////////////
TypedWriteable* RenderRelation::
make_RenderRelation(const FactoryParams &params)
{
  RenderRelation *me = new RenderRelation;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a RenderRelation object
////////////////////////////////////////////////////////////////////
void RenderRelation::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_RenderRelation);
}
