// Filename: nodePointerTo.I
// Created by:  drose (07May05)
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


#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(To *ptr) : NodePointerToBase<T>(ptr) {
}
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
~NodePointerTo() {
}
#endif  // CPPPARSER

#ifdef USE_MOVE_SEMANTICS
#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(NodePointerTo<T> &&from) NOEXCEPT :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (NodePointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // CPPPARSER
#endif  // USE_MOVE_SEMANTICS

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
operator T *() const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (To *ptr) {
  this->reassign(ptr);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodePointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
~NodeConstPointerTo() {
}
#endif  // CPPPARSER

#ifdef USE_MOVE_SEMANTICS
#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(NodePointerTo<T> &&from) NOEXCEPT :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(NodeConstPointerTo<T> &&from) NOEXCEPT :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (NodePointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (NodeConstPointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // CPPPARSER
#endif  // USE_MOVE_SEMANTICS

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
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
operator const T * () const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
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
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: NodeConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodeConstPointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER
