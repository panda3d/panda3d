/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifyCategoryProxy.h
 * @author drose
 * @date 2000-03-04
 */

#ifndef NOTIFYCATEGORYPROXY_H
#define NOTIFYCATEGORYPROXY_H

#include "dtoolbase.h"

#include "notifyCategory.h"
#include "notifySeverity.h"
#include "pnotify.h"

/**
 * A handy wrapper around a NotifyCategory pointer.  This wrapper pretends to
 * be a NotifyCategory object itself, except that it is capable of
 * initializing its pointer if it is NULL.
 *
 * The advantage to this over a normal pointer is that it can be used in
 * functions that run at static init time, without worrying about ordering
 * issues among static init routines.  If the pointer hasn't been initialized
 * yet, no sweat; it can initialize itself.
 *
 * This must be a template class so it can do this magic; it templates on a
 * class with a static method called get_category() that returns a new pointer
 * to the NotifyCategory.  This way the compiler can generate correct static-
 * init-independent code to initialize the proxy.
 *
 * In general, if the proxy object is treated as if it were itself a
 * NotifyCategory object, then it doesn't check whether its category is
 * initialized, and so may not be run at static init time.  That is, you may
 * call proxy.info(), but only when you are not running at static init time.
 * This is an optimization so you can avoid this unnecessary check when you
 * know (as in most cases) the code does not run at static init.
 *
 * On the other hand, if the proxy object is treated as if it were a *pointer*
 * to a NotifyCategory object, then it *does* check whether its category is
 * initialized; you may safely use it in this way at static init time.  Thus,
 * you may call proxy->info() safely whenever you like.
 */
template<class GetCategory>
class NotifyCategoryProxy {
public:
  // This should be set to be called at static init time; it initializes the
  // pointer if it is not already.
  NotifyCategory *init();

/*
 * You don't normally need to call these directly, but they're here anyway.
 * get_unsafe_ptr() assumes the pointer has been initialized; it should be
 * called only when you know static init has completed (i.e.  in any function
 * that is not executing at static init time).  get_safe_ptr() should be
 * called when it is possible that static init has not yet completed (i.e.  in
 * a function that might execute at static init time); it calls init() first.
 */
  INLINE NotifyCategory *get_unsafe_ptr();
  INLINE NotifyCategory *get_safe_ptr();

  // The following functions, which may be accessed using the proxy.function()
  // syntax, call get_unsafe_ptr().  They should be used only in non-static-
  // init functions.

  INLINE bool is_on(NotifySeverity severity);

  INLINE bool is_spam();
  INLINE bool is_debug();
  INLINE bool is_info();
  INLINE bool is_warning();
  INLINE bool is_error();
  INLINE bool is_fatal();

  INLINE std::ostream &out(NotifySeverity severity, bool prefix = true);
  INLINE std::ostream &spam(bool prefix = true);
  INLINE std::ostream &debug(bool prefix = true);
  INLINE std::ostream &info(bool prefix = true);
  INLINE std::ostream &warning(bool prefix = true);
  INLINE std::ostream &error(bool prefix = true);
  INLINE std::ostream &fatal(bool prefix = true);

  // The same functions as above, when accessed using proxy->function()
  // syntax, call get_safe_ptr().  These can be used safely either in static-
  // init or non-static-init functions.
  INLINE NotifyCategory *operator -> ();
  INLINE NotifyCategory &operator * ();
  INLINE operator NotifyCategory * ();

private:
  NotifyCategory *_ptr;
};

template<class GetCategory>
INLINE std::ostream &operator << (std::ostream &out, NotifyCategoryProxy<GetCategory> &proxy) {
  return out << proxy->get_fullname();
}

// Finally, here is a set of handy macros to define and reference a
// NotifyCategoryProxy object in each package.

// Following the config convention, this macro defines an external reference
// to a suitable NotifyCategoryProxy object; it should appear in the
// config_*.h file.  The proxy object will be named basename_cat.

#ifdef CPPPARSER
#define NotifyCategoryDecl(basename, expcl, exptp)
#else
#define NotifyCategoryDecl(basename, expcl, exptp) \
  class expcl NotifyCategoryGetCategory_ ## basename { \
  public: \
    NotifyCategoryGetCategory_ ## basename(); \
    static NotifyCategory *get_category(); \
  }; \
  EXPORT_TEMPLATE_CLASS(expcl, exptp, NotifyCategoryProxy<NotifyCategoryGetCategory_ ## basename>); \
  extern expcl NotifyCategoryProxy<NotifyCategoryGetCategory_ ## basename> basename ## _cat;
#endif

// This macro is the same as the above, except that it declares a category
// that is not intended to be exported from any DLL.

#define NotifyCategoryDeclNoExport(basename) \
  class NotifyCategoryGetCategory_ ## basename { \
  public: \
    NotifyCategoryGetCategory_ ## basename(); \
    static NotifyCategory *get_category(); \
  }; \
  extern NotifyCategoryProxy<NotifyCategoryGetCategory_ ## basename> basename ## _cat;

// This macro defines the actual declaration of the NotifyCategoryProxy object
// defined above; it should appear in the config_*.C file.  In this macro,
// parent_category may either be the NotifyCategoryProxy object of the parent
// category (e.g.  parent_cat), or it may be the quoted fullname of the
// parent.

#ifdef CPPPARSER
#define NotifyCategoryDefName(basename, actual_name, parent_category)
#define NotifyCategoryDef(basename, parent_category)

#else
#define NotifyCategoryDefName(basename, actual_name, parent_category) \
  template class NotifyCategoryProxy<NotifyCategoryGetCategory_ ## basename>; \
  NotifyCategoryProxy<NotifyCategoryGetCategory_ ## basename> basename ## _cat; \
  static NotifyCategoryGetCategory_ ## basename force_init_ ## basename ## _cat; \
  NotifyCategoryGetCategory_ ## basename:: \
  NotifyCategoryGetCategory_ ## basename() { \
    basename ## _cat.init(); \
  } \
  NotifyCategory *NotifyCategoryGetCategory_ ## basename:: \
  get_category() { \
    return Notify::ptr()->get_category(std::string(actual_name), parent_category); \
  }
#define NotifyCategoryDef(basename, parent_category) \
  NotifyCategoryDefName(basename, #basename, parent_category);

#endif // CPPPARSER


#include "notifyCategoryProxy.I"

#endif
