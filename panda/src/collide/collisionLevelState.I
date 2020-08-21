/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLevelState.I
 * @author drose
 * @date 2007-04-05
 */

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE CollisionLevelState<MaskType>::
CollisionLevelState(const NodePath &node_path) :
  CollisionLevelStateBase(node_path),
  _current(CurrentMask::all_off())
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * This constructor goes to the next child node in the traversal.
 */
template<class MaskType>
INLINE CollisionLevelState<MaskType>::
CollisionLevelState(const CollisionLevelState<MaskType> &parent, PandaNode *child) :
  CollisionLevelStateBase(parent, child),
  _current(parent._current)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE CollisionLevelState<MaskType>::
CollisionLevelState(const CollisionLevelState<MaskType> &copy) :
  CollisionLevelStateBase(copy),
  _current(copy._current)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE void CollisionLevelState<MaskType>::
operator = (const CollisionLevelState<MaskType> &copy) {
  CollisionLevelStateBase::operator = (copy);
  _current = copy._current;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE void CollisionLevelState<MaskType>::
clear() {
  CollisionLevelStateBase::clear();
  _current.clear();
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Adds the indicated Collider to the set of Colliders in the current level
 * state.
 */
template<class MaskType>
INLINE void CollisionLevelState<MaskType>::
prepare_collider(const ColliderDef &def, const NodePath &root) {
  int index = (int)_colliders.size();
  nassertv(!CurrentMask::has_max_num_bits() ||
           index <= CurrentMask::get_max_num_bits());

  CollisionLevelStateBase::prepare_collider(def, root);
  _current.set_bit(index);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Checks the bounding volume of the current node against each of our
 * colliders.  Eliminates from the current collider list any that are outside
 * of the bounding volume.  Returns true if any colliders remain, false if all
 * of them fall outside this node's bounding volume.
 */
template<class MaskType>
bool CollisionLevelState<MaskType>::
any_in_bounds() {
#ifndef NDEBUG
  int indent_level = 0;
  if (collide_cat.is_spam()) {
    indent_level = _node_path.get_num_nodes() * 2;
    collide_cat.spam();
    indent(collide_cat.spam(false), indent_level)
      << "Considering " << _node_path.get_node_path() << "\n";
  }
#endif  // NDEBUG

  PandaNode *pnode = node();

  CPT(BoundingVolume) node_bv = pnode->get_bounds();
  if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
    const GeometricBoundingVolume *node_gbv = (const GeometricBoundingVolume *)node_bv.p();
    CollideMask this_mask = pnode->get_net_collide_mask();

    int num_colliders = get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (has_collider(c)) {
        CollisionNode *cnode = get_collider_node(c);
        bool is_in = false;

        // Don't even bother testing the bounding volume if there are no
        // collide bits in common between our collider and this node.
        CollideMask from_mask = cnode->get_from_collide_mask() & _include_mask;
        if (!(from_mask & this_mask).is_zero()) {
          // Also don't test a node with itself, or with any of its
          // descendants.
          if (pnode == cnode) {
#ifndef NDEBUG
            if (collide_cat.is_spam()) {
              indent(collide_cat.spam(false), indent_level)
                << "Not comparing " << c << " to " << _node_path
                << " (same node)\n";
            }
#endif  // NDEBUG

          } else {
            // There are bits in common, and it's not the same instance, so go
            // ahead and try the bounding volume.
            const GeometricBoundingVolume *col_gbv =
              get_local_bound(c);

            is_in = true;  // If there's no bounding volume, we're implicitly in.

            if (col_gbv != nullptr) {
              is_in = (node_gbv->contains(col_gbv) != 0);
              _node_volume_pcollector.add_level(1);

#ifndef NDEBUG
              if (collide_cat.is_spam()) {
                indent(collide_cat.spam(false), indent_level)
                  << "Comparing " << c << ": " << *col_gbv
                  << " to " << *node_gbv << ", is_in = " << is_in << "\n";
              }
#endif  // NDEBUG
            }
          }
        }

        if (!is_in) {
          // This collider cannot intersect with any geometry at this node or
          // below.
          omit_collider(c);
        }
      }
    }
  }

#ifndef NDEBUG
  if (collide_cat.is_spam()) {
    int num_active_colliders = 0;
    int num_colliders = get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (has_collider(c)) {
        num_active_colliders++;
      }
    }

    collide_cat.spam();
    indent(collide_cat.spam(false), indent_level)
      << _node_path.get_node_path() << " has " << num_active_colliders
      << " interested colliders";
    if (num_colliders != 0) {
      collide_cat.spam(false)
        << " (";
      for (int c = 0; c < num_colliders; c++) {
        if (has_collider(c)) {
          CollisionNode *cnode = get_collider_node(c);
          collide_cat.spam(false)
            << " " << c << ". " << cnode->get_name();
        }
      }
      collide_cat.spam(false)
        << " )";
    }
    collide_cat.spam(false)
      << "\n";
  }
#endif  // NDEBUG
  return has_any_collider();
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Applies the inverse transform from the current node, if any, onto all the
 * colliders in the level state.
 *
 * Returns true if the inverse transform is valid, or false if it is not valid
 * (e.g.  the transform has a scale to zero).  If the inverse transform is not
 * valid, the caller should not visit this node.
 */
template<class MaskType>
bool CollisionLevelState<MaskType>::
apply_transform() {
  // The "parent" bounds list remembers the bounds list of the previous node.
  _parent_bounds = _local_bounds;

  if (node()->is_final()) {
    // If this node has a "final" bounds, we blank out all of the from
    // bounding volumes, since we've already tested against this node's into
    // bounds, and there's no need to test any further bounding volumes at
    // this node level or below.
    BoundingVolumes new_bounds;

    int num_colliders = get_num_colliders();
    new_bounds.reserve(num_colliders);
    for (int c = 0; c < num_colliders; c++) {
      new_bounds.push_back(nullptr);
    }

    _local_bounds = new_bounds;

  } else {
    // Otherwise, in the usual case, the bounds tests will continue.
    // Recompute the bounds list of this node (if we have a transform).
    const TransformState *node_transform = node()->get_transform();
    if (!node_transform->is_identity()) {
      CPT(TransformState) inv_transform =
        node_transform->invert_compose(TransformState::make_identity());
      if (!inv_transform->has_mat()) {
        // No inverse.
        return false;
      }

      const LMatrix4 &mat = inv_transform->get_mat();

      // Now build the new bounding volumes list.
      BoundingVolumes new_bounds;

      int num_colliders = get_num_colliders();
      new_bounds.reserve(num_colliders);
      for (int c = 0; c < num_colliders; c++) {
        if (!has_collider(c) ||
            get_local_bound(c) == nullptr) {
          new_bounds.push_back(nullptr);
        } else {
          const GeometricBoundingVolume *old_bound = get_local_bound(c);
          GeometricBoundingVolume *new_bound =
            DCAST(GeometricBoundingVolume, old_bound->make_copy());
          new_bound->xform(mat);
          new_bounds.push_back(new_bound);
        }
      }

      _local_bounds = new_bounds;
    }
  }

  return true;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Returns true if there is any the maximum number of colliders that may be
 * added to the CollisionLevelStateBase at any one time.
 */
template<class MaskType>
INLINE bool CollisionLevelState<MaskType>::
has_max_colliders() {
  return CurrentMask::has_max_num_bits();
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Returns the maximum number of colliders that may be added to the
 * CollisionLevelStateBase at any one time.
 */
template<class MaskType>
INLINE int CollisionLevelState<MaskType>::
get_max_colliders() {
  return CurrentMask::get_max_num_bits();
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Returns true if the nth collider in the LevelState is still part of the
 * level.
 */
template<class MaskType>
INLINE bool CollisionLevelState<MaskType>::
has_collider(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), false);
  return (_current.get_bit(n));
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE bool CollisionLevelState<MaskType>::
has_any_collider() const {
  return !_current.is_zero();
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class MaskType>
INLINE void CollisionLevelState<MaskType>::
omit_collider(int n) {
  nassertv(n >= 0 && n < (int)_colliders.size());
  nassertv(has_collider(n));

  _current.clear_bit(n);
}
#endif  // CPPPARSER
