/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file executionEnvironment.cxx
 * @author drose
 * @date 2000-05-15
 */

#include "executionEnvironment.h"
#include "pandaVersion.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>  // for perror

using std::cerr;
using std::string;

#ifdef __APPLE__
#include <sys/param.h>  // for realpath
#endif  // __APPLE__

#ifdef WIN32_VC
// Windows requires this for getcwd().
#include <direct.h>
#define getcwd _getcwd

// And this is for GetModuleFileName().
#include <windows.h>

// And this is for CommandLineToArgvW.
#include <shellapi.h>

// SHGetSpecialFolderPath()
#include <shlobj.h>
#endif

#ifdef __APPLE__
// This is for _NSGetExecutablePath() and _NSGetEnviron().
#include <mach-o/dyld.h>
#ifndef BUILD_IPHONE
#include <crt_externs.h>  // For some reason, not in the IPhone SDK.
#endif
#define environ (*_NSGetEnviron())
#endif

#ifdef IS_LINUX
// extern char **environ is defined here:
#include <unistd.h>
#endif

#ifdef IS_FREEBSD
extern char **environ;

// This is for sysctl.
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(IS_LINUX) || defined(IS_FREEBSD)
// For link_map and dlinfo.
#include <link.h>
#include <dlfcn.h>
#endif

// We define the symbol PREREAD_ENVIRONMENT if we cannot rely on getenv() to
// read environment variables at static init time.  In this case, we must read
// all of the environment variables directly and cache them locally.

#ifndef STATIC_INIT_GETENV
#define PREREAD_ENVIRONMENT
#endif

// We define the symbol HAVE_GLOBAL_ARGV if we have global variables named
// GLOBAL_ARGCGLOBAL_ARGV that we can read at static init time to determine
// our command-line arguments.

#if !defined(WIN32_VC) && defined(HAVE_GLOBAL_ARGV) && defined(PROTOTYPE_GLOBAL_ARGV)
extern char **GLOBAL_ARGV;
extern int GLOBAL_ARGC;
#endif

// One of the responsibilities of ExecutionEnvironment is to determine the path
// to the binary file that contains itself (this is useful for making other
// components able to read files relative to Panda's installation directory).
// When built statically, this is easy - just use the main executable filename.
// When built shared, ExecutionEnvironment will introspect the memory map of
// the running process to look for dynamic library paths matching this list of
// predetermined filenames (ordered most likely to least likely).

#ifndef LINK_ALL_STATIC
static const char *const libp3dtool_filenames[] = {
#if defined(LIBP3DTOOL_FILENAMES)

  // The build system is communicating the expected filename(s) for the
  // libp3dtool dynamic library - no guesswork needed.
  LIBP3DTOOL_FILENAMES

#elif defined(WIN32_VC)

#ifdef _DEBUG
  "libp3dtool_d.dll",
#else
  "libp3dtool.dll",
#endif

#elif defined(__APPLE__)

  "libp3dtool." PANDA_ABI_VERSION_STR ".dylib",
  "libp3dtool.dylib",

#else

  "libp3dtool.so." PANDA_ABI_VERSION_STR,
  "libp3dtool.so",

#endif
};
#endif /* !LINK_ALL_STATIC */

// Linux with GNU libc does have global argvargc variables, but we can't
// safely access them at stat init time--at least, not in libc5. (It does seem
// to work with glibc2, however.)

ExecutionEnvironment *ExecutionEnvironment::_global_ptr = nullptr;

/**
 * You shouldn't need to construct one of these; there's only one and it
 * constructs itself.
 */
ExecutionEnvironment::
ExecutionEnvironment() {
  read_environment_variables();
  read_args();
}

/**
 * Reads the string, looking for environment variable names marked by a $.
 * Expands all such variable names.  A repeated dollar sign ($$) is mapped to
 * a single dollar sign.
 *
 * Returns the expanded string.
 */
