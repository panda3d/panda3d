// Filename: collisionHandlerFloor.cxx
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerFloor.h"
#include "config_collide.h"

#include <renderRelation.h>
#include <transformTransition.h>
#include <clockObject.h>

TypeHandle CollisionHandlerFloor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerFloor::
CollisionHandlerFloor() {
  _offset = 0.0;
  _max_velocity = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerFloor::
~CollisionHandlerFloor() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::handle_entries
//       Access: Protected, Virtual
//  Description: Called by the parent class after all collisions have
//               been detected, this manages the various collisions
//               and moves around the nodes as necessary.
////////////////////////////////////////////////////////////////////
void CollisionHandlerFloor::
handle_entries() {
  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    CollisionNode *from_node = (*fi).first;
    nassertv(from_node != (CollisionNode *)NULL);
    const Entries &entries = (*fi).second;

    Colliders::const_iterator ci;
    ci = _colliders.find(from_node);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave
      // it this CollisionHandler pointer--but they didn't tell us
      // about the node.
      collide_cat.error()
	<< "CollisionHandlerFloor doesn't know about " 
	<< *from_node << "\n";

    } else {
      const ColliderDef &def = (*ci).second;

      // Get the maximum height for all collisions with this node.
      bool got_max = false;
      float max_height = 0.0;

      Entries::const_iterator ei;
      for (ei = entries.begin(); ei != entries.end(); ++ei) {
	CollisionEntry *entry = (*ei);
	nassertv(entry != (CollisionEntry *)NULL);
	nassertv(from_node == entry->get_from_node());

	if (entry->has_from_intersection_point()) {
	  LPoint3f point = entry->get_from_intersection_point();
	  if (collide_cat.is_debug()) {
	    collide_cat.debug()
	      << "Intersection point detected at " << point << "\n";
	  }

	  float height = point[2];
	  if (!got_max || height > max_height) {
	    got_max = true;
	    max_height = height;
	  }
	}
      }

      // Now set our height accordingly.
      float adjust = max_height + _offset;
      if (!IS_THRESHOLD_ZERO(adjust, 0.001)) {
	if (collide_cat.is_debug()) {
	  collide_cat.debug()
	    << "Adjusting height by " << adjust << "\n";
	}

	if (adjust < 0.0 && _max_velocity != 0.0) {
	  float max_adjust = 
	    _max_velocity * ClockObject::get_global_clock()->get_dt();
	  adjust = max(adjust, -max_adjust);
	}

	LMatrix4f mat;
	def.get_mat(mat);
	mat(3, 2) += adjust;
	def.set_mat(mat);
      } else {
	if (collide_cat.is_spam()) {
	  collide_cat.spam()
	    << "Leaving height unchanged.\n";
	}
      }
    }
  }
}
