// Filename: collisionHandlerFloor.cxx
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerFloor.h"
#include "config_collide.h"

#include <renderRelation.h>
#include <transformTransition.h>

TypeHandle CollisionHandlerFloor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerFloor::
CollisionHandlerFloor() {
  _offset = 0.0;
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

	if (entry->has_into_intersection_point()) {
	  LPoint3f point =
	    entry->get_into_intersection_point() * 
	    entry->get_into_space();
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
      if (collide_cat.is_debug()) {
	collide_cat.debug()
	  << "Resetting height to " << max_height << " + " << _offset << "\n";
      }
      LMatrix4f mat;
      def.get_mat(mat);
      mat(3, 2) = max_height + _offset;
      def.set_mat(mat);
    }
  }
}
