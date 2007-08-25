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
    _has_contact = true;
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
  
  if (!_horizontal) {
    collide_cat.error() << "collisionHandlerFluidPusher::handle_entries is only supported in "
      "horizontal mode" << endl;
    nassertr(false, false);
  }
  
  // for every fluid mover being pushed...
  FromEntries::iterator fei;
  for (fei = _from_entries.begin(); fei != _from_entries.end(); ++fei) {
    NodePath from_node_path = fei->first;
    Entries *entries = &fei->second;
    
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
      Entries entries(*entries);
      Entries next_entries;
      
      // extract out the initial set of collision solids
      CollisionSolids SCS;
      
      Entries::iterator ei;
      for (ei = entries.begin(); ei != entries.end(); ++ei) {
        SCS.push_back((*ei)->get_into());
      }
      
      // currently we only support spheres as the collider
      const CollisionSphere *sphere;
      DCAST_INTO_R(sphere, (*entries.front()).get_from(), 0);
      // use a slightly larger radius value so that when we move along
      // collision planes we don't re-collide
      float sphere_radius = sphere->get_radius() * 1.001;
      
      // make a copy of the original from_nodepath that we can mess with
      // in the process of calculating the final position
      _from_node_path_copy = from_node_path.copy_to(from_node_path.get_parent());
      
      LPoint3f N(from_node_path.get_pos_delta(*_root));
      if (_horizontal) {
        N[2] = 0.0f;
      }
      const LPoint3f orig_pos(from_node_path.get_pos(*_root));
      // this will hold the final calculated position
      LPoint3f PosX(orig_pos);
      
      // unit vector facing back into original direction of motion
      LVector3f reverse_vec(-N);
      if (_horizontal) {
        reverse_vec[2] = 0.0f;
      }
      reverse_vec.normalize();
      
      // unit vector pointing out to the right relative to the direction of motion,
      // looking into the direction of motion
      const LVector3f right_unit(LVector3f::up().cross(reverse_vec));
      
      // if both of these become true, we're stuck in a 'corner'
      bool left_halfspace_obstructed = false;
      bool right_halfspace_obstructed = false;
      float left_plane_dot = 200.0f;
      float right_plane_dot = 200.0f;
      
      // iterate until the mover runs out of movement or gets stuck
      while (true) {
        CollisionEntry *C = 0;
        // find the first (earliest) collision
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          CollisionEntry *entry = (*ei);
          nassertr(entry != (CollisionEntry *)NULL, false);
          if ((C == 0) || (entry->get_t() < C->get_t())) {
            nassertr(from_node_path == entry->get_from_node_path(), false);
            C = entry;
            break;
          }
        }
        
        // if no collisions, we're done
        if (C == 0) {
          break;
        }
        
        // calculate point of collision, move back to it
        nassertr(C->has_surface_point(), true);
        nassertr(C->has_surface_normal(), true);
        nassertr(C->has_interior_point(), true);
        LVector3f surface_normal = C->get_surface_normal(*_root);
        if (_horizontal) {
          surface_normal[2] = 0.0f;
        }
        surface_normal.normalize();
        collide_cat.info() << "normal: " << surface_normal << endl;
        PosX = C->get_surface_point(*_root) + (sphere_radius * surface_normal);
        
        // check to see if we're stuck, given this collision
        float dot = right_unit.dot(surface_normal);
        if (dot > 0.0f) {
          // positive dot means plane is coming from the left (looking along original
          // direction of motion)
          if (dot < left_plane_dot) {
            if (right_halfspace_obstructed) {
              // we have obstructions from both directions, we're stuck
              break;
            }
            left_halfspace_obstructed = true;
          }
        } else {
          // negative dot means plane is coming from the right (looking along original
          // direction of motion)
          dot = -dot;
          if (dot < right_plane_dot) {
            if (left_halfspace_obstructed) {
              // we have obstructions from both directions, we're stuck
              break;
            }
            right_halfspace_obstructed = true;
          }
        }
        
        // set up new current/last positions, re-calculate collisions
        CPT(TransformState) prev_trans(_from_node_path_copy.get_prev_transform(*_root));
        prev_trans->set_pos(_from_node_path_copy.get_pos(*_root));
        _from_node_path_copy.set_prev_transform(*_root, prev_trans);
        _from_node_path_copy.set_pos(PosX);
        
        // calculate new collisions given new movement vector
        CollisionEntry new_entry;
        new_entry._from_node_path = _from_node_path_copy;
        new_entry._from = sphere;
        next_entries.clear();
        CollisionSolids::iterator csi;
        for (csi = SCS.begin(); csi != SCS.end(); ++csi) {
          PT(CollisionEntry) result = (*csi)->test_intersection_from_sphere(new_entry);
          if (result != (CollisionEntry *)NULL) {
            next_entries.push_back(result);
          }
        }
        // swap in the new set of collision events
        entries.swap(next_entries);
      }
      
      LVector3f net_shove(PosX - orig_pos);
      if (_horizontal) {
        net_shove[2] = 0.0f;
      }
      LVector3f force_normal(net_shove);
      force_normal.normalize();
      
      collide_cat.info() << "PosX: " << PosX << endl;
      collide_cat.info() << "orig_pos: " << orig_pos << endl;
      collide_cat.info() << "net_shove: " << net_shove << endl;
      collide_cat.info() << endl;

      
      // This is the part where the node actually gets moved:
      CPT(TransformState) trans = def._target.get_transform(*_root);
      LVecBase3f pos = trans->get_pos();
      pos += net_shove * trans->get_mat();
      def._target.set_transform(*_root, trans->set_pos(pos));
      def.updated_transform();
      
      // We call this to allow derived classes to do other
      // fix-ups as they see fit:
      apply_net_shove(def, net_shove, force_normal);
      apply_linear_force(def, force_normal);
    }
  }
  
  return okflag;
}