string ExecutionEnvironment::
expand_string(const string &str) {
  string result;

  size_t last = 0;
  size_t dollar = str.find('$');
  while (dollar != string::npos && dollar + 1 < str.length()) {
    size_t start = dollar + 1;

    if (str[start] == '$') {
      // A double dollar sign maps to a single dollar sign.
      result += str.substr(last, start - last);
      last = start + 1;

    } else {
      string varname;
      size_t end = start;

      if (str[start] == '{') {
        // Curly braces delimit the variable name explicitly.
        end = str.find('}', start + 1);
        if (end != string::npos) {
          varname = str.substr(start + 1, end - (start + 1));
          end++;
        }
      }

      if (end == start) {
        // Scan for the end of the variable name.
        while (end < str.length() && (isalnum(str[end]) || str[end] == '_')) {
          end++;
        }
        varname = str.substr(start, end - start);
      }

      string subst =
      result += str.substr(last, dollar - last);
      result += get_environment_variable(varname);
      last = end;
    }

    dollar = str.find('$', last);
  }

  result += str.substr(last);

  return result;
}

/**
 * Returns the name of the current working directory.
 */
Filename ExecutionEnvironment::
get_cwd() {
#ifdef WIN32_VC
  // getcwd() requires us to allocate a dynamic buffer and grow it on demand.
  static size_t bufsize = 1024;
  static wchar_t *buffer = nullptr;

  if (buffer == nullptr) {
    buffer = new wchar_t[bufsize];
  }

  while (_wgetcwd(buffer, bufsize) == nullptr) {
    if (errno != ERANGE) {
      perror("getcwd");
      return string();
    }
    delete[] buffer;
    bufsize = bufsize * 2;
    buffer = new wchar_t[bufsize];
    assert(buffer != nullptr);
  }

  Filename cwd = Filename::from_os_specific_w(buffer);
  cwd.make_true_case();
  return cwd;
#else  // WIN32_VC
  // getcwd() requires us to allocate a dynamic buffer and grow it on demand.
  static size_t bufsize = 1024;
  static char *buffer = nullptr;

  if (buffer == nullptr) {
    buffer = new char[bufsize];
  }

  while (getcwd(buffer, bufsize) == nullptr) {
    if (errno != ERANGE) {
      perror("getcwd");
      return string();
    }
    delete[] buffer;
    bufsize = bufsize * 2;
    buffer = new char[bufsize];
    assert(buffer != nullptr);
  }

  Filename cwd = Filename::from_os_specific(buffer);
  cwd.make_true_case();
  return cwd;
#endif  // WIN32_VC
}

/**
 * Returns true if the indicated environment variable is defined.  The
 * nonstatic implementation.
 */
bool ExecutionEnvironment::
ns_has_environment_variable(const string &var) const {
#ifdef PREREAD_ENVIRONMENT
  return _variables.count(var) != 0;
#else
  return getenv(var.c_str()) != nullptr;
#endif
}

/**
 * Returns the definition of the indicated environment variable, or the empty
 * string if the variable is undefined.  The nonstatic implementation.
 */
