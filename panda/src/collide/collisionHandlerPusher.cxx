// Filename: collisionHandlerPusher.cxx
// Created by:  drose (25Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerPusher.h"
#include "config_collide.h"

TypeHandle CollisionHandlerPusher::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPusher::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerPusher::
CollisionHandlerPusher() {
  _horizontal = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPusher::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerPusher::
~CollisionHandlerPusher() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPusher::handle_entries
//       Access: Protected, Virtual
//  Description: Called by the parent class after all collisions have
//               been detected, this manages the various collisions
//               and moves around the nodes as necessary.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPusher::
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
	<< "CollisionHandlerPusher doesn't know about " 
	<< *from_node << "\n";

    } else {
      const ColliderDef &def = (*ci).second;

      Entries::const_iterator ei;
      for (ei = entries.begin(); ei != entries.end(); ++ei) {
	CollisionEntry *entry = (*ei);
	nassertv(entry != (CollisionEntry *)NULL);
	nassertv(from_node == entry->get_from_node());

	if (!entry->has_into_surface_normal() ||
	    !entry->has_into_depth()) {
	  if (collide_cat.is_debug()) {
	    collide_cat.debug()
	      << "Cannot shove on " << *from_node << " for collision into "
	      << *entry->get_into_node() << "; no normal/depth information.\n";
	  }

	} else {
	  // Shove it just enough to clear the volume.
	  LVector3f shove = 
	    entry->get_into_surface_normal() *
	    entry->get_into_depth();
	  
	  if (_horizontal) {
	    shove[2] = 0.0;
	  }
	  
	  if (collide_cat.is_debug()) {
	    collide_cat.debug()
	      << "Shoving on " << *from_node << " in the amount of: " 
	      << shove << "\n";
	  }
	  
	  LMatrix4f mat;
	  def.get_mat(mat);
	  def.set_mat(LMatrix4f::translate_mat(shove) * mat);
	}
      }
    }
  }
}
