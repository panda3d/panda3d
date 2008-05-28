// Filename: typeRegistryNode.I
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeRegistryNode::Inherit::
Inherit() {
  _top = (TypeRegistryNode *)NULL;
  _mask = 0;
  _bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeRegistryNode::Inherit::
Inherit(const TypeRegistryNode::Inherit &copy) :
  _top(copy._top),
  _mask(copy._mask),
  _bits(copy._bits)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TypeRegistryNode::Inherit::
operator = (const TypeRegistryNode::Inherit &copy) {
  _top = copy._top;
  _mask = copy._mask;
  _bits = copy._bits;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TypeRegistryNode::Inherit::
operator < (const Inherit &other) const {
  return _top < other._top;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistryNode::Inherit::is_derived_from
//       Access: Public
//  Description: Assuming the two Inherit objects share the same
//               subtree top, this returns true if the bitmasks
//               indicate that child inherits from base, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TypeRegistryNode::Inherit::
is_derived_from(const TypeRegistryNode::Inherit &child, 
                const TypeRegistryNode::Inherit &base) {
  assert(child._top == base._top);

  // Child derives from base if and only if its subtree mask contains
  // more bits (or the same number of bits), and the n low-order
  // subtree bits that are in common are identical.
  return ((child._mask & base._mask) == base._mask &&
          (child._bits & base._mask) == base._bits);
}