string ExecutionEnvironment::
ns_get_environment_variable(const string &var) const {
  EnvironmentVariables::const_iterator evi;
  evi = _variables.find(var);
  if (evi != _variables.end()) {
    return (*evi).second;
  }

  // Some special case variables.  We virtually stuff these values into the
  // Panda environment, shadowing whatever values they have in the true
  // environment, so they can be used in config files.
  if (var == "HOME") {
    return Filename::get_home_directory().to_os_specific();
  } else if (var == "TEMP") {
    return Filename::get_temp_directory().to_os_specific();
  } else if (var == "USER_APPDATA") {
    return Filename::get_user_appdata_directory().to_os_specific();
  } else if (var == "COMMON_APPDATA") {
    return Filename::get_common_appdata_directory().to_os_specific();
  } else if (var == "MAIN_DIR") {
    // Return the binary name's parent directory.  If we're running inside the
    // Python interpreter, this will be overridden by a setting from
    // panda3d/core.py.
    if (!_binary_name.empty()) {
      Filename main_dir (_binary_name);
      main_dir.make_absolute();
      return Filename(main_dir.get_dirname()).to_os_specific();
    }
  }

#ifndef PREREAD_ENVIRONMENT
  const char *def = getenv(var.c_str());
  if (def != nullptr) {
    return def;
  }
#endif

#ifdef _WIN32
  // On Windows only, we also simulate several standard folder names as
  // "environment" variables.  I know we're supposed to be using
  // KnownFolderID's these days, but those calls aren't compatible with XP, so
  // we'll continue to use SHGetSpecialFolderPath() until we're forced out of
  // it.

  static struct { int id; const char *name; } csidl_table[] = {
    { CSIDL_ADMINTOOLS, "ADMINTOOLS" },
    { CSIDL_ALTSTARTUP, "ALTSTARTUP" },
    { CSIDL_APPDATA, "APPDATA" },
    { CSIDL_BITBUCKET, "BITBUCKET" },
    { CSIDL_CDBURN_AREA, "CDBURN_AREA" },
    { CSIDL_COMMON_ADMINTOOLS, "COMMON_ADMINTOOLS" },
    { CSIDL_COMMON_ALTSTARTUP, "COMMON_ALTSTARTUP" },
    { CSIDL_COMMON_APPDATA, "COMMON_APPDATA" },
    { CSIDL_COMMON_DESKTOPDIRECTORY, "COMMON_DESKTOPDIRECTORY" },
    { CSIDL_COMMON_DOCUMENTS, "COMMON_DOCUMENTS" },
    { CSIDL_COMMON_FAVORITES, "COMMON_FAVORITES" },
    { CSIDL_COMMON_MUSIC, "COMMON_MUSIC" },
    { CSIDL_COMMON_OEM_LINKS, "COMMON_OEM_LINKS" },
    { CSIDL_COMMON_PICTURES, "COMMON_PICTURES" },
    { CSIDL_COMMON_PROGRAMS, "COMMON_PROGRAMS" },
    { CSIDL_COMMON_STARTMENU, "COMMON_STARTMENU" },
    { CSIDL_COMMON_STARTUP, "COMMON_STARTUP" },
    { CSIDL_COMMON_TEMPLATES, "COMMON_TEMPLATES" },
    { CSIDL_COMMON_VIDEO, "COMMON_VIDEO" },
    { CSIDL_COMPUTERSNEARME, "COMPUTERSNEARME" },
    { CSIDL_CONNECTIONS, "CONNECTIONS" },
    { CSIDL_CONTROLS, "CONTROLS" },
    { CSIDL_COOKIES, "COOKIES" },
    { CSIDL_DESKTOP, "DESKTOP" },
    { CSIDL_DESKTOPDIRECTORY, "DESKTOPDIRECTORY" },
    { CSIDL_DRIVES, "DRIVES" },
    { CSIDL_FAVORITES, "FAVORITES" },
    { CSIDL_FONTS, "FONTS" },
    { CSIDL_HISTORY, "HISTORY" },
    { CSIDL_INTERNET, "INTERNET" },
    { CSIDL_INTERNET_CACHE, "INTERNET_CACHE" },
    { CSIDL_LOCAL_APPDATA, "LOCAL_APPDATA" },
    { CSIDL_MYDOCUMENTS, "MYDOCUMENTS" },
    { CSIDL_MYMUSIC, "MYMUSIC" },
    { CSIDL_MYPICTURES, "MYPICTURES" },
    { CSIDL_MYVIDEO, "MYVIDEO" },
    { CSIDL_NETHOOD, "NETHOOD" },
    { CSIDL_NETWORK, "NETWORK" },
    { CSIDL_PERSONAL, "PERSONAL" },
    { CSIDL_PRINTERS, "PRINTERS" },
    { CSIDL_PRINTHOOD, "PRINTHOOD" },
    { CSIDL_PROFILE, "PROFILE" },
    { CSIDL_PROGRAM_FILES, "PROGRAM_FILES" },
    { CSIDL_PROGRAM_FILESX86, "PROGRAM_FILESX86" },
    { CSIDL_PROGRAM_FILES_COMMON, "PROGRAM_FILES_COMMON" },
    { CSIDL_PROGRAM_FILES_COMMONX86, "PROGRAM_FILES_COMMONX86" },
    { CSIDL_PROGRAMS, "PROGRAMS" },
    { CSIDL_RECENT, "RECENT" },
    { CSIDL_RESOURCES, "RESOURCES" },
    { CSIDL_RESOURCES_LOCALIZED, "RESOURCES_LOCALIZED" },
    { CSIDL_SENDTO, "SENDTO" },
    { CSIDL_STARTMENU, "STARTMENU" },
    { CSIDL_STARTUP, "STARTUP" },
    { CSIDL_SYSTEM, "SYSTEM" },
    { CSIDL_SYSTEMX86, "SYSTEMX86" },
    { CSIDL_TEMPLATES, "TEMPLATES" },
    { CSIDL_WINDOWS, "WINDOWS" },
    { 0, nullptr },
  };

  for (int i = 0; csidl_table[i].name != nullptr; ++i) {
    if (strcmp(var.c_str(), csidl_table[i].name) == 0) {
      wchar_t buffer[MAX_PATH];
      if (SHGetSpecialFolderPathW(nullptr, buffer, csidl_table[i].id, true)) {
        Filename pathname = Filename::from_os_specific_w(buffer);
        return pathname.to_os_specific();
      }
      break;
    }
  }

#elif !defined(__APPLE__)
  // Similarly, we define fallbacks on POSIX systems for the variables defined
  // in the XDG Base Directory specification, so that they can be safely used
  // in Config.prc files.
  if (var == "XDG_CONFIG_HOME") {
    Filename home_dir = Filename::get_home_directory();
    return home_dir.get_fullpath() + "/.config";

  } else if (var == "XDG_CACHE_HOME") {
    Filename home_dir = Filename::get_home_directory();
    return home_dir.get_fullpath() + "/.cache";

  } else if (var == "XDG_DATA_HOME") {
    Filename home_dir = Filename::get_home_directory();
    return home_dir.get_fullpath() + "/.local/share";
  }
#endif // _WIN32

  return string();
}

