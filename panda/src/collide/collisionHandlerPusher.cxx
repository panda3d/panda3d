// Filename: collisionHandlerPusher.cxx
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

#include "collisionHandlerPusher.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "config_collide.h"
#include "dcast.h"

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
  LVector3f _vector;
  float _length;
  bool _valid;
  CollisionEntry *_entry;
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
//
//               The return value is normally true, but it may be
//               false to indicate the CollisionTraverser should
//               disable this handler from being called in the future.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPusher::
handle_entries() {
  bool okflag = true;

  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    CollisionNode *from_node = (*fi).first;
    nassertr(from_node != (CollisionNode *)NULL, false);
    const Entries &entries = (*fi).second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave
      // it this CollisionHandler pointer--but they didn't tell us
      // about the node.
      collide_cat.error()
        << "CollisionHandlerPusher doesn't know about "
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
        
        typedef pvector<ShoveData> Shoves;
        Shoves shoves;

        Entries::const_iterator ei;
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          CollisionEntry *entry = (*ei);
          nassertr(entry != (CollisionEntry *)NULL, false);
          nassertr(from_node == entry->get_from_node(), false);
          
          if (!entry->has_into_surface_normal() ||
              !entry->has_into_depth()) {
#ifndef NDEBUG          
            if (collide_cat.is_debug()) {
              collide_cat.debug()
                << "Cannot shove on " << *from_node << " for collision into "
                << *entry->get_into_node() << "; no normal/depth information.\n";
            }
#endif
            
          } else {
            // Shove it just enough to clear the volume.
            if (entry->get_into_depth() != 0.0f) {
              LVector3f normal = entry->get_into_surface_normal();
              if (_horizontal) {
                normal[2] = 0.0f;
              }
              // Just to be on the safe size, we normalize the normal
              // vector, even though it really ought to be unit-length
              // already (unless we just forced it horizontal, above).
              normal.normalize();

              ShoveData sd;
              sd._vector = normal;
              sd._length = entry->get_into_depth();
              sd._valid = true;
              sd._entry = entry;
              
#ifndef NDEBUG          
              if (collide_cat.is_debug()) {
                collide_cat.debug()
                  << "Shove on " << *from_node << " from "
                  << *entry->get_into_node() << ": " << sd._vector
                  << " times " << sd._length << "\n";
              }
#endif
              
              shoves.push_back(sd);
            }
          }
        }
        
        if (!shoves.empty()) {
          // Now we look for two shoves that are largely in the same
          // direction, so we can combine them into a single shove of
          // the same magnitude; we also check for two shoves at 90
          // degrees, so we can detect whether we are hitting an inner
          // or an outer corner.

          Shoves::iterator si;
          for (si = shoves.begin(); si != shoves.end(); ++si) {
            ShoveData &sd = (*si);
            Shoves::iterator sj;
            for (sj = shoves.begin(); sj != si; ++sj) {
              ShoveData &sd2 = (*sj);
              if (sd2._valid) {
                
                float d = sd._vector.dot(sd2._vector);
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

                } else if (fabs(d) < 0.1) {
                  // These two shoves are largely at 90 degress to
                  // each other.  If they are both from polygons
                  // that are a child of the same node, try to
                  // determine the shape of the corner (convex or
                  // concave).
                  const CollisionSolid *s1 = sd._entry->get_into();
                  const CollisionSolid *s2 = sd2._entry->get_into();
                  if (s1 != (CollisionSolid *)NULL &&
                      s2 != (CollisionSolid *)NULL &&
                      s1->is_exact_type(CollisionPolygon::get_class_type()) &&
                      s2->is_exact_type(CollisionPolygon::get_class_type()) &&
                      sd._entry->get_into_node_path() ==
                      sd2._entry->get_into_node_path()) {
                    const CollisionPolygon *p1 = DCAST(CollisionPolygon, s1);
                    const CollisionPolygon *p2 = DCAST(CollisionPolygon, s2);
                    if (p1->dist_to_plane(p2->get_collision_origin()) < 0 &&
                        p2->dist_to_plane(p1->get_collision_origin()) < 0) {
                      // Each polygon is behind the other one.  That
                      // means we have a convex corner, and therefore
                      // we should discard one of the shoves (or the
                      // user will get stuck coming at a convex
                      // corner).
                      if (collide_cat.is_debug()) {
                        collide_cat.debug()
                          << "Discarding shove from convex corner.\n";
                      }

                      // This time, unlike the case of two parallel
                      // walls above, we discard the larger of the two
                      // shoves, not the smaller.  This is because as
                      // we slide off the convex corner, the wall we
                      // are sliding away from will get a bigger and
                      // bigger shove--and we need to keep ignoring
                      // the same wall as we slide.
                      if (sd2._length < sd._length) {
                        sd._valid = false;
                      } else {
                        sd2._valid = false;
                      }
                    }
                  }
                }
              }
            }
          }
          
          // Now we can determine the net shove.
          LVector3f net_shove(0.0f, 0.0f, 0.0f);
          for (si = shoves.begin(); si != shoves.end(); ++si) {
            const ShoveData &sd = (*si);
            if (sd._valid) {
              net_shove += sd._vector * sd._length;
            }
          }

#ifndef NDEBUG          
          if (collide_cat.is_debug()) {
            collide_cat.debug()
              << "Net shove on " << *from_node << " is: "
              << net_shove << "\n";
          }
#endif
          
          LMatrix4f mat;
          def.get_mat(mat);
          def.set_mat(LMatrix4f::translate_mat(net_shove) * mat);
        }
      }
    }
  }

  return okflag;
}
