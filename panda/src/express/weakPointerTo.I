// Filename: weakPointerTo.I
// Created by:  drose (27Sep04)
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  nassertr(!is_null(), *((To *)NULL));
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME WeakPointerTo<T>::To *WeakPointerTo<T>::
operator -> () const {
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
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
operator TYPENAME WeakPointerToBase<T>::To *() const {
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
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
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerTo<T> &WeakPointerTo<T>::
operator = (To *ptr) {
  reassign(ptr);
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
  reassign((const PointerToBase<T> &)copy);
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
  reassign((const PointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T>::
WeakConstPointerTo(const TYPENAME WeakConstPointerTo<T>::To *ptr) :
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  WeakPointerToBase<T>((const PointerToBase<T> &)copy)
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
  nassertr(!is_null(), *((To *)NULL));
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME WeakConstPointerTo<T>::To *WeakConstPointerTo<T>::
operator -> () const {
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
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
operator const TYPENAME WeakPointerToBase<T>::To *() const {
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
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
  nassertr(!is_null(), (To *)NULL);
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakConstPointerTo::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakConstPointerTo<T> &WeakConstPointerTo<T>::
operator = (const To *ptr) {
  reassign((To *)ptr);
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
  reassign((const PointerToBase<T> &)copy);
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
  reassign((const PointerToBase<T> &)copy);
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
  reassign((const PointerToBase<T> &)copy);
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
  reassign((const PointerToBase<T> &)copy);
  return *this;
}
