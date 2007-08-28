// Filename: collisionHandlerFluidPusher.cxx
// Created by:  drose (16Mar02)
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

#include "collisionHandlerFluidPusher.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "config_collide.h"
#include "dcast.h"

TypeHandle CollisionHandlerFluidPusher::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFluidPusher::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerFluidPusher::
CollisionHandlerFluidPusher() {
  _wants_all_potential_collidees = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFluidPusher::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void CollisionHandlerFluidPusher::
add_entry(CollisionEntry *entry) {
  nassertv(entry != (CollisionEntry *)NULL);
  // skip over CollisionHandlerPhysical::add_entry, since it filters
  // out collidees by orientation; our collider can change direction
  // mid-frame, so it may collide with something that would have been
  // filtered out
  CollisionHandlerEvent::add_entry(entry);

  // filter out non-tangibles
  if (entry->get_from()->is_tangible() &&
      (!entry->has_into() || entry->get_into()->is_tangible())) {

    _from_entries[entry->get_from_node_path()].push_back(entry);
    if (entry->collided()) {
      _has_contact = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFluidPusher::handle_entries
//       Access: Protected, Virtual
//  Description: Calculates a reasonable final position for a 
//               collider given a set of collidees
////////////////////////////////////////////////////////////////////
bool CollisionHandlerFluidPusher::
handle_entries() {
  /*
    This pusher repeatedly calculates the first collision, calculates a new
    trajectory based on that collision, and repeats until the original motion is
    exhausted or the collider becomes "stuck". This solves the "acute collisions"
    problem where colliders could bounce their way through to the other side
    of a wall.
    
    Pseudocode:
    
    INPUTS
    PosA = collider's previous position
    PosB = collider's current position
    M = movement vector (PosB - PosA)
    BV = bounding sphere that includes collider at PosA and PosB
    CS = 'collision set', all 'collidables' within BV (collision polys, tubes, etc)
    
    VARIABLES
    N = movement vector since most recent collision (or start of frame)
    SCS = 'sub collision set', all collidables that could still be collided with
    C = single collider currently being collided with
    PosX = new position given movement along N interrupted by collision with C
    
    OUTPUTS
    final position is PosX
    
    1. N = M, SCS = CS, PosX = PosB
    2. compute, using SCS and N, which collidable C is the first collision
    3. if no collision found, DONE
    4. if movement in direction M is now blocked, then
       PosX = initial point of contact with C along N, DONE
    5. calculate PosX (and new N) assuming that there will be no more collisions
    6. remove C from SCS (assumes that you can't collide against a solid more than once per frame)
    7. go to 2
  */
  bool okflag = true;

  // if all we got was potential collisions, don't bother
  if (!_has_contact) {
    return okflag;
  }

  if (!_horizontal) {
    collide_cat.error() << "collisionHandlerFluidPusher::handle_entries is only supported in "
      "horizontal mode" << endl;
    nassertr(false, false);
  }
  
  // for every fluid mover being pushed...
  FromEntries::iterator fei;
  for (fei = _from_entries.begin(); fei != _from_entries.end(); ++fei) {
    NodePath from_node_path = fei->first;
    Entries *orig_entries = &fei->second;
    
    Colliders::iterator ci;
    ci = _colliders.find(from_node_path);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave
      // it this CollisionHandler pointer--but they didn't tell us
      // about the node.
      collide_cat.error()
        << "CollisionHandlerFluidPusher doesn't know about "
        << from_node_path << ", disabling.\n";
      okflag = false;
    } else {
      ColliderDef &def = (*ci).second;
      
      // extract the collision entries into a vector that we can safely modify
      Entries entries(*orig_entries);
      
      // make a copy of the original collision entries that we can use to re-test the collisions
      Entries SCS(*orig_entries);
      
      // currently we only support spheres as the collider
      const CollisionSphere *sphere;
      DCAST_INTO_R(sphere, entries.front()->get_from(), 0);
      
      // this is the original position delta for the entire frame, before collision response
      LPoint3f M(from_node_path.get_pos_delta(*_root));
      if (_horizontal) {
        M[2] = 0.0f;
      }
      // this is used to track position deltas every time we collide against a solid
      LPoint3f N(M);
      //collide_cat.info() << "N: " << N << endl;
      
      const LPoint3f orig_pos(from_node_path.get_pos(*_root));
      CPT(TransformState) prev_trans(from_node_path.get_prev_transform(*_root));
      const LPoint3f orig_prev_pos(prev_trans->get_pos());
      //collide_cat.info() << "orig_pos: " << orig_pos << endl;
      //collide_cat.info() << "orig_prev_pos: " << orig_prev_pos << endl;
      
      // this will hold the final calculated position at each iteration
      LPoint3f candidate_final_pos(orig_pos);
      // this holds the position before reacting to collisions
      LPoint3f uncollided_pos(candidate_final_pos);
      //collide_cat.info() << "candidate_final_pos: " << candidate_final_pos << endl;
      
      // unit vector facing back into original direction of motion
      LVector3f reverse_vec(-M);
      if (_horizontal) {
        reverse_vec[2] = 0.0f;
      }
      reverse_vec.normalize();
      //collide_cat.info() << "reverse_vec: " << reverse_vec << endl;
      
      // unit vector pointing out to the right relative to the direction of motion,
      // looking into the direction of motion
      const LVector3f right_unit(LVector3f::up().cross(reverse_vec));
      //collide_cat.info() << "right_unit: " << right_unit << endl;
      
      // if both of these become true, we're stuck in a 'corner'
      bool left_halfspace_obstructed = false;
      bool right_halfspace_obstructed = false;
      LVector3f left_halfspace_normal;
      LVector3f right_halfspace_normal;
      float left_plane_dot = 200.0f;
      float right_plane_dot = 200.0f;
      
      // iterate until the mover runs out of movement or gets stuck
      while (true) {
        const CollisionEntry *C = 0;
        // find the first (earliest) collision
        Entries::const_iterator cei;
        for (cei = entries.begin(); cei != entries.end(); ++cei) {
          const CollisionEntry *entry = (*cei);
          nassertr(entry != (CollisionEntry *)NULL, false);
          if (entry->collided() && ((C == 0) || (entry->get_t() < C->get_t()))) {
            nassertr(from_node_path == entry->get_from_node_path(), false);
            C = entry;
            break;
          }
        }
        
        // if no collisions, we're done
        if (C == 0) {
          break;
        }
        
        //collide_cat.info() << "t: " << C->get_t() << endl;
        
        // move back to initial contact point
        LPoint3f contact_point;
        LVector3f contact_normal;

        if (!C->get_all_contact_info(*_root, contact_point, contact_normal)) {
          collide_cat.warning()
            << "Cannot shove on " << from_node_path << " for collision into "
            << C->get_into_node_path() << "; no contact point/normal information.\n";
          break;
        }

        uncollided_pos = candidate_final_pos;
        candidate_final_pos[0] = contact_point[0];
        candidate_final_pos[1] = contact_point[1];
        //collide_cat.info() << "contact_point: " << contact_point << endl;
        
        LVector3f proj_surface_normal(contact_normal);
        if (_horizontal) {
          proj_surface_normal[2] = 0.0f;
        }
        //collide_cat.info() << "normal: " << contact_normal << endl;
        //collide_cat.info() << "proj_surface_normal: " << proj_surface_normal << endl;
        
        LVector3f norm_proj_surface_normal(proj_surface_normal);
        norm_proj_surface_normal.normalize();
        //collide_cat.info() << "norm_proj_surface_normal: " << norm_proj_surface_normal << endl;
        
        // check to see if we're stuck, given this collision
        float dot = right_unit.dot(norm_proj_surface_normal);
        //collide_cat.info() << "dot: " << dot << endl;
        
        if (dot > 0.0f) {
          // positive dot means plane is coming from the left (looking along original
          // direction of motion)
          if (right_halfspace_obstructed) {
            // we have obstructions from both directions, we're stuck
            break;
          }
          left_halfspace_obstructed = true;
          if (dot < left_plane_dot) {
            left_halfspace_normal = norm_proj_surface_normal;
          } else {
            // detected collision has a steeper plane wrt fwd motion than a previous collision
            // continue colliding against the shallower plane
            norm_proj_surface_normal = left_halfspace_normal;
          }
        } else {
          // negative dot means plane is coming from the right (looking along original
          // direction of motion)
          if (left_halfspace_obstructed) {
            // we have obstructions from both directions, we're stuck
            break;
          }
          right_halfspace_obstructed = true;
          dot = -dot;
          if (dot < right_plane_dot) {
            right_halfspace_normal = norm_proj_surface_normal;
          } else {
            // detected collision has a steeper plane wrt fwd motion than a previous collision
            // continue colliding against the shallower plane
            norm_proj_surface_normal = right_halfspace_normal;
          }
        }
        
        // calculate new position given that you collided with this thing
        // project the final position onto the plane of the obstruction
        LVector3f blocked_movement(uncollided_pos - contact_point);
        if (_horizontal) {
          blocked_movement[2] = 0.0f;
        }
        //collide_cat.info() << "blocked movement: " << blocked_movement << endl;
        
        candidate_final_pos += (norm_proj_surface_normal *
                                -blocked_movement.dot(norm_proj_surface_normal));
        
        // this is how the regular pusher pushes
        //candidate_final_pos += (contact_point - interior_point).length() * norm_proj_surface_normal;
        
        //collide_cat.info() << "candidate_final_pos: " << candidate_final_pos << endl;
        
        // set up new current/last positions, re-calculate collisions
        from_node_path.set_pos(*_root, candidate_final_pos);
        CPT(TransformState) prev_trans(from_node_path.get_prev_transform(*_root));
        prev_trans->set_pos(contact_point);
        from_node_path.set_prev_transform(*_root, prev_trans);
        
        // recalculate the position delta
        N = from_node_path.get_pos_delta(*_root);
        if (_horizontal) {
          N[2] = 0.0f;
        }
        //collide_cat.info() << "N: " << N << endl;
        
        // calculate new collisions given new movement vector
        entries.clear();
//        for (cei = SCS.begin(); cei != SCS.end(); ++cei) {
//          *cei->reset_collided();
//          PT(CollisionEntry) result = (*cei)->get_from()->test_intersection(**cei);
//          if (result != (CollisionEntry *)NULL) {
//            collide_cat.info() << "new collision" << endl;
//            entries.push_back(result);
//          }
//        }
      }
      
      // put things back where they were
      from_node_path.set_pos(*_root, orig_pos);
      // restore the appropriate previous position
      prev_trans = from_node_path.get_prev_transform(*_root);
      prev_trans->set_pos(orig_prev_pos);
      from_node_path.set_prev_transform(*_root, prev_trans);
      
      LVector3f net_shove(candidate_final_pos - orig_pos);
      LVector3f force_normal(net_shove);
      force_normal.normalize();
      
      //collide_cat.info() << "candidate_final_pos: " << candidate_final_pos << endl;
      //collide_cat.info() << "orig_pos: " << orig_pos << endl;
      //collide_cat.info() << "net_shove: " << net_shove << endl;
      
      // This is the part where the node actually gets moved:
      def._target.set_pos(*_root, candidate_final_pos);
      
      // We call this to allow derived classes to do other
      // fix-ups as they see fit:
      apply_net_shove(def, net_shove, force_normal);
      apply_linear_force(def, force_normal);
      
      //collide_cat.info() << endl;
    }
  }
  
  return okflag;
}
