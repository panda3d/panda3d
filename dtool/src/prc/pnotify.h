/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnotify.h
 * @author drose
 * @date 2000-02-28
 */

#ifndef NOTIFY_H
#define NOTIFY_H

#include "dtoolbase.h"
#include "notifySeverity.h"
#include <map>

class NotifyCategory;

/**
 * An object that handles general error reporting to the user.  It contains a
 * pointer to an ostream, initially cerr, which can be reset at will to point
 * to different output devices, according to the needs of the application.
 * All output generated within Panda should vector through the Notify ostream.
 *
 * This also includes a collection of Categories and Severities, which may be
 * independently enabled or disabled, so that error messages may be squelched
 * or respected according to the wishes of the user.
 */
class EXPCL_DTOOL_PRC Notify {
PUBLISHED:
  Notify();
  ~Notify();

  void set_ostream_ptr(std::ostream *ostream_ptr, bool delete_later);
  std::ostream *get_ostream_ptr() const;

  typedef bool AssertHandler(const char *expression, int line,
                             const char *source_file);

  void set_assert_handler(AssertHandler *assert_handler);
  void clear_assert_handler();
  bool has_assert_handler() const;
  AssertHandler *get_assert_handler() const;

  INLINE bool has_assert_failed() const;
  INLINE const std::string &get_assert_error_message() const;
  INLINE void clear_assert_failed();

  NotifyCategory *get_top_category();
  NotifyCategory *get_category(const std::string &basename,
                               NotifyCategory *parent_category);
  NotifyCategory *get_category(const std::string &basename,
                               const std::string &parent_fullname);
  NotifyCategory *get_category(const std::string &fullname);

  static std::ostream &out();
  static std::ostream &null();
  static void write_string(const std::string &str);
  static Notify *ptr();

public:
  static ios_fmtflags get_literal_flag();

  bool assert_failure(const std::string &expression, int line,
                      const char *source_file);
  bool assert_failure(const char *expression, int line,
                      const char *source_file);

  static NotifySeverity string_severity(const std::string &string);

  void config_initialized();

private:
  std::ostream *_ostream_ptr;
  bool _owns_ostream_ptr;
  std::ostream *_null_ostream_ptr;

  AssertHandler *_assert_handler;
  bool _assert_failed;
  std::string _assert_error_message;

  // This shouldn't be a pmap, since it might be invoked before we initialize
  // the global malloc pointers.
  typedef std::map<std::string, NotifyCategory *> Categories;
  Categories _categories;

  static Notify *_global_ptr;
};


// This defines the symbol nout in the same way that cerr and cout are
// defined, for compactness of C++ code that uses Notify in its simplest form.
// Maybe it's a good idea to define this symbol and maybe it's not, but it
// does seem that "nout" isn't likely to collide with any other name.

#define nout (Notify::out())

// Here are a couple of assert-type functions.  These are designed to avoid
// simply dumping core, since that's quite troublesome when the programmer is
// working in a higher-level environment that is calling into the C++ layer.

// nassertr() is intended to be used in functions that have return values; it
// returns the indicated value if the assertion fails.

// nassertv() is intended to be used in functions that do not have return
// values; it simply returns if the assertion fails.

// nassertd() does not return from the function, but instead executes the
// following block of code (like an if statement) if the assertion fails.

// nassertr_always() and nassertv_always() are like nassertr() and nassertv(),
// except that they will not get completely compiled out if NDEBUG is set.
// Instead, they will quietly return from the function.  These macros are
// appropriate, for instance, for sanity checking user input parameters, where
// optimal performance is not paramount.

// nassert_static() is a compile-time assertion.  It should only be used with
// constant expressions and compilation will fail if the assertion is not
// true.

#ifdef __GNUC__
// Tell the optimizer to optimize for the case where the condition is true.
#define _nassert_check(condition) (__builtin_expect(!(condition), 0))
#else
#define _nassert_check(condition) (!(condition))
#endif

#ifdef NDEBUG

#define nassertr(condition, return_value)
#define nassertv(condition)
#define nassertd(condition) if (false)
// We trust the compiler to optimize the above out.

#define nassertr_always(condition, return_value) \
  { \
    if (_nassert_check(condition)) { \
      return return_value; \
    } \
  }

#define nassertv_always(condition) \
  { \
    if (_nassert_check(condition)) { \
      return; \
    } \
  }

#define nassert_raise(message) Notify::write_string(message)

#else   // NDEBUG

#define nassertr(condition, return_value) \
  { \
    if (_nassert_check(condition)) { \
      if (Notify::ptr()->assert_failure(#condition, __LINE__, __FILE__)) { \
        return return_value; \
      } \
    } \
  }

#define nassertv(condition) \
  { \
    if (_nassert_check(condition)) { \
      if (Notify::ptr()->assert_failure(#condition, __LINE__, __FILE__)) { \
        return; \
      } \
    } \
  }

#define nassertd(condition) \
  if (_nassert_check(condition) && \
      Notify::ptr()->assert_failure(#condition, __LINE__, __FILE__))

#define nassertr_always(condition, return_value) nassertr(condition, return_value)
#define nassertv_always(condition) nassertv(condition)

#define nassert_raise(message) Notify::ptr()->assert_failure(message, __LINE__, __FILE__)

#endif  // NDEBUG

#if __cplusplus >= 201103
#define __nassert_static(condition, line, file) static_assert((condition), #condition " at line " #line " of " file)
#define _nassert_static(condition, line, file) __nassert_static(condition, line, file)
#define nassert_static(condition) _nassert_static(condition, __LINE__, __FILE__)
#else
#define __nassert_static(condition, suffix) typedef char nassert_static_ ## suffix [(condition) ? 1 : -1];
#define _nassert_static(condition, suffix) __nassert_static(condition, suffix)
#define nassert_static(condition) _nassert_static(condition, __COUNTER__)
#endif

#include "pnotify.I"

#endif
