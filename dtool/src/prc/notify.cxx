/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notify.cxx
 * @author drose
 * @date 2000-02-28
 */

#include "pnotify.h"
#include "notifyCategory.h"
#include "configPageManager.h"
#include "configVariableFilename.h"
#include "configVariableBool.h"
#include "filename.h"
#include "config_prc.h"
#include "patomic.h"

#include <ctype.h>

#ifdef BUILD_IPHONE
#include <fcntl.h>
#endif

#ifdef ANDROID
#include <sys/stat.h>
#include <android/log.h>
#include "androidLogStream.h"
#endif

#ifdef __EMSCRIPTEN__
#include "emscriptenLogStream.h"
#endif

#ifndef NDEBUG
#ifdef PHAVE_EXECINFO_H
#include <execinfo.h>  // for backtrace()
#include <dlfcn.h>
#include <cxxabi.h>
#endif

#ifdef _WIN32
#include <dbghelp.h>
#endif
#endif  // NDEBUG

using std::cerr;
using std::cout;
using std::ostream;
using std::ostringstream;
using std::string;

Notify *Notify::_global_ptr = nullptr;

/**
 *
 */
Notify::
Notify() {
  _ostream_ptr = &std::cerr;
  _owns_ostream_ptr = false;
  _null_ostream_ptr = new std::fstream;

  _assert_handler = nullptr;
  _assert_failed = false;
}

/**
 *
 */
Notify::
~Notify() {
  if (_owns_ostream_ptr) {
    delete _ostream_ptr;
  }
  delete _null_ostream_ptr;
}

/**
 * Changes the ostream that all subsequent Notify messages will be written to.
 * If the previous ostream was set with delete_later = true, this will delete
 * the previous ostream.  If ostream_ptr is NULL, this resets the default to
 * cerr.
 */
void Notify::
set_ostream_ptr(ostream *ostream_ptr, bool delete_later) {
  if (_owns_ostream_ptr && ostream_ptr != _ostream_ptr) {
    delete _ostream_ptr;
  }

  if (ostream_ptr == nullptr) {
    _ostream_ptr = &cerr;
    _owns_ostream_ptr = false;
  } else {
    _ostream_ptr = ostream_ptr;
    _owns_ostream_ptr = delete_later;
  }
}

/**
 * Returns the system-wide ostream for all Notify messages.
 */
ostream *Notify::
get_ostream_ptr() const {
  return _ostream_ptr;
}

/**
 * Returns a flag that may be set on the Notify stream via setf() that, when
 * set, enables "literal" mode, which means the Notify stream will not attempt
 * to do any fancy formatting (like word-wrapping).
 *
 * Notify does not itself respect this flag; this is left up to the ostream
 * that Notify writes to.  Note that Notify just maps to cerr by default, in
 * which case this does nothing.  But the flag is available in case any
 * extended types want to make use of it.
 */
ios_fmtflags Notify::
get_literal_flag() {
  static bool got_flag = false;
  static ios_fmtflags flag;

  if (!got_flag) {
#ifndef PHAVE_IOSTREAM
    flag = std::ios::bitalloc();
#else
    // We lost bitalloc in the new iostream?  Ok, this feature will just be
    // disabled for now.  No big deal.
    flag = (ios_fmtflags)0;
#endif
    got_flag = true;
  }

  return flag;
}

/**
 * Sets a pointer to a C function that will be called when an assertion test
 * fails.  This function may decide what to do when that happens: it may
 * choose to abort or return.  If it returns, it should return true to
 * indicate that the assertion should be respected (and the calling function
 * should return out of its block of code), or false to indicate that the
 * assertion should be completely ignored.
 *
 * If an assert handler is installed, it completely replaces the default
 * behavior of nassertr() and nassertv().
 */
void Notify::
set_assert_handler(Notify::AssertHandler *assert_handler) {
  _assert_handler = assert_handler;
}

/**
 * Removes the installed assert handler and restores default behavior of
 * nassertr() and nassertv().
 */
void Notify::
clear_assert_handler() {
  _assert_handler = nullptr;
}

/**
 * Returns true if a user assert handler has been installed, false otherwise.
 */
bool Notify::
has_assert_handler() const {
  return (_assert_handler != nullptr);
}

/**
 * Returns a pointer to the user-installed assert handler, if one was
 * installed, or NULL otherwise.
 */
Notify::AssertHandler *Notify::
get_assert_handler() const {
  return _assert_handler;
}

