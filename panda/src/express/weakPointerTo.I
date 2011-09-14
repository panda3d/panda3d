// Filename: weakPointerTo.I
// Created by:  drose (27Sep04)
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
//     Function: WeakPointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(To *ptr) : WeakPointerToBase<T>(ptr) {
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(const PointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T>::
WeakPointerTo(const WeakPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME WeakPointerTo<T>::To &WeakPointerTo<T>::
operator *() const {
  nassertr(!this->was_deleted(), *((To *)NULL));
  return *((To *)WeakPointerToBase<T>::_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME WeakPointerTo<T>::To *WeakPointerTo<T>::
operator -> () const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert WeakPointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T>::
operator T * () const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a WeakPointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME WeakPointerTo<T>::To *WeakPointerTo<T>::
p() const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::get_orig
//       Access: Public
//  Description: Returns the original pointer value, even if the
//               object has since been deleted.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME WeakPointerTo<T>::To *WeakPointerTo<T>::
get_orig() const {
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (To *ptr) {
  ((WeakPointerTo<T> *)this)->reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  ((WeakPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (const WeakPointerTo<T> &copy) {
  ((WeakPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const To *ptr) :
  WeakPointerToBase<T>((TYPENAME WeakConstPointerTo<T>::To *)ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const PointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const ConstPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const WeakConstPointerTo<T> &copy) :
  WeakPointerToBase<T>(*(const PointerToBase<T> *)&copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME WeakConstPointerTo<T>::To &WeakConstPointerTo<T>::
operator *() const {
  nassertr(!this->was_deleted(), *((To *)NULL));
  return *((To *)WeakPointerToBase<T>::_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
operator -> () const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert WeakConstPointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////

template<class T>
INLINE WeakConstPointerTo<T>::
operator const T * () const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a WeakConstPointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
p() const {
  nassertr(!this->was_deleted(), (To *)NULL);
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::get_orig
//       Access: Public
//  Description: Returns the original pointer value, even if the
//               object has since been deleted.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
get_orig() const {
  return (To *)WeakPointerToBase<T>::_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const To *ptr) {
  ((WeakConstPointerTo<T> *)this)->reassign((To *)ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const ConstPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const WeakConstPointerTo<T> &copy) {
  ((WeakConstPointerTo<T> *)this)->reassign(*(const PointerToBase<T> *)&copy);
  return *this;
}
