/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaSystem.cxx
 * @author drose
 * @date 2005-01-26
 */

#include "pandaSystem.h"
#include "pandaVersion.h"
#include "dtool_platform.h"

using std::string;

PandaSystem *PandaSystem::_global_ptr = nullptr;
TypeHandle PandaSystem::_type_handle;

/**
 * Don't try to construct a PandaSystem object; there is only one of these,
 * and it constructs itself.  Use get_global_ptr() to get a pointer to the one
 * PandaSystem.
 */
PandaSystem::
PandaSystem() :
  _systems(get_class_type())
{
  _system_names_dirty = false;

#ifdef STDFLOAT_DOUBLE
  add_system("stdfloat-double");
#endif

#ifdef HAVE_EIGEN
  add_system("eigen");
#ifdef LINMATH_ALIGN
  set_system_tag("eigen", "vectorize", "1");
#else
  set_system_tag("eigen", "vectorize", "0");
#endif
#ifdef __AVX__
  set_system_tag("eigen", "avx", "1");
#else
  set_system_tag("eigen", "avx", "0");
#endif
#endif  // HAVE_EIGEN

#ifdef USE_MEMORY_DLMALLOC
  set_system_tag("system", "malloc", "dlmalloc");
#elif defined(USE_MEMORY_PTMALLOC2)
  set_system_tag("system", "malloc", "ptmalloc2");
#elif defined(USE_MEMORY_MIMALLOC)
  set_system_tag("system", "malloc", "mimalloc");
#else
  set_system_tag("system", "malloc", "malloc");
#endif

#ifdef _LIBCPP_VERSION
  set_system_tag("system", "stdlib", "libc++");
#elif defined(__GLIBCXX__)
  set_system_tag("system", "stdlib", "libstdc++");
#endif
}

/**
 * Don't try to destruct the global PandaSystem object.
 */
PandaSystem::
~PandaSystem() {
}

/**
 * Returns the current version of Panda, expressed as a string, e.g.  "1.0.0".
 * The string will end in the letter "c" if this build does not represent an
 * official version.
 */
string PandaSystem::
get_version_string() {
  return PANDA_VERSION_STR;
}

/**
 * Returns the major version number of the current version of Panda.  This is
 * the first number of the dotted triple returned by get_version_string().  It
 * changes very rarely.
 */
int PandaSystem::
get_major_version() {
  return PANDA_MAJOR_VERSION;
}

/**
 * Returns the minor version number of the current version of Panda.  This is
 * the second number of the dotted triple returned by get_version_string().
 * It changes with each release that introduces new features.
 */
int PandaSystem::
get_minor_version() {
  return PANDA_MINOR_VERSION;
}

/**
 * Returns the sequence version number of the current version of Panda.  This
 * is the third number of the dotted triple returned by get_version_string().
 * It changes with bugfix updates and very minor feature updates.
 */
int PandaSystem::
get_sequence_version() {
  return PANDA_SEQUENCE_VERSION;
}

/**
 * Returns true if current version of Panda claims to be an "official"
 * version, that is, one that was compiled by an official distributor of Panda
 * using a specific version of the panda source tree.  If this is true, there
 * will not be a "c" at the end of the version string returned by
 * get_version_string().
 *
 * Note that we must take the distributor's word for it here.
 */
bool PandaSystem::
is_official_version() {
#ifdef PANDA_OFFICIAL_VERSION
  return true;
#else
  return false;
#endif
}

/**
 * Returns the memory alignment that Panda's allocators are using.
 */
int PandaSystem::
get_memory_alignment() {
  return MEMORY_HOOK_ALIGNMENT;
}

/**
 * Returns the string defined by the distributor of this version of Panda, or
 * "homebuilt" if this version was built directly from the sources by the end-
 * user.  This is a completely arbitrary string.
 */
string PandaSystem::
get_distributor() {
  return PANDA_DISTRIBUTOR;
}

/**
 * Returns a string representing the compiler that was used to generate this
 * version of Panda, if it is available, or "unknown" if it is not.
 */
string PandaSystem::
get_compiler() {
#if defined(_MSC_VER)
  // MSVC defines this macro.  It's an integer; we need to format it.
  std::ostringstream strm;
  strm << "MSC v." << _MSC_VER;

  // We also get this suite of macros that tells us what the build platform
  // is.
#if defined(_M_IX86)
  #ifdef MS_WIN64
  strm << " 64 bit (Intel)";
  #else  // MS_WIN64
  strm << " 32 bit (Intel)";
  #endif  // MS_WIN64
#elif defined(_M_IA64)
  strm << " 64 bit (Itanium)";
#elif defined(_M_AMD64)
  strm << " 64 bit (AMD64)";
#endif

  return strm.str();

#elif defined(__clang__)
  // Clang has this macro.  This case has to go before __GNUC__ because that
  // is also defined by clang.
  return "Clang " __clang_version__;

#elif defined(__GNUC__)
  // GCC defines this simple macro.
  return "GCC " __VERSION__;

#else
  // For other compilers, you're on your own.
  return "unknown";
#endif
}

/**
 * Returns a string representing the date and time at which this version of
 * Panda (or at least dtool) was compiled, if available.
 *
 * @deprecated
 */
