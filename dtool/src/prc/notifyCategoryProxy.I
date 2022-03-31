/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifyCategoryProxy.I
 * @author drose
 * @date 2000-03-04
 */

/**
 * Initializes the proxy object by calling get_category() on the template
 * class.
 */
template<class GetCategory>
NotifyCategory *NotifyCategoryProxy<GetCategory>::
init() {
  if (_ptr == nullptr) {
    _ptr = GetCategory::get_category();
  }
  return _ptr;
}

/**
 * Returns a pointer which is assumed to have been already initialized.  This
 * function should only be used in functions that will certainly not execute
 * at static init time.  All of the category methods that are accessed via the
 * dot operator, e.g.  proxy.info(), use this method.
 */
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
get_unsafe_ptr() {
  nassertd(_ptr != nullptr) {
    init();
    nout << "Uninitialized notify proxy: " << _ptr->get_fullname() << "\n";
  }
  return _ptr;
}

/**
 * Returns a pointer which is *not* assumed to have been already initialized;
 * if necessary, it will be initialized before it returns.  This function may
 * be used in functions that might execute at static init time.  All of the
 * category methods that are accessed via the arrow operator, e.g.
 * proxy->info(), use this method.
 */
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
get_safe_ptr() {
  return init();
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_on(NotifySeverity severity) {
  return get_unsafe_ptr()->is_on(severity);
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_spam() {
#ifdef NOTIFY_DEBUG
  // Instruct the compiler to optimize for the usual case.
  return UNLIKELY(get_unsafe_ptr()->is_spam());
#else
  return false;
#endif
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_debug() {
#ifdef NOTIFY_DEBUG
  // Instruct the compiler to optimize for the usual case.
  return UNLIKELY(get_unsafe_ptr()->is_debug());
#else
  return false;
#endif
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_info() {
  return get_unsafe_ptr()->is_info();
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_warning() {
  return get_unsafe_ptr()->is_warning();
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_error() {
  return get_unsafe_ptr()->is_error();
}

/**
 *
 */
template<class GetCategory>
INLINE bool NotifyCategoryProxy<GetCategory>::
is_fatal() {
  return get_unsafe_ptr()->is_fatal();
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
out(NotifySeverity severity, bool prefix) {
  return get_unsafe_ptr()->out(severity, prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
spam(bool prefix) {
  return get_unsafe_ptr()->spam(prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
debug(bool prefix) {
  return get_unsafe_ptr()->debug(prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
info(bool prefix) {
  return get_unsafe_ptr()->info(prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
warning(bool prefix) {
  return get_unsafe_ptr()->warning(prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
error(bool prefix) {
  return get_unsafe_ptr()->error(prefix);
}

/**
 *
 */
template<class GetCategory>
INLINE std::ostream &NotifyCategoryProxy<GetCategory>::
fatal(bool prefix) {
  return get_unsafe_ptr()->fatal(prefix);
}

/**
 * This magic operator function defines the syntax proxy->info(), etc., for
 * all of the methods that are defined for NotifyCategory.  It's designed to
 * vector through get_safe_ptr(), so this syntax is safe for functions that
 * may execute at static init time.
 */
template<class GetCategory>
INLINE NotifyCategory *NotifyCategoryProxy<GetCategory>::
operator -> () {
  return get_safe_ptr();
}

/**
 * This operator handles the case of dereferencing the proxy object as if it
 * were a pointer, e.g.  (*proxy).info().  It works the same way as the ->
 * operator, above.
 */
template<class GetCategory>
INLINE NotifyCategory &NotifyCategoryProxy<GetCategory>::
operator * () {
  return *get_safe_ptr();
}

/**
 * This operator handles the case of passing the proxy object to a function
 * that accepts a NotifyCategory pointer.  It works the same way as the -> and
 * * operators, above.
 */
template<class GetCategory>
INLINE NotifyCategoryProxy<GetCategory>::
operator NotifyCategory * () {
  return get_safe_ptr();
}