/**
 * Returns the topmost Category in the hierarchy.  This may be used to
 * traverse the hierarchy of available Categories.
 */
NotifyCategory *Notify::
get_top_category() {
  return get_category(string());
}

/**
 * Finds or creates a new Category given the basename of the category and its
 * parent in the category hierarchy.  The parent pointer may be NULL to
 * indicate this is a top-level Category.
 */
NotifyCategory *Notify::
get_category(const string &basename, NotifyCategory *parent_category) {
  // The string should not contain colons.
  nassertr(basename.find(':') == string::npos, nullptr);

  string fullname;
  if (parent_category != nullptr) {
    fullname = parent_category->get_fullname() + ":" + basename;
  } else {
    // The parent_category is NULL.  If basename is empty, that means we refer
    // to the very top-level category (with an empty fullname); otherwise,
    // it's a new category just below that top level.
    if (!basename.empty()) {
      parent_category = get_top_category();
      fullname = ":" + basename;
    }
  }

  std::pair<Categories::iterator, bool> result =
    _categories.insert(Categories::value_type(fullname, nullptr));

  bool inserted = result.second;
  NotifyCategory *&category = (*result.first).second;

  if (inserted) {
    // If we just inserted a new record, then we have to create a new Category
    // pointer.  Otherwise, there was already one created from before.
    category = new NotifyCategory(fullname, basename, parent_category);
  }

  return category;
}

/**
 * Finds or creates a new Category given the basename of the category and the
 * fullname of its parent.  This is another way to create a category when you
 * don't have a pointer to its parent handy, but you know the name of its
 * parent.  If the parent Category does not already exist, it will be created.
 */
NotifyCategory *Notify::
get_category(const string &basename, const string &parent_fullname) {
  return get_category(basename, get_category(parent_fullname));
}

/**
 * Finds or creates a new Category given the fullname of the Category.  This
 * name should be a sequence of colon-separated names of parent Categories,
 * ending in the basename of this Category, e.g.  display:glxdisplay.  This is
 * a shorthand way to define a Category when a pointer to its parent is not
 * handy.
 */
NotifyCategory *Notify::
get_category(const string &fullname) {
  Categories::const_iterator ci;
  ci = _categories.find(fullname);
  if (ci != _categories.end()) {
    return (*ci).second;
  }

  // No such Category; create one.  First identify the parent name, based on
  // the rightmost colon.
  NotifyCategory *parent_category = nullptr;
  string basename = fullname;

  size_t colon = fullname.rfind(':');
  if (colon != string::npos) {
    parent_category = get_category(fullname.substr(0, colon));
    basename = fullname.substr(colon + 1);

  } else if (!fullname.empty()) {
    // The fullname didn't begin with a colon.  Infer one.
    parent_category = get_top_category();
  }

  return get_category(basename, parent_category);
}

/**
 * A convenient way to get the ostream that should be written to for a Notify-
 * type message of a particular severity.  Also see Category::out() for a
 * message that is specific to a particular Category.
 */
ostream &Notify::
out(NotifySeverity severity) {
#if defined(ANDROID) || defined(__EMSCRIPTEN__)
  // Android and JavaScript have dedicated log systems.
  return *(ptr()->_log_streams[severity]);
#else
  return *(ptr()->_ostream_ptr);
#endif
}

/**
 * A convenient way to get the ostream that should be written to for a Notify-
 * type message.  Also see Category::out() for a message that is specific to a
 * particular Category.
 */
ostream &Notify::
out() {
  return *(ptr()->_ostream_ptr);
}

/**
 * A convenient way to get an ostream that doesn't do anything.  Returned by
 * Category::out() when a particular Category and/or Severity is disabled.
 */
ostream &Notify::
null() {
  return *(ptr()->_null_ostream_ptr);
}

/**
 * A convenient way for scripting languages, which may know nothing about
 * ostreams, to write to Notify.  This writes a single string, followed by an
 * implicit newline, to the Notify output stream.
 */
void Notify::
write_string(const string &str) {
  out() << str << "\n";
}

/**
 * Returns the pointer to the global Notify object.  There is only one of
 * these in the world.
 */
Notify *Notify::
ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new Notify;
  }
  return _global_ptr;
}

/**
 * This function is not intended to be called directly by user code.  It's
 * called from the nassertr() and assertv() macros when an assertion test
 * fails; it handles the job of printing the warning message and deciding what
 * to do about it.
 *
 * If this function returns true, the calling function should return out of
 * its function; if it returns false, the calling function should ignore the
 * assertion.
 */
