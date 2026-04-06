/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePointerTo.I
 * @author drose
 * @date 2005-05-07
 */

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(To *ptr) : NodePointerToBase<T>(ptr) {
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(const NodePointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T>::
NodePointerTo(NodePointerTo<T> &&from) noexcept :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (NodePointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE typename NodePointerTo<T>::To &NodePointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE typename NodePointerTo<T>::To *NodePointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * We also have the typecast operator to automatically convert NodePointerTo's
 * to the required kind of actual pointer.  This introduces ambiguities which
 * the compiler will resolve one way or the other, but we don't care which way
 * it goes because either will be correct.
 */
template<class T>
INLINE NodePointerTo<T>::
operator T *() const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Returns an ordinary pointer instead of a NodePointerTo.  Useful to work
 * around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE typename NodePointerTo<T>::To *NodePointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (To *ptr) {
  this->reassign(ptr);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodePointerTo<T> &NodePointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const typename NodeConstPointerTo<T>::To *ptr) :
  NodePointerToBase<T>((typename NodeConstPointerTo<T>::To *)ptr)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const NodePointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(const NodeConstPointerTo<T> &copy) :
  NodePointerToBase<T>((const NodePointerToBase<T> &)copy)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(NodePointerTo<T> &&from) noexcept :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T>::
NodeConstPointerTo(NodeConstPointerTo<T> &&from) noexcept :
  NodePointerToBase<T>((NodePointerToBase<T> &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (NodePointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (NodeConstPointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE const typename NodeConstPointerTo<T>::To &NodeConstPointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE const typename NodeConstPointerTo<T>::To *NodeConstPointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * We also have the typecast operator to automatically convert
 * NodeConstPointerTo's to the required kind of actual pointer.  This
 * introduces ambiguities which the compiler will resolve one way or the
 * other, but we don't care which way it goes because either will be correct.
 */

template<class T>
INLINE NodeConstPointerTo<T>::
operator const T * () const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * Returns an ordinary pointer instead of a NodeConstPointerTo.  Useful to
 * work around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE const typename NodeConstPointerTo<T>::To *NodeConstPointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodePointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE NodeConstPointerTo<T> &NodeConstPointerTo<T>::
operator = (const NodeConstPointerTo<T> &copy) {
  this->reassign((const NodePointerToBase<T> &)copy);
  return *this;
}
#endif  // CPPPARSER
