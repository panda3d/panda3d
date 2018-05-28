/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPusher.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandlerPusher.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "config_collide.h"
#include "dcast.h"
#include "epvector.h"

TypeHandle CollisionHandlerPusher::_type_handle;

/**
 * The ShoveData class is used within
 * CollisionHandlerPusher::handle_entries(), to track multiple shoves onto a
 * given collider.  It's not exported outside this file.
 */
class ShoveData {
public:
  LVector3 _vector;
  PN_stdfloat _length;
  bool _valid;
  CollisionEntry *_entry;
};

/**
 *
 */
CollisionHandlerPusher::
CollisionHandlerPusher() {
  _horizontal = pushers_horizontal;
}

/**
 *
 */
CollisionHandlerPusher::
~CollisionHandlerPusher() {
}

/**
 * Called by the parent class after all collisions have been detected, this
 * manages the various collisions and moves around the nodes as necessary.
 *
 * The return value is normally true, but it may be false to indicate the
 * CollisionTraverser should disable this handler from being called in the
 * future.
 */
bool CollisionHandlerPusher::
handle_entries() {
  bool okflag = true;

  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    const NodePath &from_node_path = (*fi).first;
    const Entries &entries = (*fi).second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node_path);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave it this
      // CollisionHandler pointer--but they didn't tell us about the node.
      collide_cat.error()
        << "CollisionHandlerPusher doesn't know about "
        << from_node_path << ", disabling.\n";
      okflag = false;
    } else {
      ColliderDef &def = (*ci).second;
      {
        // How to apply multiple shoves from different solids onto the same
        // collider?  One's first intuition is to vector sum all the shoves.
        // However, this causes problems when two parallel walls shove on the
        // collider, because we end up with a double shove.  We hack around
        // this by testing if two shove vectors share nearly the same
        // direction, and if so, we keep only the longer of the two.

        typedef epvector<ShoveData> Shoves;
        Shoves shoves;

        Entries::const_iterator ei;
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          CollisionEntry *entry = (*ei);
          nassertr(entry != nullptr, false);
          nassertr(from_node_path == entry->get_from_node_path(), false);

          LPoint3 surface_point;
          LVector3 normal;
          LPoint3 interior_point;

          if (!entry->get_all(def._target, surface_point, normal, interior_point)) {
#ifndef NDEBUG
            if (collide_cat.is_debug()) {
              collide_cat.debug()
                << "Cannot shove on " << from_node_path << " for collision into "
                << entry->get_into_node_path() << "; no normal/depth information.\n";
            }
#endif
          } else {
            // Shove it just enough to clear the volume.
            if (!surface_point.almost_equal(interior_point)) {
              if (_horizontal) {
                normal[2] = 0.0f;
              }
              // Just to be on the safe size, we normalize the normal vector,
              // even though it really ought to be unit-length already (unless
              // we just forced it horizontal, above).
              normal.normalize();

              ShoveData sd;
              sd._vector = normal;
              sd._length = (surface_point - interior_point).length();
              sd._valid = true;
              sd._entry = entry;

              #ifndef NDEBUG
              if (collide_cat.is_debug()) {
                collide_cat.debug()
                  << "Shove on " << from_node_path << " from "
                  << entry->get_into_node_path() << ": " << sd._vector
                  << " times " << sd._length << "\n";
              }
              #endif

              shoves.push_back(sd);
            }
          }
        }

        if (!shoves.empty()) {
          // Now we look for two shoves that are largely in the same
          // direction, so we can combine them into a single shove of the same
          // magnitude; we also check for two shoves at 90 degrees, so we can
          // detect whether we are hitting an inner or an outer corner.

          Shoves::iterator si;
          for (si = shoves.begin(); si != shoves.end(); ++si) {
            ShoveData &sd = (*si);
            Shoves::iterator sj;
            for (sj = shoves.begin(); sj != si; ++sj) {
              ShoveData &sd2 = (*sj);
              if (sd2._valid) {
                PN_stdfloat d = sd._vector.dot(sd2._vector);
                if (collide_cat.is_debug()) {
                  collide_cat.debug()
                    << "Considering dot product " << d << "\n";
                }

                if (d > 0.9) {
                  // These two shoves are largely in the same direction; save
                  // the larger of the two.
                  if (sd2._length < sd._length) {
                    sd2._valid = false;
                  } else {
                    sd._valid = false;
                  }
                } else {
                  // These two shoves are not in the same direction.  If they
                  // are both from polygons that are a child of the same node,
                  // try to determine the shape of the corner (convex or
                  // concave).
                  const CollisionSolid *s1 = sd._entry->get_into();
                  const CollisionSolid *s2 = sd2._entry->get_into();
                  if (s1 != nullptr &&
                      s2 != nullptr &&
                      s1->is_exact_type(CollisionPolygon::get_class_type()) &&
                      s2->is_exact_type(CollisionPolygon::get_class_type()) &&
                      sd._entry->get_into_node_path() ==
                      sd2._entry->get_into_node_path()) {
                    const CollisionPolygon *p1 = DCAST(CollisionPolygon, s1);
                    const CollisionPolygon *p2 = DCAST(CollisionPolygon, s2);
                    if (p1->dist_to_plane(p2->get_collision_origin()) < 0 &&
                        p2->dist_to_plane(p1->get_collision_origin()) < 0) {
                      // Each polygon is behind the other one.  That means we
                      // have a convex corner, and therefore we should discard
                      // one of the shoves (or the user will get stuck coming
                      // at a convex corner).
                      if (collide_cat.is_debug()) {
                        collide_cat.debug()
                          << "Discarding shove from convex corner.\n";
                      }

                      // This time, unlike the case of two parallel walls
                      // above, we discard the larger of the two shoves, not
                      // the smaller.  This is because as we slide off the
                      // convex corner, the wall we are sliding away from will
                      // get a bigger and bigger shove--and we need to keep
                      // ignoring the same wall as we slide.
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
          LVector3 net_shove(0.0f, 0.0f, 0.0f);
          LVector3 force_normal(0.0f, 0.0f, 0.0f);
          for (si = shoves.begin(); si != shoves.end(); ++si) {
            const ShoveData &sd = (*si);
            if (sd._valid) {
              net_shove += sd._vector * sd._length;
              force_normal += sd._vector;
            }
          }

          #ifndef NDEBUG
          if (collide_cat.is_debug()) {
            collide_cat.debug()
              << "Net shove on " << from_node_path << " is: "
              << net_shove << "\n";
          }
          #endif

          // This is the part where the node actually gets moved:
          CPT(TransformState) trans = def._target.get_transform();
          LVecBase3 pos = trans->get_pos();
          pos += net_shove * trans->get_mat();
          def._target.set_transform(trans->set_pos(pos));
          def.updated_transform();

          // We call this to allow derived classes to do other fix-ups as they
          // see fit:
          apply_net_shove(def, net_shove, force_normal);
          apply_linear_force(def, force_normal);
        }
      }
    }
  }

  return okflag;
}

/**
 * This is an optional hook for derived classes to do some work with the
 * ColliderDef and the force vector.
 */
void CollisionHandlerPusher::
apply_net_shove(ColliderDef &def, const LVector3 &net_shove,
    const LVector3 &force_normal) {
}

/**
 * This is an optional hook for derived classes to do some work with the
 * ColliderDef and the force vector.
 */
void CollisionHandlerPusher::
apply_linear_force(ColliderDef &def, const LVector3 &force_normal) {
}
