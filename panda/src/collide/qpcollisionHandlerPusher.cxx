// Filename: qpcollisionHandlerPusher.cxx
// Created by:  drose (16Mar02)
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

#include "qpcollisionHandlerPusher.h"
#include "qpcollisionNode.h"
#include "qpcollisionEntry.h"
#include "config_collide.h"

TypeHandle qpCollisionHandlerPusher::_type_handle;

///////////////////////////////////////////////////////////////////
//       Class : qpShoveData
// Description : The qpShoveData class is used within
//               qpCollisionHandlerPusher::handle_entries(), to track
//               multiple shoves onto a given collider.  It's not
//               exported outside this file.
////////////////////////////////////////////////////////////////////
class qpShoveData {
public:
  LVector3f _shove;
  float _length;
  LVector3f _normalized_shove;
  bool _valid;
};

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPusher::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpCollisionHandlerPusher::
qpCollisionHandlerPusher() {
  _horizontal = true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPusher::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpCollisionHandlerPusher::
~qpCollisionHandlerPusher() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPusher::handle_entries
//       Access: Protected, Virtual
//  Description: Called by the parent class after all collisions have
//               been detected, this manages the various collisions
//               and moves around the nodes as necessary.
//
//               The return value is normally true, but it may be
//               false to indicate the CollisionTraverser should
//               disable this handler from being called in the future.
////////////////////////////////////////////////////////////////////
bool qpCollisionHandlerPusher::
handle_entries() {
  bool okflag = true;

  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    qpCollisionNode *from_node = (*fi).first;
    nassertr(from_node != (qpCollisionNode *)NULL, false);
    const Entries &entries = (*fi).second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node);
    if (ci == _colliders.end()) {
      // Hmm, someone added a qpCollisionNode to a traverser and gave
      // it this qpCollisionHandler pointer--but they didn't tell us
      // about the node.
      collide_cat.error()
        << "qpCollisionHandlerPusher doesn't know about "
        << *from_node << ", disabling.\n";
      okflag = false;

    } else {
      ColliderDef &def = (*ci).second;
      if (!def.is_valid()) {
        collide_cat.error()
          << "Removing invalid collider " << *from_node << " from "
          << get_type() << "\n";
        _colliders.erase(ci);

      } else {
        // How to apply multiple shoves from different solids onto the
        // same collider?  One's first intuition is to vector sum all
        // the shoves.  However, this causes problems when two parallel
        // walls shove on the collider, because we end up with a double
        // shove.  We hack around this by testing if two shove vectors
        // share nearly the same direction, and if so, we keep only the
        // longer of the two.
        
        typedef pvector<qpShoveData> Shoves;
        Shoves shoves;
        
        Entries::const_iterator ei;
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          qpCollisionEntry *entry = (*ei);
          nassertr(entry != (qpCollisionEntry *)NULL, false);
          nassertr(from_node == entry->get_from_node(), false);
          
          if (!entry->has_into_surface_normal() ||
              !entry->has_into_depth()) {
            if (collide_cat.is_debug()) {
              collide_cat.debug()
                << "Cannot shove on " << *from_node << " for collision into "
                << *entry->get_into_node() << "; no normal/depth information.\n";
            }
            
          } else {
            // Shove it just enough to clear the volume.
            if (entry->get_into_depth() != 0.0f) {
              qpShoveData sd;
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
            qpShoveData &sd = (*si);
            Shoves::iterator sj;
            for (sj = shoves.begin(); sj != si; ++sj) {
              qpShoveData &sd2 = (*sj);
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
          LVector3f net_shove(0.0f, 0.0f, 0.0f);
          for (si = shoves.begin(); si != shoves.end(); ++si) {
            const qpShoveData &sd = (*si);
            if (sd._valid) {
              net_shove += sd._shove;
            }
          }
          
          if (_horizontal) {
            net_shove[2] = 0.0f;
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

  return okflag;
}