string PandaSystem::
get_build_date() {
#ifdef PANDA_BUILD_DATE_STR
  return PANDA_BUILD_DATE_STR;
#else
  return __DATE__ " " __TIME__;
#endif
}

/**
 * Returns a string representing the git commit hash that this source tree is
 * based on, or the empty string if it has not been specified at build time.
 */
string PandaSystem::
get_git_commit() {
#ifdef PANDA_GIT_COMMIT_STR
  return PANDA_GIT_COMMIT_STR;
#else
  return string();
#endif
}

/**
 * Returns a string representing the runtime platform that we are currently
 * running on.  This will be something like "win32" or "osx_i386" or
 * "linux_amd64".
 */
string PandaSystem::
get_platform() {
  return DTOOL_PLATFORM;
}

/**
 * Returns true if the current version of Panda claims to have the indicated
 * subsystem installed, false otherwise.  The set of available subsystems is
 * implementation defined.
 */
bool PandaSystem::
has_system(const string &system) const {
  Systems::const_iterator si;
  si = _systems.find(system);
  return (si != _systems.end());
}

/**
 * Returns the number of Panda subsystems that have registered themselves.
 * This can be used with get_system() to iterate through the entire list of
 * available Panda subsystems.
 */
size_t PandaSystem::
get_num_systems() const {
  return _systems.size();
}

/**
 * Returns the nth Panda subsystem that has registered itself.  This list will
 * be sorted in alphabetical order.
 */
string PandaSystem::
get_system(size_t n) const {
  if (n >= _systems.size()) {
    return string();
  }

  if (_system_names_dirty) {
    ((PandaSystem *)this)->reset_system_names();
  }

  return _system_names[n];
}

/**
 * Returns the value associated with the indicated tag for the given system.
 * This provides a standard way to query each subsystem's advertised
 * capabilities.  The set of tags and values are per-system and
 * implementation-defined.
 *
 * The return value is the empty string if the indicated system is undefined
 * or if does not define the indicated tag.
 */
string PandaSystem::
get_system_tag(const string &system, const string &tag) const {
  Systems::const_iterator si;
  si = _systems.find(system);
  if (si != _systems.end()) {
    const SystemTags &tags = (*si).second;
    SystemTags::const_iterator ti;
    ti = tags.find(tag);
    if (ti != tags.end()) {
      return (*ti).second;
    }
  }

  return string();
}

/**
 * Intended for use by each subsystem to register itself at startup.
 */
void PandaSystem::
add_system(const string &system) {
  bool inserted = _systems.insert(Systems::value_type(system, SystemTags(get_class_type()))).second;
  if (inserted) {
    _system_names_dirty = true;
  }
}

/**
 * Intended for use by each subsystem to register its set of capabilities at
 * startup.
 */
void PandaSystem::
set_system_tag(const string &system, const string &tag,
               const string &value) {
  std::pair<Systems::iterator, bool> result;
  result = _systems.insert(Systems::value_type(system, SystemTags(get_class_type())));
  if (result.second) {
    _system_names_dirty = true;
  }

  SystemTags &tags = (*result.first).second;
  tags[tag] = value;
}

/**
 * Attempts to release memory back to the system, if possible.  The pad
 * argument is the minimum amount of unused memory to keep in the heap
 * (against future allocations).  Any memory above that may be released to the
 * system, reducing the memory size of this process.  There is no guarantee
 * that any memory may be released.
 *
 * Returns true if any memory was actually released, false otherwise.
 */
bool PandaSystem::
heap_trim(size_t pad) {
  // This actually just vectors into _memory_hook, which isn't published.
  // This method only exists on PandaSystem for the convenience of Python
  // programmers.
  return memory_hook->heap_trim(pad);
}

/**
 *
 */
void PandaSystem::
output(std::ostream &out) const {
  out << "Panda version " << get_version_string();
}

/**
 *
 */
void PandaSystem::
write(std::ostream &out) const {
  out << *this << "\n"
      << "compiled on " << get_build_date() << " by "
      << get_distributor() << "\n"
      << "with compiler " << PandaSystem::get_compiler() << "\n\n";

  out << "Optional systems:\n";
  for (Systems::const_iterator si = _systems.begin();
       si != _systems.end();
       ++si) {
    out << "  " << (*si).first << "\n";
    const SystemTags &tags = (*si).second;
    SystemTags::const_iterator ti;
    for (ti = tags.begin(); ti != tags.end(); ++ti) {
      out << "    " << (*ti).first << " " << (*ti).second << "\n";
    }
  }
}


/**
 * Returns the global PandaSystem object.
 */
PandaSystem *PandaSystem::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    init_type();
    _global_ptr = new PandaSystem;
  }

  return _global_ptr;
}

/**
 * Refills the _system_names vector, which is used for get_system_name(), from
 * the current set of available system names.
 */
void PandaSystem::
reset_system_names() {
  _system_names.clear();
  _system_names.reserve(_systems.size());

  Systems::const_iterator si;
  for (si = _systems.begin(); si != _systems.end(); ++si) {
    _system_names.push_back((*si).first);
  }

  _system_names_dirty = false;
}
