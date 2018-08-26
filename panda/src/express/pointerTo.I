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
PointerTo(To *ptr) noexcept : PointerToBase<T>(ptr) {
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

/**
 *
 */
template<class T>
INLINE PointerTo<T>::
PointerTo(PointerTo<T> &&from) noexcept :
  PointerToBase<T>(std::move(from))
{
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE PointerTo<T>::
PointerTo(Y *ptr) noexcept :
  PointerToBase<T>(ptr)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE PointerTo<T>::
PointerTo(const PointerTo<Y> &r) noexcept :
  PointerToBase<T>(r.p())
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE PointerTo<T>::
PointerTo(PointerTo<Y> &&r) noexcept :
  PointerToBase<T>(std::move(r))
{
}
#endif  // !CPPPARSER

/**
 *
 */
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (PointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE PointerTo<T> &PointerTo<T>::
operator = (const PointerTo<Y> &r) noexcept {
  this->reassign(r.p());
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE PointerTo<T> &PointerTo<T>::
operator = (PointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}
#endif  // !CPPPARSER

/**
 *
 */
template<class T>
constexpr typename PointerTo<T>::To &PointerTo<T>::
operator *() const noexcept {
  return *((To *)(this->_void_ptr));
}

/**
 *
 */
template<class T>
constexpr typename PointerTo<T>::To *PointerTo<T>::
operator -> () const noexcept {
  return (To *)(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert PointerTo's to
 * the required kind of actual pointer.  This introduces ambiguities which the
 * compiler will resolve one way or the other, but we don't care which way it
 * goes because either will be correct.
 */
template<class T>
constexpr PointerTo<T>::
operator T * () const noexcept {
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
constexpr typename PointerTo<T>::To *PointerTo<T>::
p() const noexcept {
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
ConstPointerTo(const typename ConstPointerTo<T>::To *ptr) noexcept :
  PointerToBase<T>((typename ConstPointerTo<T>::To *)ptr)
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
ConstPointerTo(const ConstPointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(PointerTo<T> &&from) noexcept :
  PointerToBase<T>(std::move(from))
{
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(ConstPointerTo<T> &&from) noexcept :
  PointerToBase<T>(std::move(from))
{
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(const Y *ptr) noexcept :
  PointerToBase<T>((Y *)ptr)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(const PointerTo<Y> &r) noexcept :
  PointerToBase<T>(r.p())
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(const ConstPointerTo<Y> &r) noexcept :
  PointerToBase<T>((Y *)r.p())
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(PointerTo<Y> &&r) noexcept :
  PointerToBase<T>(std::move(r))
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T>::
ConstPointerTo(ConstPointerTo<Y> &&r) noexcept :
  PointerToBase<T>(std::move(r))
{
}
#endif  // !CPPPARSER

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (PointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (ConstPointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const PointerTo<Y> &r) noexcept {
  this->reassign(r.p());
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const ConstPointerTo<Y> &r) noexcept {
  this->reassign((Y *)r.p());
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (PointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (ConstPointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}
#endif  // !CPPPARSER

/**
 *
 */
template<class T>
constexpr const typename ConstPointerTo<T>::To &ConstPointerTo<T>::
operator *() const noexcept {
  return *((To *)(this->_void_ptr));
}

/**
 *
 */
template<class T>
constexpr const typename ConstPointerTo<T>::To *ConstPointerTo<T>::
operator -> () const noexcept {
  return (To *)(this->_void_ptr);
}

/**
 * We also have the typecast operator to automatically convert
 * ConstPointerTo's to the required kind of actual pointer.  This introduces
 * ambiguities which the compiler will resolve one way or the other, but we
 * don't care which way it goes because either will be correct.
 */
template<class T>
constexpr ConstPointerTo<T>::
operator const T * () const noexcept {
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
constexpr const typename ConstPointerTo<T>::To *ConstPointerTo<T>::
p() const noexcept {
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