bool Notify::
assert_failure(const string &expression, int line,
               const char *source_file) {
  return assert_failure(expression.c_str(), line, source_file);
}

/**
 * This function is not intended to be called directly by user code.  It's
 * called from the nassertr() and assertv() macros when an assertion test
 * fails; it handles the job of printing the warning message and deciding what
 * to do about it.
 *
 * If this function returns true, the calling function should return out of
 * its function; if it returns false, the calling function should ignore the
 * assertion.
 */
bool Notify::
assert_failure(const char *expression, int line,
               const char *source_file) {
  ostringstream message_str;
  message_str
    << expression << " at line " << line << " of " << source_file;
  string message = message_str.str();

  Notify *self = ptr();
  if (!self->_assert_failed) {
    // We only save the first assertion failure message, as this is usually
    // the most meaningful when several occur in a row.
    self->_assert_failed = true;
    self->_assert_error_message = message;
  }

  if (self->has_assert_handler()) {
    return (*self->_assert_handler)(expression, line, source_file);
  }

#ifdef ANDROID
  // Write to Android log system.
  __android_log_assert("assert", "Panda3D", "Assertion failed: %s", message.c_str());
#endif

#ifdef __EMSCRIPTEN__
  // Write to JavaScript console.
  emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR | EM_LOG_C_STACK,
                 "Assertion failed: %s", message.c_str());

#else
  nout << "Assertion failed: " << message << "\n";
#endif

  // This is redefined here, shadowing the defining in config_prc.h, so we can
  // guarantee it has already been constructed.
  ALIGN_16BYTE ConfigVariableBool assert_abort("assert-abort", false);
  if (assert_abort) {
    // Make sure the error message has been flushed to the output.
    nout.flush();

    // Capture and list a stack trace.
#ifdef NDEBUG
#elif defined(PHAVE_EXECINFO_H)
    void *trace[64];
    int size = backtrace(trace, 64);
    if (size > 0) {
      // Remove the frame(s) corresponding to the current function.
      void **tracep = trace;
      void *return_addr = __builtin_return_address(0);
      for (int i = 0; i < size; ++i) {
        if (trace[i] == return_addr) {
          tracep = trace + (i + 1);
          size -= i + 1;
          break;
        }
      }
      write_backtrace(tracep, size);
    }
#elif defined(_WIN32)
    const ULONG max_size = 62;
    void *trace[max_size];
    int size = CaptureStackBackTrace(1, max_size, trace, nullptr);
    if (size > 0) {
      write_backtrace(trace, size);
    }
#endif

#ifdef _MSC_VER
    // How to trigger an exception in VC++ that offers to take us into the
    // debugger?  abort() doesn't do it.  We used to be able to assert(false),
    // but in VC++ 7 that just throws an exception, and an uncaught exception
    // just exits, without offering to open the debugger.

    // DebugBreak() seems to be provided for this purpose, but it doesn't seem
    // to work properly either, since we don't seem to get a reliable stack
    // trace.

    // The old reliable int 3 works (at least on an Intel platform) if you are
    // already running within a debugger.  But it doesn't offer to bring up a
    // debugger otherwise.

    // So we'll force a segfault, which works every time.
    int *ptr = nullptr;
    *ptr = 1;

#elif defined(__EMSCRIPTEN__)
    // This should drop us into the browser's JavaScript debugger.
    //emscripten_debugger();
    EM_ASM(debugger;);

#else  // _MSC_VER
    abort();
#endif  // _MSC_VER
  }

  return true;
}

/**
 *
 */
