/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeRegistryNode.cxx
 * @author drose
 * @date 2001-08-06
 */

#include "typeRegistryNode.h"

#include <algorithm>
#include <string.h>

bool TypeRegistryNode::_paranoid_inheritance = false;

/**
 *
 */
TypeRegistryNode::
TypeRegistryNode(TypeHandle handle, const std::string &name, TypeHandle &ref) :
  _handle(handle), _name(name), _ref(ref)
{
  clear_subtree();
  memset(_memory_usage, 0, sizeof(_memory_usage));
}

/**
 * Returns true if the child RegistryNode represents a class that inherits
 * directly or indirectly from the class represented by the base RegistryNode.
 */
bool TypeRegistryNode::
is_derived_from(const TypeRegistryNode *child, const TypeRegistryNode *base) {
  // This function is the basis for TypedObject::is_of_type(), which gets used
  // quite frequently within Panda, often in inner-loop code.  Therefore, we
  // go through some pains to make this function as efficient as possible.

  // First, compare the subtree tops.  If they are the same, then this node
  // and the base node are within the same single-inheritance subtree, and we
  // can use our bitmask trick to determine the relationship with no
  // additional work.  (See r_build_subtrees()).

  if (child->_inherit._top == base->_inherit._top) {
    assert(child->_inherit._top != nullptr);

    bool derives =
      Inherit::is_derived_from(child->_inherit, base->_inherit);

#ifndef NDEBUG
    if (_paranoid_inheritance) {
      bool paranoid_derives = check_derived_from(child, base);
      if (derives != paranoid_derives) {
        std::cerr
          << "Inheritance test for " << child->_name
          << " from " << base->_name << " failed!\n"
          << "Result: " << derives << " should have been: "
          << paranoid_derives << "\n"
          << "Classes are in the same single inheritance subtree, children of "
          << child->_inherit._top->_name << "\n"
          << std::hex
          << child->_name << " has mask " << child->_inherit._mask
          << " and bits " << child->_inherit._bits << "\n"
          << base->_name << " has mask " << base->_inherit._mask
          << " and bits " << base->_inherit._bits << "\n"
          << std::dec;
        return paranoid_derives;
      }
    }
#endif

    /*
    cerr << "trivial: " << child->_name << " vs. " << base->_name
         << " (" << child->_inherit._top->_name << ") = " << derives << "\n";
    */

    return derives;
  }

  // The two nodes are not within the same single-inheritance subtree.  This
  // complicates things a bit.

  // First, we should check whether the subtree tops of the two nodes inherit
  // from each other.
  TypeRegistryNode *child_top = child->_inherit._top;
  TypeRegistryNode *base_top = base->_inherit._top;

  bool derives = false;

  // If child_top does not inherit from base_top, it follows that child does
  // not inherit from base.
  TopInheritance::const_iterator ti =
    lower_bound(child_top->_top_inheritance.begin(),
                child_top->_top_inheritance.end(),
                Inherit(base_top, 0, 0));

  while (ti != child_top->_top_inheritance.end() &&
         (*ti)._top == base_top &&
         !derives) {
    // If child_top *does* inherit from base_top, then child may or may not
    // inherit from base.  This depends on the exact path of inheritance.
    // Since there might be multiple paths from child_top to base_top, we have
    // to examine all of them.
    const Inherit &connection = (*ti);

    // Here is one inheritance from child_top to base_top.  If the connecting
    // node inherits from base, then child also inherits from base.  If the
    // connecting node does not inherit from base, we must keep looking.
    derives = Inherit::is_derived_from(connection, base->_inherit);

    ++ti;
  }

#ifndef NDEBUG
  if (_paranoid_inheritance) {
    bool paranoid_derives = check_derived_from(child, base);
    if (derives != paranoid_derives) {
      std::cerr
        << "Inheritance test for " << child->_name
        << " from " << base->_name << " failed!\n"
        << "Result: " << derives << " should have been: "
        << paranoid_derives << "\n"
        << child->_name << " is a descendent of "
        << child_top->_name << "\n"
        << base->_name << " is a descendent of "
        << base_top->_name << "\n";
      return paranoid_derives;
    }
  }
#endif

  /*
  cerr << "complex: " << child->_name << " (" << child->_inherit._top->_name
       << ") vs. " << base->_name << " (" << base->_inherit._top->_name
       << ") = " << derives << "\n";
  */
  return derives;
}

/**
 * Returns the first parent class of child that is a descendant of the
 * indicated base class.
 */
TypeHandle TypeRegistryNode::
get_parent_towards(const TypeRegistryNode *child,
                   const TypeRegistryNode *base) {
  if (child == base) {
    return child->_handle;
  }

  Classes::const_iterator ni;
  for (ni = child->_parent_classes.begin();
       ni != child->_parent_classes.end(); ++ni) {
    if (is_derived_from((*ni), base)) {
      return (*ni)->_handle;
    }
  }

  return TypeHandle::none();
}


/**
 * Removes any subtree definition previously set up via define_subtree(), in
 * preparation for rebuilding the subtree data.
 */
void TypeRegistryNode::
clear_subtree() {
  _inherit = Inherit();
  _top_inheritance.clear();
  _visit_count = 0;
}

/**
 * Indicates that this TypeRegistryNode is the top of a subtree within the
 * inheritance graph (typically, this indicates a multiple-inheritance node).
 * Builds all the subtree_mask etc.  flags for nodes at this level and below.
 */
void TypeRegistryNode::
define_subtree() {
  // cerr << "Building subtree for " << _name << ", top inheritance is:\n";

    /*
  TopInheritance::const_iterator ti;
  for (ti = _top_inheritance.begin(); ti != _top_inheritance.end(); ++ti) {
    const Inherit &t = (*ti);
    cerr << "  from " << t._top->_name << " via "
         << hex << t._bits << " / " << t._mask << dec << "\n";
  }
    */

  r_build_subtrees(this, 0, 0);
}