/**
 * Changes the definition of the indicated environment variable.  The
 * nonstatic implementation.
 */
void ExecutionEnvironment::
ns_set_environment_variable(const string &var, const string &value) {
  _variables[var] = value;
  string putstr = var + "=" + value;

  // putenv() requires us to malloc a new C-style string.
  char *put = (char *)malloc(putstr.length() + 1);
  strcpy(put, putstr.c_str());
  putenv(put);
}

/**
 *
 */
void ExecutionEnvironment::
ns_shadow_environment_variable(const string &var, const string &value) {
  _variables[var] = value;
  string putstr = var + "=" + value;
}

/**
 *
 */
void ExecutionEnvironment::
ns_clear_shadow(const string &var) {
  EnvironmentVariables::iterator vi = _variables.find(var);
  if (vi == _variables.end()) {
    return;
  }

#ifdef PREREAD_ENVIRONMENT
  // Now we have to replace the value in the table.
  const char *def = getenv(var.c_str());
  if (def != nullptr) {
    (*vi).second = def;
  } else {
    _variables.erase(vi);
  }
#endif  // PREREAD_ENVIRONMENT
}

/**
 * Returns the number of command-line arguments available, not counting arg 0,
 * the binary name.  The nonstatic implementation.
 */
size_t ExecutionEnvironment::
ns_get_num_args() const {
  return _args.size();
}

/**
 * Returns the nth command-line argument.  The index n must be in the range [0
 * .. get_num_args()).  The first parameter, n == 0, is the first actual
 * parameter, not the binary name.  The nonstatic implementation.
 */
string ExecutionEnvironment::
ns_get_arg(size_t n) const {
  assert(n < ns_get_num_args());
  return _args[n];
}

/**
 * Returns the name of the binary executable that started this program, if it
 * can be determined.  The nonstatic implementation.
 */
string ExecutionEnvironment::
ns_get_binary_name() const {
  if (_binary_name.empty()) {
    return "unknown";
  }
  return _binary_name;
}

/**
 * Returns the name of the libp3dtool DLL that is used in this program, if it
 * can be determined.  The nonstatic implementation.
 */
string ExecutionEnvironment::
ns_get_dtool_name() const {
  if (_dtool_name.empty()) {
    return "unknown";
  }
  return _dtool_name;
}

/**
 * Returns a static pointer that may be used to access the global
 * ExecutionEnvironment object.
 */