void Notify::
write_backtrace(void **trace, int size) {
  std::ostream &out = nout;

#ifdef NDEBUG
#elif defined(PHAVE_EXECINFO_H)
  char namebuf[128];

  for (int i = 0; i < size; ++i) {
    void *addr = trace[i];
    Dl_info info;
    if (dladdr((char *)addr - 1, &info) != 0) {
      const char *name = nullptr;

      if (info.dli_sname != nullptr) {
        int status = 0;
        size_t size = 0;
        char *demangled = nullptr;
        if (info.dli_sname[0] == '_') {
          size = sizeof(namebuf) - 1;
          demangled = abi::__cxa_demangle(info.dli_sname, namebuf, &size, &status);
        }
        if (status == 0 && demangled != nullptr) {
          namebuf[size] = 0;
          name = demangled;
          //if (strncmp(name, "Notify::assert_failure", 22) == 0) {
            //continue;
          //}
        } else {
          name = info.dli_sname;
        }
      }

      out << "[" << addr << "] ";

      if (info.dli_fname != nullptr) {
        const char *slash = strrchr(info.dli_fname, '/');
        out << std::left << std::setw(30) << (slash ? slash + 1 : info.dli_fname) << " ";
      }

      if (name != nullptr) {
        out << name;
      } else {
        out << info.dli_saddr;
      }

      int offset = reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(info.dli_saddr);
      if (offset > 0) {
        out << " + " << offset;
      }
      out << "\n";
    } else {
      out << "[" << addr << "]\n";
    }
  }
#elif defined(_WIN32)
  HMODULE handle = LoadLibraryA("dbghelp.dll");
  if (!handle) {
    return;
  }

  auto pSymInitialize = (BOOL (WINAPI *)(HANDLE, PCSTR, BOOL))GetProcAddress(handle, "SymInitialize");
  auto pSymCleanup = (BOOL (WINAPI *)(HANDLE))GetProcAddress(handle, "SymCleanup");
  auto pSymFromAddr = (BOOL (WINAPI *)(HANDLE, DWORD64, DWORD64 *, PSYMBOL_INFO))GetProcAddress(handle, "SymFromAddr");
  auto pSymGetModuleInfo64 = (BOOL (WINAPI *)(HANDLE, DWORD64, PIMAGEHLP_MODULE64))GetProcAddress(handle, "SymGetModuleInfo64");
  auto pSymGetLineFromAddr64 = (BOOL (WINAPI *)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64))GetProcAddress(handle, "SymGetLineFromAddr64");
  if (!pSymInitialize || !pSymCleanup || !pSymFromAddr) {
    FreeLibrary(handle);
    return;
  }

  HANDLE process = GetCurrentProcess();
  pSymInitialize(process, nullptr, TRUE);

  for (int i = 0; i < size; ++i) {
    DWORD64 addr = (DWORD64)trace[i];

    alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + 256] = {0};
    SYMBOL_INFO *symbol = (SYMBOL_INFO *)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;

    out << "[" << (void *)addr << "]";

    // Show the name of the library.
    IMAGEHLP_MODULE64 module;
    module.SizeOfStruct = sizeof(module);
    const char *basename = "???";
    if (pSymGetModuleInfo64 && pSymGetModuleInfo64(process, addr, &module)) {
      const char *slash = strrchr(module.ImageName, '\\');
      basename = (slash ? slash + 1 : module.ImageName);
    }

    // Look up the symbol name.  If the reported name comes from the export
    // table and the address is way off, ignore it, it's probably not right.
    if (pSymFromAddr(process, addr, nullptr, symbol) &&
        ((symbol->Flags & SYMFLAG_EXPORT) == 0 || addr - (DWORD64)symbol->Address < 0x4000)) {

      out << " " << std::left << std::setw(30) << basename << " " << symbol->Name;

      DWORD64 offset = addr - (DWORD64)(symbol->Address);
      if (offset > 0) {
        out << " + " << offset;
      }

      // Look up filename + line number if available.
      IMAGEHLP_LINE64 line;
      DWORD displacement = 0;
      line.SizeOfStruct = sizeof(line);
      if (pSymGetLineFromAddr64(process, addr, &displacement, &line)) {
        const char *slash = strrchr(line.FileName, '\\');
        const char *basename = (slash ? slash + 1 : line.FileName);

        out << " (" << basename << ":" << line.LineNumber;
        //if (displacement > 0) {
        //  out << " + " << displacement;
        //}
        out << ")";
      }
    } else {
      out << " " << basename;
    }
    out << "\n";
  }

  pSymCleanup(process);
  FreeLibrary(handle);
#endif

  out.flush();
}

/**
 * Given a string, one of "debug", "info", "warning", etc., return the
 * corresponding Severity level, or NS_unspecified if none of the strings
 * matches.
 */
NotifySeverity Notify::
string_severity(const string &str) {
  // Convert the string to lowercase for a case-insensitive comparison.
  string lstring;
  for (string::const_iterator si = str.begin();
       si != str.end();
       ++si) {
    lstring += (char)tolower(*si);
  }

  if (lstring == "spam") {
    return NS_spam;

  } else if (lstring == "debug") {
    return NS_debug;

  } else if (lstring == "info") {
    return NS_info;

  } else if (lstring == "warning") {
    return NS_warning;

  } else if (lstring == "error") {
    return NS_error;

  } else if (lstring == "fatal") {
    return NS_fatal;

  } else {
    return NS_unspecified;
  }
}