/**
 * Recursively builds up all the subtree cache information for this node and
 * the ones below.  This information is used to quickly determine class
 * inheritance.
 */
void TypeRegistryNode::
r_build_subtrees(TypeRegistryNode *top, int bit_count,
                 TypeRegistryNode::SubtreeMaskType bits) {
  // The idea with these bits is to optimize the common case of a single-
  // inheritance graph (that is, an inheritance tree), or a single-inheritance
  // subgraph of the full multiple-inheritance graph (i.e.  a subtree of the
  // inheritance graph).

/*
 * When we have just single inheritance, we can define a unique number for
 * each node in the inheritance tree that allows us to immediately determine
 * the inheritance relationship between any two nodes in the tree.  We choose
 * a number such that for a given node whose number has n bits, each child
 * node has m + n bits where the low-order n bits are the same as the parent
 * node's bits, and the high-order m bits are unique among each sibling.  The
 * node at the top of the tree has zero bits.
 */

  // That way, we can simply compare bitmasks to determine if class A inherits
  // from class B.  If the low-order bits are the same, they have some
  // ancestry in common.  The highest-order bit that still matches corresponds
  // to the lowest node in the tree that they have in common; i.e.  the node
  // from which they both inherit.

  // To put it more formally, let count(A) be the number of bits in A's
  // number, and count(B) be the number of bits in B's number.  A inherits
  // from B if and only if count(B) <= count(A), and the lower count(B) bits
  // of A's number are the same as those in B's number.

  // This algorithm breaks down in the presence of multiple inheritance, since
  // we can't make up a single number for each node any more.  We still take
  // advantage of the algorithm by considering each single-inheritance
  // subgraph separately.

  // To handle multiple inheritance, we reset the numbers to zero every time
  // we come across a multiple-inheritance node (this begins a new subtree).
  // There are relatively few of these "subtree top" nodes, and we record the
  // explicit inheritance of each one from all of its ancestor "subtree top"
  // nodes within the node itself.

  if (top != this && _parent_classes.size() != 1) {
    assert(!_parent_classes.empty());

    // This class multiply inherits; it therefore begins a new subtree.

    // Copy in the inheritance relations from our parent subtree tops.
    _top_inheritance.insert(_top_inheritance.end(),
                            top->_top_inheritance.begin(),
                            top->_top_inheritance.end());
    _top_inheritance.push_back(Inherit(top, bit_count, bits));

    _visit_count++;
    if (_visit_count == (int)_parent_classes.size()) {
      // This is the last time we'll visit this node, so continue the
      // recursion now.
      assert(_inherit._top == nullptr);
      sort(_top_inheritance.begin(), _top_inheritance.end());
      define_subtree();
    }

  } else {
    // This class singly inherits, so this had better be the only time this
    // function is called on it since clear_subtree().
    assert(_inherit._top == nullptr);

    assert(bit_count < (int)(sizeof(SubtreeMaskType) * 8));

    _inherit = Inherit(top, bit_count, bits);

    // Now, how many more bits do we need to encode each of our children?
    int num_children = (int)_child_classes.size();
    int more_bits = 0;
    int i = num_children - 1;
    while (i > 0) {
      more_bits++;
      i >>= 1;
    }

    // We need at least one bit, even if there is only one child, so we can
    // differentiate parent from child.
    more_bits = std::max(more_bits, 1);

    assert(more_bits < (int)(sizeof(SubtreeMaskType) * 8));

    if (bit_count + more_bits > (int)(sizeof(SubtreeMaskType) * 8)) {
      // Too many bits; we need to start a new subtree right here.  This node
      // becomes a subtree top node, even though it's not a multiple-
      // inheritance node.
      assert(top != this);
      _top_inheritance = top->_top_inheritance;
      _top_inheritance.push_back(_inherit);
      sort(_top_inheritance.begin(), _top_inheritance.end());
      _inherit = Inherit();
      define_subtree();

    } else {
      // Still plenty of bits, so keep going.
      for (i = 0; i < num_children; i++) {
        TypeRegistryNode *child = _child_classes[i];
        SubtreeMaskType next_bits = ((SubtreeMaskType)i << bit_count);

        child->r_build_subtrees(top, bit_count + more_bits,
                                bits | next_bits);
      }
    }
  }
}

/**
 * Recurses through the parent nodes to find the best Python type object to
 * represent objects of this type.
 */
PyObject *TypeRegistryNode::
r_get_python_type() const {
  Classes::const_iterator ni;
  for (ni = _parent_classes.begin(); ni != _parent_classes.end(); ++ni) {
    const TypeRegistryNode *parent = *ni;
    if (parent->_python_type != nullptr) {
      return parent->_python_type;

    } else if (!parent->_parent_classes.empty()) {
      PyObject *py_type = parent->r_get_python_type();
      if (py_type != nullptr) {
        return py_type;
      }
    }
  }

  return nullptr;
}

/**
 * A recursive function to double-check the result of is_derived_from().  This
 * is the slow, examine-the-whole-graph approach, as opposed to the clever and
 * optimal algorithm of is_derived_from(); it's intended to be used only for
 * debugging said clever algorithm.
 */
bool TypeRegistryNode::
check_derived_from(const TypeRegistryNode *child,
                   const TypeRegistryNode *base) {
  if (child == base) {
    return true;
  }

  Classes::const_iterator ni;
  for (ni = child->_parent_classes.begin();
       ni != child->_parent_classes.end();
       ++ni) {
    if (check_derived_from(*ni, base)) {
      return true;
    }
  }

  return false;
}
