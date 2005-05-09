// Filename: nodePointerTo.I
// Created by:  drose (07May05)
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


////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(To *ptr) : NodePointerToBase<T>(ptr) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(const NodePointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME NodePointerTo<T>::To &NodePointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME NodePointerTo<T>::To *NodePointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert NodePointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
operator TYPENAME NodePointerToBase<T>::To *() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a NodePointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME NodePointerTo<T>::To *NodePointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (To *ptr) {
  reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  reassign((const NodePointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const TYPENAME NodeConstPointerTo<T>::To *ptr) :
  NodePointerToBase<T>((TYPENAME NodeConstPointerTo<T>::To *)ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const NodePointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const NodeConstPointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME NodeConstPointerTo<T>::To &NodeConstPointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME NodeConstPointerTo<T>::To *NodeConstPointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert NodeConstPointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////

template<class T>
INLINE NodeConstPointerTo<T>::
operator const TYPENAME NodePointerToBase<T>::To *() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a NodeConstPointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME NodeConstPointerTo<T>::To *NodeConstPointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const To *ptr) {
  reassign((To *)ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  reassign((const NodePointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodeConstPointerTo<T> &copy) {
  reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
