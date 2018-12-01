/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeRegistryNode.I
 * @author drose
 * @date 2001-08-06
 */

/**
 * Returns the Python type object associated with this node.
 */
INLINE PyObject *TypeRegistryNode::
get_python_type() const {
  if (_python_type != nullptr || _parent_classes.empty()) {
    return _python_type;
  } else {
    // Recurse through parent classes.
    return r_get_python_type();
  }
}

/**
 *
 */
INLINE TypeRegistryNode::Inherit::
Inherit() {
  _top = nullptr;
  _mask = 0;
  _bits = 0;
}

/**
 *
 */
INLINE TypeRegistryNode::Inherit::
Inherit(TypeRegistryNode *top, int bit_count,
        TypeRegistryNode::SubtreeMaskType bits) {
  assert(bit_count < (int)(sizeof(SubtreeMaskType) * 8));
  _top = top;

  // Build a bitmask consisting of bit_count low-order bits.
  _mask = ((SubtreeMaskType)1 << bit_count) - 1;

  // There shouldn't be anything but zeroes after bit_count bits.
  assert((bits & ~_mask) == 0);
  _bits = bits;
}

/**
 *
 */
INLINE TypeRegistryNode::Inherit::
Inherit(const TypeRegistryNode::Inherit &copy) :
  _top(copy._top),
  _mask(copy._mask),
  _bits(copy._bits)
{
}

/**
 *
 */
INLINE void TypeRegistryNode::Inherit::
operator = (const TypeRegistryNode::Inherit &copy) {
  _top = copy._top;
  _mask = copy._mask;
  _bits = copy._bits;
}

/**
 *
 */
INLINE bool TypeRegistryNode::Inherit::
operator < (const Inherit &other) const {
  return _top < other._top;
}

/**
 * Assuming the two Inherit objects share the same subtree top, this returns
 * true if the bitmasks indicate that child inherits from base, or false
 * otherwise.
 */
INLINE bool TypeRegistryNode::Inherit::
is_derived_from(const TypeRegistryNode::Inherit &child,
                const TypeRegistryNode::Inherit &base) {
  assert(child._top == base._top);

  // Child derives from base if and only if its subtree mask contains more
  // bits (or the same number of bits), and the n low-order subtree bits that
  // are in common are identical.
  return ((child._mask & base._mask) == base._mask &&
          (child._bits & base._mask) == base._bits);
}
