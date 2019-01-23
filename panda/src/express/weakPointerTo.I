/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerTo.I
 * @author drose
 * @date 2004-09-27
 */

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(To *ptr) : WeakPointerToBase<T>(ptr) {
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(const PointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(const WeakPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const WeakPointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(WeakPointerTo<T> &&from) noexcept :
  WeakPointerToBase<T>(std::move(from))
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T>::
WeakPointerTo(const WeakPointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T>::
WeakPointerTo(const PointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T>::
WeakPointerTo(WeakPointerTo<Y> &&r) noexcept :
  WeakPointerToBase<T>(std::move(r))
{
}

/**
 *
 */
template<class T>
INLINE typename WeakPointerTo<T>::To &WeakPointerTo<T>::
operator *() const {
  assert(!this->was_deleted());
  return *((To *)WeakPointerToBase<T>::_void_ptr);
}

/**
 *
 */
template<class T>
INLINE typename WeakPointerTo<T>::To *WeakPointerTo<T>::
operator -> () const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * We also have the typecast operator to automatically convert WeakPointerTo's
 * to the required kind of actual pointer.  This introduces ambiguities which
 * the compiler will resolve one way or the other, but we don't care which way
 * it goes because either will be correct.
 */
template<class T>
INLINE WeakPointerTo<T>::
operator T * () const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * A thread-safe way to access the underlying pointer; will silently return
 * null if the underlying pointer was deleted or null.
 * Note that this may return null even if was_deleted() still returns false,
 * which can occur if the object has reached reference count 0 and is about to
 * be destroyed.
 *
 * @since 1.10.0
 */
template<class T>
INLINE PointerTo<T> WeakPointerTo<T>::
lock() const {
  PointerTo<T> ptr;
  this->lock_into(ptr);
  return ptr;
}

/**
 * Returns an ordinary pointer instead of a WeakPointerTo.  Useful to work
 * around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE typename WeakPointerTo<T>::To *WeakPointerTo<T>::
p() const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * Returns the original pointer value, even if the object has since been
 * deleted.
 */
template<class T>
INLINE typename WeakPointerTo<T>::To *WeakPointerTo<T>::
get_orig() const {
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (To *ptr) {
  ((WeakPointerTo<T> *)this)->reassign(ptr);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  ((WeakPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const WeakPointerTo<T> &copy) {
  ((WeakPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (WeakPointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const WeakPointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const PointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (WeakPointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const To *ptr) :
  WeakPointerToBase<T>((typename WeakConstPointerTo<T>::To *)ptr)
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const PointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const ConstPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const WeakPointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakConstPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const WeakPointerToBase<T> *)&copy)
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(WeakPointerTo<T> &&from) noexcept :
  WeakPointerToBase<T>(std::move(from))
{
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(WeakConstPointerTo<T> &&from) noexcept :
  WeakPointerToBase<T>(std::move(from))
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakPointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakConstPointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const PointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const ConstPointerTo<Y> &r) noexcept :
  WeakPointerToBase<T>(r)
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(WeakPointerTo<Y> &&r) noexcept :
  WeakPointerToBase<T>(std::move(r))
{
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(WeakConstPointerTo<Y> &&r) noexcept :
  WeakPointerToBase<T>(std::move(r))
{
}

/**
 *
 */
template<class T>
INLINE const typename WeakConstPointerTo<T>::To &WeakConstPointerTo<T>::
operator *() const {
  assert(!this->was_deleted());
  return *((To *)WeakPointerToBase<T>::_void_ptr);
}

/**
 *
 */
template<class T>
INLINE const typename WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
operator -> () const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * We also have the typecast operator to automatically convert
 * WeakConstPointerTo's to the required kind of actual pointer.  This
 * introduces ambiguities which the compiler will resolve one way or the
 * other, but we don't care which way it goes because either will be correct.
 */

template<class T>
INLINE WeakConstPointerTo<T>::
operator const T * () const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * A thread-safe way to access the underlying pointer; will silently return
 * null if the underlying pointer was deleted or null.
 * Note that this may return null even if was_deleted() still returns false,
 * which can occur if the object has reached reference count 0 and is about to
 * be destroyed.
 *
 * @since 1.10.0
 */
template<class T>
INLINE ConstPointerTo<T> WeakConstPointerTo<T>::
lock() const {
  ConstPointerTo<T> ptr;
  this->lock_into(ptr);
  return ptr;
}

/**
 * Returns an ordinary pointer instead of a WeakConstPointerTo.  Useful to
 * work around compiler problems, particularly for implicit upcasts.
 */
template<class T>
INLINE const typename WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
p() const {
  assert(!this->was_deleted());
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 * Returns the original pointer value, even if the object has since been
 * deleted.
 */
template<class T>
INLINE const typename WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
get_orig() const {
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const To *ptr) {
  ((WeakConstPointerTo<T> *)this)->reassign((To *)ptr);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const ConstPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakConstPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (WeakPointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (WeakConstPointerTo<T> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakPointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakConstPointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const PointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const ConstPointerTo<Y> &r) noexcept {
  this->reassign(r);
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (WeakPointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}

/**
 *
 */
template<class T>
template<class Y>
ALWAYS_INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (WeakConstPointerTo<Y> &&r) noexcept {
  this->reassign(std::move(r));
  return *this;
}
