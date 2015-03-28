// Filename: pointerTo.I
// Created by:  drose (10Feb99)
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
//     Function: PointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
PointerTo(To *ptr) : PointerToBase<T>(ptr) {
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
PointerTo(const PointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
PointerTo(PointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (PointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
~PointerTo() {
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME PointerTo<T>::To &PointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME PointerTo<T>::To *PointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert PointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
operator T * () const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::cheat
//       Access: Public
//  Description: Returns a reference to the underlying pointer.  This
//               is a very unsafe method.  It's only used by some
//               interrogate code.  If you think this method might be
//               useful to you, you're probably wrong.
//
//               Promise me you won't use this, okay?
////////////////////////////////////////////////////////////////////
template<class T>
INLINE T *&PointerTo<T>::
cheat() {
  return (To *&)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::p
//       Access: Published
//  Description: Returns an ordinary pointer instead of a PointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME PointerTo<T>::To *PointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (To *ptr) {
  this->reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (const PointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const TYPENAME ConstPointerTo<T>::To *ptr) :
  PointerToBase<T>((TYPENAME ConstPointerTo<T>::To *)ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const PointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
~ConstPointerTo() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const ConstPointerTo<T> &copy) :
  PointerToBase<T>((const PointerToBase<T> &)copy)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(PointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(ConstPointerTo<T> &&from) NOEXCEPT :
  PointerToBase<T>(move(from))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (PointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (ConstPointerTo<T> &&from) NOEXCEPT {
  this->reassign(move(from));
  return *this;
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Dereference operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To &ConstPointerTo<T>::
operator *() const {
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Member access operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To *ConstPointerTo<T>::
operator -> () const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert ConstPointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
operator const T * () const {
return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::cheat
//       Access: Public
//  Description: Returns a reference to the underlying pointer.  This
//               is a very unsafe method.  It's only used by some
//               interrogate code.  If you think this method might be
//               useful to you, you're probably wrong.
//
//               Promise me you won't use this, okay?
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const T *&ConstPointerTo<T>::
cheat() {
  return (const To *&)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::p
//       Access: Published
//  Description: Returns an ordinary pointer instead of a ConstPointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME ConstPointerTo<T>::To *ConstPointerTo<T>::
p() const {
  return (To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const ConstPointerTo<T> &copy) {
  this->reassign((const PointerToBase<T> &)copy);
  return *this;
}
