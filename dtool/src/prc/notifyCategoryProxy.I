// Filename: notifyCategoryProxy.I
// Created by:  drose (04Mar00)
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
//     Function: NotifyCategoryProxy::init()
//       Access: Public
//  Description: Initializes the proxy object by calling
//               get_category() on the template class.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
NotifyCategory *NotifyCategoryProxy<GetCategory>::
init() {
  if (_ptr == (NotifyCategory *)NULL) {
    _ptr = GetCategory::get_category();
  }
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::get_unsafe_ptr()
//       Access: Public
//  Description: Returns a pointer which is assumed to have been
//               already initialized.  This function should only be
//               used in functions that will certainly not execute at
//               static init time.  All of the category methods that
//               are accessed via the dot operator, e.g. proxy.info(),
//               use this method.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
get_unsafe_ptr() {
  nassertd(_ptr != (NotifyCategory *)NULL) {
    init();
    nout << "Uninitialized notify proxy: " << _ptr->get_fullname() << "\n";
  }
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::get_safe_ptr()
//       Access: Public
//  Description: Returns a pointer which is *not* assumed to have been
//               already initialized; if necessary, it will be
//               initialized before it returns.  This function may be
//               used in functions that might execute at static init
//               time.  All of the category methods that are accessed
//               via the arrow operator, e.g. proxy->info(), use this
//               method.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
get_safe_ptr() {
  return init();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_on(NotifySeverity severity) {
  return get_unsafe_ptr()->is_on(severity);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_spam
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
#ifdef NOTIFY_DEBUG
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_spam() {
  return get_unsafe_ptr()->is_spam();
}
#else
template<class GetCategory>
CONSTEXPR bool NotifyCategoryProxy<GetCategory>::
is_spam() {
  return false;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_debug
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
#ifdef NOTIFY_DEBUG
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_debug() {
  return get_unsafe_ptr()->is_debug();
}
#else
template<class GetCategory>
CONSTEXPR bool NotifyCategoryProxy<GetCategory>::
is_debug() {
  return false;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_info
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_info() {
  return get_unsafe_ptr()->is_info();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_warning
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_warning() {
  return get_unsafe_ptr()->is_warning();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_error
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_error() {
  return get_unsafe_ptr()->is_error();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::is_fatal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_fatal() {
  return get_unsafe_ptr()->is_fatal();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::out
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
out(NotifySeverity severity, bool prefix) {
  return get_unsafe_ptr()->out(severity, prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::spam
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
spam(bool prefix) {
  return get_unsafe_ptr()->spam(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::debug
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
debug(bool prefix) {
  return get_unsafe_ptr()->debug(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::info
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
info(bool prefix) {
  return get_unsafe_ptr()->info(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::warning
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
warning(bool prefix) {
  return get_unsafe_ptr()->warning(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::error
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
error(bool prefix) {
  return get_unsafe_ptr()->error(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::fatal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE ostream &NotifyCategoryProxy<GetCategory>::
fatal(bool prefix) {
  return get_unsafe_ptr()->fatal(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::Member Access Operator
//       Access: Public
//  Description: This magic operator function defines the syntax
//               proxy->info(), etc., for all of the methods that are
//               defined for NotifyCategory.  It's designed to vector
//               through get_safe_ptr(), so this syntax is safe for
//               functions that may execute at static init time.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
operator -> () {
  return get_safe_ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::Dereference Operator
//       Access: Public
//  Description: This operator handles the case of dereferencing the
//               proxy object as if it were a pointer,
//               e.g. (*proxy).info().  It works the same way as the
//               -> operator, above.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE NotifyCategory &NotifyCategoryProxy<GetCategory>::
operator * () {
  return *get_safe_ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategoryProxy::Typecast Operator
//       Access: Public
//  Description: This operator handles the case of passing the
//               proxy object to a function that accepts a
//               NotifyCategory pointer.  It works the same way as the
//               -> and * operators, above.
////////////////////////////////////////////////////////////////////
template<class GetCategory>
INLINE NotifyCategoryProxy<GetCategory>::
operator NotifyCategory * () {
  return get_safe_ptr();
}