/**
 * Intended to be called only by Config, this is a callback that indicates to
 * Notify when Config has done initializing and Notify can safely set up some
 * internal state variables that depend on Config variables.
 */
void Notify::
config_initialized() {
  // We allow this to be called more than once to allow the user to specify a
  // notify-output even after the initial import of Panda3D modules.  However,
  // it cannot be changed after the first time it is set.

#if defined(__EMSCRIPTEN__)
  // We have no writable filesystem in JavaScript.  Instead, we set up a
  // special stream that logs straight into the Javascript console.

  EmscriptenLogStream *error_stream = new EmscriptenLogStream(EM_LOG_CONSOLE | EM_LOG_ERROR);
  EmscriptenLogStream *warn_stream = new EmscriptenLogStream(EM_LOG_CONSOLE | EM_LOG_WARN);
  EmscriptenLogStream *info_stream = new EmscriptenLogStream(EM_LOG_CONSOLE);

  Notify *ptr = Notify::ptr();
  ptr->_log_streams[NS_unspecified] = info_stream;
  ptr->_log_streams[NS_spam] = info_stream;
  ptr->_log_streams[NS_debug] = info_stream;
  ptr->_log_streams[NS_info] = info_stream;
  ptr->_log_streams[NS_warning] = warn_stream;
  ptr->_log_streams[NS_error] = error_stream;
  ptr->_log_streams[NS_fatal] = error_stream;

#else
  if (_global_ptr == nullptr || _global_ptr->_ostream_ptr == &cerr) {
    static ConfigVariableFilename notify_output
      ("notify-output", "",
       "The filename to which to write all the output of notify");

    // We use this to ensure that only one thread can initialize the output.
    static patomic_flag initialized = ATOMIC_FLAG_INIT;

    std::string value = notify_output.get_value();
    if (!value.empty() && !initialized.test_and_set()) {
      Notify *ptr = Notify::ptr();

      if (value == "stdout") {
        cout.setf(std::ios::unitbuf);
        ptr->set_ostream_ptr(&cout, false);

      } else if (value == "stderr") {
        ptr->set_ostream_ptr(&cerr, false);

      } else {
        Filename filename = value;
        filename.set_text();
#ifdef BUILD_IPHONE
        // On the iPhone, route everything through cerr, and then send cerr to
        // the log file, since we can't get the cerr output otherwise.
        string os_specific = filename.to_os_specific();
        int logfile_fd = open(os_specific.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (logfile_fd < 0) {
          nout << "Unable to open file " << filename << " for output.\n";
        } else {
          dup2(logfile_fd, STDOUT_FILENO);
          dup2(logfile_fd, STDERR_FILENO);
          close(logfile_fd);

          ptr->set_ostream_ptr(&cerr, false);
        }
#else
        pofstream *out = new pofstream;
        if (!filename.open_write(*out)) {
          nout << "Unable to open file " << filename << " for output.\n";
          delete out;
        } else {
          out->setf(std::ios::unitbuf);
          ptr->set_ostream_ptr(out, true);
        }
#endif  // BUILD_IPHONE
      }

#ifdef ANDROID
      for (int severity = 0; severity <= NS_fatal; ++severity) {
        ptr->_log_streams[severity] = ptr->_ostream_ptr;
      }

    } else {
      // By default, we always redirect the notify stream to the Android log,
      // except if we are running from the adb shell.  We decide this based
      // on whether stderr is redirected to /dev/null.
      Notify *ptr = Notify::ptr();
      struct stat a, b;
      if (fstat(STDERR_FILENO, &a) == 0 && stat("/dev/null", &b) == 0 &&
          a.st_dev == b.st_dev && a.st_ino == b.st_ino) {
        // Android redirects stdio and stderr to /dev/null,
        // but does provide its own logging system.  We use a special
        // type of stream that redirects it to Android's log system.
        for (int severity = 0; severity <= NS_fatal; ++severity) {
          int priority = ANDROID_LOG_UNKNOWN;
          if (severity != NS_unspecified) {
            priority = severity + 1;
          }
          ptr->_log_streams[severity] = new AndroidLogStream(priority);
        }
        ptr->set_ostream_ptr(new AndroidLogStream(ANDROID_LOG_INFO), true);
      } else {
        // Running from the terminal, set all the log streams to point to the
        // same output.
        for (int severity = 0; severity <= NS_fatal; ++severity) {
          ptr->_log_streams[severity] = &cerr;
        }
      }
#endif
    }
  }
#endif
}