ExecutionEnvironment *ExecutionEnvironment::
get_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ExecutionEnvironment;
  }
  return _global_ptr;
}


/**
 * Fills up the internal table of existing environment variables, if we are in
 * PREREAD_ENVIRONMENT mode.  Otherwise, does nothing.
 */
void ExecutionEnvironment::
read_environment_variables() {
#ifdef PREREAD_ENVIRONMENT
#if defined(IS_OSX) || defined(IS_FREEBSD) || defined(IS_LINUX)
  // In the case of Mac, we'll try reading _NSGetEnviron(). In the case of
  // FreeBSD and Linux, use the "environ" variable.

  char **envp;
  for (envp = environ; envp && *envp; envp++) {
    string variable;
    string value;

    char *envc;
    for (envc = *envp; envc && *envc && strncmp(envc, "=", 1) != 0; envc++) {
      variable += (char) *envc;
    }

    if (strncmp(envc, "=", 1) == 0) {
      for (envc++; envc && *envc; envc++) {
        value += (char) *envc;
      }
    }

    if (!variable.empty()) {
      _variables[variable] = value;
    }
  }
#elif defined(HAVE_PROC_SELF_ENVIRON)
  // In some cases, we may have a file called procselfenviron that may be read
  // to determine all of our environment variables.

  pifstream proc("/proc/self/environ");
  if (proc.fail()) {
    cerr << "Cannot read /proc/self/environ; environment variables unavailable.\n";
    return;
  }

  int ch = proc.get();
  while (!proc.eof() && !proc.fail()) {
    string variable;
    string value;

    while (!proc.eof() && !proc.fail() && ch != '=' && ch != '\0') {
      variable += (char)ch;
      ch = proc.get();
    }

    if (ch == '=') {
      ch = proc.get();
      while (!proc.eof() && !proc.fail() && ch != '\0') {
        value += (char)ch;
        ch = proc.get();
      }
    }

    if (!variable.empty()) {
      _variables[variable] = value;
    }
    ch = proc.get();
  }
#else
  cerr << "Warning: environment variables unavailable to dconfig.\n";
#endif
#endif // PREREAD_ENVIRONMENT
}

/**
 * Reads all the command-line arguments and the name of the binary file, if
 * possible.
 */
