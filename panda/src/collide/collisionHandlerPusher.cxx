// Filename: collisionHandlerPusher.cxx
// Created by:  drose (25Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "collisionHandlerPusher.h"
#include "config_collide.h"

TypeHandle CollisionHandlerPusher::_type_handle;

///////////////////////////////////////////////////////////////////
//       Class : ShoveData
// Description : The ShoveData class is used within
//               CollisionHandlerPusher::handle_entries(), to track
//               multiple shoves onto a given collider.  It's not
//               exported outside this file.
////////////////////////////////////////////////////////////////////
class ShoveData {
public:
  LVector3f _shove;
  float _length;
  LVector3f _normalized_shove;
  bool _valid;
};

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

      // How to apply multiple shoves from different solids onto the
      // same collider?  One's first intuition is to vector sum all
      // the shoves.  However, this causes problems when two parallel
      // walls shove on the collider, because we end up with a double
      // shove.  We hack around this by testing if two shove vectors
      // share nearly the same direction, and if so, we keep only the
      // longer of the two.

      typedef vector<ShoveData> Shoves;
      Shoves shoves;

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
          if (entry->get_into_depth() != 0.0) {
            ShoveData sd;
            sd._shove =
              entry->get_into_surface_normal() *
              entry->get_into_depth();

            if (collide_cat.is_debug()) {
              collide_cat.debug()
                << "Shove on " << *from_node << " from "
                << *entry->get_into_node() << ": " << sd._shove << "\n";
            }

            sd._length = sd._shove.length();
            sd._normalized_shove = sd._shove / sd._length;
            sd._valid = true;

            shoves.push_back(sd);
          }
        }
      }

      if (!shoves.empty()) {
        // Now we combine any two shoves that shove in largely the
        // same direction.  Hacky.

        Shoves::iterator si;
        for (si = shoves.begin(); si != shoves.end(); ++si) {
          ShoveData &sd = (*si);
          Shoves::iterator sj;
          for (sj = shoves.begin(); sj != si; ++sj) {
            ShoveData &sd2 = (*sj);
            if (sd2._valid) {

              float d = sd._normalized_shove.dot(sd2._normalized_shove);
              if (collide_cat.is_debug()) {
                collide_cat.debug()
                  << "Considering dot product " << d << "\n";
              }

              if (d > 0.9) {
                // These two shoves are largely in the same direction;
                // save the larger of the two.
                if (sd2._length < sd._length) {
                  sd2._valid = false;
                } else {
                  sd._valid = false;
                }
              }
            }
          }
        }

        // Now we can determine the net shove.
        LVector3f net_shove(0.0, 0.0, 0.0);
        for (si = shoves.begin(); si != shoves.end(); ++si) {
          const ShoveData &sd = (*si);
          if (sd._valid) {
            net_shove += sd._shove;
          }
        }

        if (_horizontal) {
          net_shove[2] = 0.0;
        }

        if (collide_cat.is_debug()) {
          collide_cat.debug()
            << "Net shove on " << *from_node << " is: "
            << net_shove << "\n";
        }

        LMatrix4f mat;
        def.get_mat(mat);
        def.set_mat(LMatrix4f::translate_mat(net_shove) * mat);
      }
    }
  }
}
