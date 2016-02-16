/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSafePointerTo.I
 * @author drose
 * @date 2006-04-28
 */

/**

 */
template<class T>
INLINE ThreadSafePointerTo<T>::
ThreadSafePointerTo(To *ptr) : ThreadSafePointerToBase<T>(ptr) {
}

/**

 */
template<class T>
INLINE ThreadSafePointerTo<T>::
ThreadSafePointerTo(const ThreadSafePointerTo<T> &copy) :
  ThreadSafePointerToBase<T>((const ThreadSafePointerToBase<T> &)copy)
{
}

/**

 */
template<class T>
INLINE ThreadSafePointerTo<T>::
~ThreadSafePointerTo() {
}

/**

 */
template<class T>
INLINE TYPENAME ThreadSafePointerTo<T>::To &ThreadSafePointerTo<T>::
operator *() const {
  return *((To *)AtomicAdjust::get_ptr(this->_void_ptr));
}

/**

 */
template<class T>
INLINE TYPENAME ThreadSafePointerTo<T>::To *ThreadSafePointerTo<T>::
operator -> () const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert
 * ThreadSafePointerTo's to the required kind of actual pointer.  This
 * introduces ambiguities which the compiler will resolve one way or the other,
 * but we don't care which way it goes because either will be correct.
 */
template<class T>
INLINE ThreadSafePointerTo<T>::
operator T * () const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**
 * Returns an ordinary pointer instead of a ThreadSafePointerTo.  Useful to work
 * around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE TYPENAME ThreadSafePointerTo<T>::To *ThreadSafePointerTo<T>::
p() const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**

 */
template<class T>
INLINE ThreadSafePointerTo<T> &ThreadSafePointerTo<T>::
operator = (To *ptr) {
  this->reassign(ptr);
  return *this;
}

/**

 */
template<class T>
INLINE ThreadSafePointerTo<T> &ThreadSafePointerTo<T>::
operator = (const ThreadSafePointerTo<T> &copy) {
  this->reassign((const ThreadSafePointerToBase<T> &)copy);
  return *this;
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T>::
ThreadSafeConstPointerTo(const TYPENAME ThreadSafeConstPointerTo<T>::To *ptr) :
  ThreadSafePointerToBase<T>((TYPENAME ThreadSafeConstPointerTo<T>::To *)ptr)
{
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T>::
ThreadSafeConstPointerTo(const ThreadSafePointerTo<T> &copy) :
  ThreadSafePointerToBase<T>((const ThreadSafePointerToBase<T> &)copy)
{
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T>::
~ThreadSafeConstPointerTo() {
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T>::
ThreadSafeConstPointerTo(const ThreadSafeConstPointerTo<T> &copy) :
  ThreadSafePointerToBase<T>((const ThreadSafePointerToBase<T> &)copy)
{
}

/**

 */
template<class T>
INLINE const TYPENAME ThreadSafeConstPointerTo<T>::To &ThreadSafeConstPointerTo<T>::
operator *() const {
  return *((To *)AtomicAdjust::get_ptr(this->_void_ptr));
}

/**

 */
template<class T>
INLINE const TYPENAME ThreadSafeConstPointerTo<T>::To *ThreadSafeConstPointerTo<T>::
operator -> () const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert
 * ThreadSafeConstPointerTo's to the required kind of actual pointer.  This
 * introduces ambiguities which the compiler will resolve one way or the other,
 * but we don't care which way it goes because either will be correct.
 */

template<class T>
INLINE ThreadSafeConstPointerTo<T>::
operator const T * () const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**
 * Returns an ordinary pointer instead of a ThreadSafeConstPointerTo.  Useful to
 * work around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE const TYPENAME ThreadSafeConstPointerTo<T>::To *ThreadSafeConstPointerTo<T>::
p() const {
  return (To *)AtomicAdjust::get_ptr(this->_void_ptr);
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T> &ThreadSafeConstPointerTo<T>::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T> &ThreadSafeConstPointerTo<T>::
operator = (const ThreadSafePointerTo<T> &copy) {
  this->reassign((const ThreadSafePointerToBase<T> &)copy);
  return *this;
}

/**

 */
template<class T>
INLINE ThreadSafeConstPointerTo<T> &ThreadSafeConstPointerTo<T>::
operator = (const ThreadSafeConstPointerTo<T> &copy) {
  this->reassign((const ThreadSafePointerToBase<T> &)copy);
  return *this;
}
