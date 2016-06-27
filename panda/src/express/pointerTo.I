/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerTo.I
 * @author drose
 * @date 1999-02-10
 */

/**
 *
 */
template<class T>
ALWAYS_INLINE PointerTo<T>::
PointerTo(To *ptr) : PointerToBase<T>(ptr) {
}

/**
 *
 */
template<class T>
INLINE PointerTo<T>::
PointerTo(const PointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

#ifdef USE_MOVE_SEMANTICS
/**
 *
 */
template<class T>
INLINE PointerTo<T>::
PointerTo(PointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

/**
 *
 */
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (PointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // USE_MOVE_SEMANTICS

/**
 *
 */
template<class T>
INLINE PointerTo<T>::
~PointerTo() {
}

/**
 *
 */
template<class T>
INLINE TYPENAME PointerTo<T>::To &PointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

/**
 *
 */
template<class T>
INLINE TYPENAME PointerTo<T>::To *PointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert PointerTo's to
 * the required kind of actual pointer.  This introduces ambiguities which the
 * compiler will resolve one way or the other, but we don't care which way it
 * goes because either will be correct.
 */
template<class T>
INLINE PointerTo<T>::
operator T * () const {
  return (To *)(this->_void_ptr);
}

/**
 * Returns a reference to the underlying pointer.  This is a very unsafe
 * method.  It's only used by some interrogate code.  If you think this method
 * might be useful to you, you're probably wrong.
 *
 * Promise me you won't use this, okay?
 */
template<class T>
INLINE T *&PointerTo<T>::
cheat() {
  return (To *&)(this->_void_ptr);
}

/**
 * Returns an ordinary pointer instead of a PointerTo.  Useful to work around
 * compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE TYPENAME PointerTo<T>::To *PointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

/**
 *
 */
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (To *ptr) {
  this->reassign(ptr);
  return *this;
}

/**
 *
 */
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (const PointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(const TYPENAME ConstPointerTo<T>::To *ptr) :
  PointerToBase<T>((TYPENAME ConstPointerTo<T>::To *)ptr)
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const PointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
~ConstPointerTo() {
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const ConstPointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

#ifdef USE_MOVE_SEMANTICS
/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(PointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(ConstPointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (PointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (ConstPointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // USE_MOVE_SEMANTICS

/**
 *
 */
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To &ConstPointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

/**
 *
 */
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To *ConstPointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert
 * ConstPointerTo's to the required kind of actual pointer.  This introduces
 * ambiguities which the compiler will resolve one way or the other, but we
 * don't care which way it goes because either will be correct.
 */
template<class T>
INLINE ConstPointerTo<T>::
operator const T * () const {
return (To *)(this->_void_ptr);
}

/**
 * Returns a reference to the underlying pointer.  This is a very unsafe
 * method.  It's only used by some interrogate code.  If you think this method
 * might be useful to you, you're probably wrong.
 *
 * Promise me you won't use this, okay?
 */
template<class T>
INLINE const T *&ConstPointerTo<T>::
cheat() {
  return (const To *&)(this->_void_ptr);
}

/**
 * Returns an ordinary pointer instead of a ConstPointerTo.  Useful to work
 * around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To *ConstPointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const ConstPointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}