void ExecutionEnvironment::
read_args() {
  // First, we need to fill in _dtool_name.  This contains the full path to
  // the p3dtool library.

#ifndef LINK_ALL_STATIC
#if defined(WIN32_VC)
  for (const char *filename : libp3dtool_filenames) {
    if (!_dtool_name.empty()) break;

    HMODULE dllhandle = GetModuleHandle(filename);
    if (!dllhandle) continue;

    static const DWORD buffer_size = 1024;
    wchar_t buffer[buffer_size];
    DWORD size = GetModuleFileNameW(dllhandle, buffer, buffer_size);
    if (size != 0) {
      Filename tmp = Filename::from_os_specific_w(std::wstring(buffer, size));
      tmp.make_true_case();
      _dtool_name = tmp;
    }
  }

#elif defined(__APPLE__)
  // And on OSX we don't have procselfmaps, but some _dyld_* functions.

  uint32_t ic = _dyld_image_count();
  for (uint32_t i = 0; i < ic; ++i) {
    if (!_dtool_name.empty()) break;

    const char *buffer = _dyld_get_image_name(i);
    if (!buffer) continue;
    const char *tail = strrchr(buffer, '/');
    if (!tail) continue;

    for (const char *filename : libp3dtool_filenames) {
      if (strcmp(&tail[1], filename) == 0) {
        _dtool_name = buffer;
        break;
      }
    }
  }

#elif defined(RTLD_DI_ORIGIN)
  // When building with glibc/uClibc, we typically have access to RTLD_DI_ORIGIN in Unix-like operating systems.

  char origin[PATH_MAX + 1];

  for (const char *filename : libp3dtool_filenames) {
    if (!_dtool_name.empty()) break;

    void *dtool_handle = dlopen(filename, RTLD_NOW | RTLD_NOLOAD);
    if (dtool_handle != nullptr && dlinfo(dtool_handle, RTLD_DI_ORIGIN, origin) != -1) {
      _dtool_name = origin;
      _dtool_name += '/';
      _dtool_name += filename;
    }
  }

#elif defined(RTLD_DI_LINKMAP)
  // On platforms without RTLD_DI_ORIGIN, we can use dlinfo with RTLD_DI_LINKMAP to get the origin of a loaded library.
  if (_dtool_name.empty()) {
    struct link_map *map;
#ifdef RTLD_SELF
    void *self = RTLD_SELF;
#else
    void *self = dlopen(NULL, RTLD_NOW | RTLD_NOLOAD);
#endif
    if (dlinfo(self, RTLD_DI_LINKMAP, &map)) {
      while (map != nullptr) {
        if (!_dtool_name.empty()) break;

        const char *head = strchr(map->l_name, '/');
        if (!head) continue;
        const char *tail = strrchr(head, '/');
        if (!tail) continue;

        for (const char *filename : libp3dtool_filenames) {
          if (strcmp(&tail[1], filename) == 0) {
            _dtool_name = head;
            break;
          }
        }

        map = map->l_next;
      }
    }
  }
#endif

#if defined(HAVE_PROC_SELF_MAPS) || defined(HAVE_PROC_CURPROC_MAP)
  // Some operating systems provide a file in the proc filesystem.

  if (_dtool_name.empty()) {
#ifdef HAVE_PROC_CURPROC_MAP
    pifstream maps("/proc/curproc/map");
#else
    pifstream maps("/proc/self/maps");
#endif
    while (!maps.fail() && !maps.eof()) {
      if (!_dtool_name.empty()) break;

      char buffer[PATH_MAX];
      buffer[0] = 0;
      maps.getline(buffer, PATH_MAX);
      const char *head = strchr(buffer, '/');
      if (!head) continue;
      const char *tail = strrchr(head, '/');
      if (!tail) continue;

      for (const char *filename : libp3dtool_filenames) {
        if (strcmp(&tail[1], filename) == 0) {
          _dtool_name = head;
          break;
        }
      }
    }
    maps.close();
  }
#endif
#endif /* !LINK_ALL_STATIC */

  // Now, we need to fill in _binary_name.  This contains the full path to the
  // currently running executable.

#ifdef WIN32_VC
  if (_binary_name.empty()) {
    static const DWORD buffer_size = 1024;
    wchar_t buffer[buffer_size];
    DWORD size = GetModuleFileNameW(nullptr, buffer, buffer_size);
    if (size != 0) {
      Filename tmp = Filename::from_os_specific_w(std::wstring(buffer, size));
      tmp.make_true_case();
      _binary_name = tmp;
    }
  }
#endif

#if defined(__APPLE__)
  // And on Mac, we have _NSGetExecutablePath.
  if (_binary_name.empty()) {
    char *pathbuf = new char[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (_NSGetExecutablePath(pathbuf, &bufsize) == 0) {
      _binary_name = pathbuf;
    }
    delete[] pathbuf;
  }
#endif

#if defined(IS_FREEBSD)
  // In FreeBSD, we can use sysctl to determine the pathname.

  if (_binary_name.empty()) {
    size_t bufsize = 4096;
    char buffer[4096];
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    mib[3] = getpid();
    if (sysctl(mib, 4, (void*) buffer, &bufsize, nullptr, 0) == -1) {
      perror("sysctl");
    } else {
      _binary_name = buffer;
    }
  }
#endif

#if defined(HAVE_PROC_SELF_EXE) || defined(HAVE_PROC_CURPROC_FILE)
  // Some operating systems provide a symbolic link to the executable in the
  // proc filesystem.  Use readlink to resolve that link.

  if (_binary_name.empty()) {
    char readlinkbuf [PATH_MAX];
#ifdef HAVE_PROC_CURPROC_FILE
    ssize_t pathlen = readlink("/proc/curproc/file", readlinkbuf, PATH_MAX - 1);
#else
    ssize_t pathlen = readlink("/proc/self/exe", readlinkbuf, PATH_MAX - 1);
#endif
    if (pathlen > 0) {
      readlinkbuf[pathlen] = 0;
      _binary_name = readlinkbuf;
    }
  }
#endif

  // Next we need to fill in _args, which is a vector containing the command-
  // line arguments that the executable was invoked with.

#if defined(WIN32_VC)

  // We cannot rely on __argv when Python is linked in Unicode mode.  Instead,
  // let's use GetCommandLine.

  LPWSTR cmdline = GetCommandLineW();
  int argc = 0;
  LPWSTR *wargv = CommandLineToArgvW(cmdline, &argc);

  if (wargv == nullptr) {
    cerr << "CommandLineToArgvW failed; command-line arguments unavailable to config.\n";

  } else {
    TextEncoder encoder;
    encoder.set_encoding(Filename::get_filesystem_encoding());

    for (int i = 0; i < argc; ++i) {
      std::wstring wtext(wargv[i]);
      encoder.set_wtext(wtext);

      if (i == 0) {
        if (_binary_name.empty()) {
          _binary_name = encoder.get_text();
        }
      } else {
        _args.push_back(encoder.get_text());
      }
    }

    LocalFree(wargv);
  }

#elif defined(IS_FREEBSD)
  // In FreeBSD, we can use sysctl to determine the command-line arguments.

  size_t bufsize = 4096;
  char buffer[4096];
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ARGS, 0};
  mib[3] = getpid();
  if (sysctl(mib, 4, (void*) buffer, &bufsize, nullptr, 0) == -1) {
    perror("sysctl");
  } else {
    if (_binary_name.empty()) {
      _binary_name = buffer;
    }
    size_t idx = strlen(buffer) + 1;
    while (idx < bufsize) {
      _args.push_back((char*)(buffer + idx));
      size_t newidx = strlen(buffer + idx);
      idx += newidx + 1;
    }
  }

#elif defined(HAVE_GLOBAL_ARGV)
  int argc = GLOBAL_ARGC;

  // On Windows, __argv can be NULL when the main entry point is compiled in
  // Unicode mode (as is the case with Python 3)
  if (GLOBAL_ARGV != nullptr) {
    if (_binary_name.empty() && argc > 0) {
      _binary_name = GLOBAL_ARGV[0];
      // This really needs to be resolved against PATH.
    }

    for (int i = 1; i < argc; i++) {
      _args.push_back(GLOBAL_ARGV[i]);
    }
  }

#elif defined(HAVE_PROC_SELF_CMDLINE) || defined(HAVE_PROC_CURPROC_CMDLINE)
  // In Linux, and possibly in other systems as well, we might not be able to
  // use the global ARGCARGV variables at static init time.  However, we may
  // be lucky and have a file called procselfcmdline that may be read to
  // determine all of our command-line arguments.

#ifdef HAVE_PROC_CURPROC_CMDLINE
  pifstream proc("/proc/curproc/cmdline");
  if (proc.fail()) {
    cerr << "Cannot read /proc/curproc/cmdline; command-line arguments unavailable to config.\n";
#else
  pifstream proc("/proc/self/cmdline");
  if (proc.fail()) {
    cerr << "Cannot read /proc/self/cmdline; command-line arguments unavailable to config.\n";
#endif
  } else {
    int ch = proc.get();
    int index = 0;
    while (!proc.eof() && !proc.fail()) {
      string arg;

      while (!proc.eof() && !proc.fail() && ch != '\0') {
        arg += (char)ch;
        ch = proc.get();
      }

      if (index == 0) {
        if (_binary_name.empty())
          _binary_name = arg;
      } else {
        _args.push_back(arg);
      }
      index++;

      ch = proc.get();
    }
  }
#endif

#ifndef _WIN32
  // Try to use realpath to get cleaner paths.

  if (!_binary_name.empty()) {
    char newpath [PATH_MAX + 1];
    if (realpath(_binary_name.c_str(), newpath) != nullptr) {
      _binary_name = newpath;
    }
  }

  if (!_dtool_name.empty()) {
    char newpath [PATH_MAX + 1];
    if (realpath(_dtool_name.c_str(), newpath) != nullptr) {
      _dtool_name = newpath;
    }
  }
#endif  // _WIN32

  if (_dtool_name.empty()) {
    _dtool_name = _binary_name;
  }
}
