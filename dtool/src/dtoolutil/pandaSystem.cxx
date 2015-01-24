// Filename: pandaSystem.cxx
// Created by:  drose (26Jan05)
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

#include "pandaSystem.h"
#include "pandaVersion.h"
#include "dtool_platform.h"

PandaSystem *PandaSystem::_global_ptr = NULL;
TypeHandle PandaSystem::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::Constructor
//       Access: Protected
//  Description: Don't try to construct a PandaSystem object; there is
//               only one of these, and it constructs itself.  Use
//               get_global_ptr() to get a pointer to the one
//               PandaSystem.
////////////////////////////////////////////////////////////////////
PandaSystem::
PandaSystem() :
  _systems(get_class_type())
{
  _system_names_dirty = false;

  // These are settable via Config.prc, but only in development
  // (!NDEBUG) mode, and only if they are not already defined.
  _package_version_string = PANDA_PACKAGE_VERSION_STR;
  _package_host_url = PANDA_PACKAGE_HOST_URL;

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
#endif  // HAVE_EIGEN

#ifdef USE_MEMORY_DLMALLOC
  set_system_tag("system", "malloc", "dlmalloc");
#elif defined(USE_MEMORY_PTMALLOC2)
  set_system_tag("system", "malloc", "ptmalloc2");
#else
  set_system_tag("system", "malloc", "malloc");
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::Destructor
//       Access: Protected
//  Description: Don't try to destruct the global PandaSystem object.
////////////////////////////////////////////////////////////////////
PandaSystem::
~PandaSystem() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_version_string
//       Access: Published, Static
//  Description: Returns the current version of Panda, expressed as a
//               string, e.g. "1.0.0".  The string will end in the
//               letter "c" if this build does not represent an
//               official version.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_version_string() {
  return PANDA_VERSION_STR;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_package_version_string
//       Access: Published, Static
//  Description: Returns the version of the Panda3D distributable
//               package that provides this build of Panda.
//
//               When the currently-executing version of Panda was
//               loaded from a distributable package, such as via the
//               browser plugin, then this string will be nonempty and
//               will contain the corresponding version string.  You
//               can build applications that use this particular
//               version of Panda by requesting it in the pdef file,
//               using "panda3d", this version string, and the
//               download host provided by get_package_host_url().
//
//               If this string is empty, then the currently-executing
//               Panda was built independently, and is not part of a
//               distributable package.
//
//               This string is set explicitly at compilation time.
//               Normally, it should be set to a nonempty string only
//               when building a Panda3D package for distribution.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_package_version_string() {
#ifdef NDEBUG
  return PANDA_PACKAGE_VERSION_STR;
#else
  return get_global_ptr()->_package_version_string;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_package_host_url
//       Access: Published, Static
//  Description: Returns the URL of the download server that provides
//               the Panda3D distributable package currently running.
//               This can be used, along with the
//               get_package_version_string(), to uniquely identify
//               the running version of Panda among distributable
//               Panda versions.
//
//               See get_package_version_string() for more information.
//
//               This string is set explicitly at compilation time.
//               Normally, it should be set to a nonempty string only
//               when building a Panda3D package for distribution.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_package_host_url() {
#ifdef NDEBUG
  return PANDA_PACKAGE_HOST_URL;
#else
  return get_global_ptr()->_package_host_url;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_p3d_coreapi_version_string
//       Access: Published, Static
//  Description: Returns the current version of Panda's Core API,
//               expressed as a string of dot-delimited integers.
//               There are usually four integers in this version, but
//               this is not guaranteed.
//
//               The Core API is used during the runtime (plugin)
//               environment only.  This may be the empty string if
//               the current version of Panda is not built to provide
//               a particular Core API, which will be the normal case
//               in a development SDK.  However, you should not use
//               this method to determine whether you are running in a
//               runtime environment or not.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_p3d_coreapi_version_string() {
#ifndef P3D_COREAPI_VERSION_STR
  return "";
#else
  return P3D_COREAPI_VERSION_STR;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_major_version
//       Access: Published, Static
//  Description: Returns the major version number of the current
//               version of Panda.  This is the first number of the
//               dotted triple returned by get_version_string().  It
//               changes very rarely.
////////////////////////////////////////////////////////////////////
int PandaSystem::
get_major_version() {
  return PANDA_MAJOR_VERSION;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_minor_version
//       Access: Published, Static
//  Description: Returns the minor version number of the current
//               version of Panda.  This is the second number of the
//               dotted triple returned by get_version_string().  It
//               changes with each release that introduces new
//               features.
////////////////////////////////////////////////////////////////////
int PandaSystem::
get_minor_version() {
  return PANDA_MINOR_VERSION;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_sequence_version
//       Access: Published, Static
//  Description: Returns the sequence version number of the current
//               version of Panda.  This is the third number of the
//               dotted triple returned by get_version_string().  It
//               changes with bugfix updates and very minor feature
//               updates.
////////////////////////////////////////////////////////////////////
int PandaSystem::
get_sequence_version() {
  return PANDA_SEQUENCE_VERSION;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::is_official_version
//       Access: Published, Static
//  Description: Returns true if current version of Panda claims to be
//               an "official" version, that is, one that was compiled
//               by an official distributor of Panda using a specific
//               version of the panda source tree.  If this is true,
//               there will not be a "c" at the end of the version
//               string returned by get_version_string().
//
//               Note that we must take the distributor's word for it
//               here.
////////////////////////////////////////////////////////////////////
bool PandaSystem::
is_official_version() {
#ifdef PANDA_OFFICIAL_VERSION
  return true;
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_distributor
//       Access: Published, Static
//  Description: Returns the string defined by the distributor of this
//               version of Panda, or "homebuilt" if this version was
//               built directly from the sources by the end-user.
//               This is a completely arbitrary string.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_distributor() {
  return PANDA_DISTRIBUTOR;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_compiler
//       Access: Published, Static
//  Description: Returns a string representing the compiler that was
//               used to generate this version of Panda, if it is
//               available, or "unknown" if it is not.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_compiler() {
#if defined(_MSC_VER)
  // MSVC defines this macro.  It's an integer; we need to format it.
  ostringstream strm;
  strm << "MSC v." << _MSC_VER;

  // We also get this suite of macros that tells us what the build
  // platform is.
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

#elif defined(__GNUC__)
  // GCC defines this simple macro.
  return "GCC " __VERSION__;

#else
  // For other compilers, you're on your own.
  return "unknown";
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_build_date
//       Access: Published, Static
//  Description: Returns a string representing the date and time at
//               which this version of Panda (or at least dtool) was
//               compiled, if available.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_build_date() {
  return __DATE__ " " __TIME__;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_git_commit
//       Access: Published, Static
//  Description: Returns a string representing the git commit hash
//               that this source tree is based on, or the empty
//               string if it has not been specified at build time.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_git_commit() {
#ifdef PANDA_GIT_COMMIT_STR
  return PANDA_GIT_COMMIT_STR;
#else
  return string();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_platform
//       Access: Published, Static
//  Description: Returns a string representing the runtime platform
//               that we are currently running on.  This will be
//               something like "win32" or "osx_i386" or
//               "linux_amd64".
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_platform() {
  return DTOOL_PLATFORM;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::has_system
//       Access: Published
//  Description: Returns true if the current version of Panda claims
//               to have the indicated subsystem installed, false
//               otherwise.  The set of available subsystems is
//               implementation defined.
////////////////////////////////////////////////////////////////////
bool PandaSystem::
has_system(const string &system) const {
  Systems::const_iterator si;
  si = _systems.find(system);
  return (si != _systems.end());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_num_systems
//       Access: Published
//  Description: Returns the number of Panda subsystems that have
//               registered themselves.  This can be used with
//               get_system() to iterate through the entire list of
//               available Panda subsystems.
////////////////////////////////////////////////////////////////////
int PandaSystem::
get_num_systems() const {
  return _systems.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_system
//       Access: Published
//  Description: Returns the nth Panda subsystem that has registered
//               itself.  This list will be sorted in alphabetical
//               order.
////////////////////////////////////////////////////////////////////
string PandaSystem::
get_system(int n) const {
  if (n < 0 || n >= (int)_systems.size()) {
    return string();
  }

  if (_system_names_dirty) {
    ((PandaSystem *)this)->reset_system_names();
  }

  return _system_names[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_system_tag
//       Access: Published
//  Description: Returns the value associated with the indicated tag
//               for the given system.  This provides a standard way
//               to query each subsystem's advertised capabilities.
//               The set of tags and values are per-system and
//               implementation-defined.
//
//               The return value is the empty string if the indicated
//               system is undefined or if does not define the
//               indicated tag.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::add_system
//       Access: Published
//  Description: Intended for use by each subsystem to register itself
//               at startup.
////////////////////////////////////////////////////////////////////
void PandaSystem::
add_system(const string &system) {
  bool inserted = _systems.insert(Systems::value_type(system, SystemTags(get_class_type()))).second;
  if (inserted) {
    _system_names_dirty = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::set_system_tag
//       Access: Published
//  Description: Intended for use by each subsystem to register its
//               set of capabilities at startup.
////////////////////////////////////////////////////////////////////
void PandaSystem::
set_system_tag(const string &system, const string &tag,
               const string &value) {
  pair<Systems::iterator, bool> result;
  result = _systems.insert(Systems::value_type(system, SystemTags(get_class_type())));
  if (result.second) {
    _system_names_dirty = true;
  }

  SystemTags &tags = (*result.first).second;
  tags[tag] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::heap_trim
//       Access: Published
//  Description: Attempts to release memory back to the system, if
//               possible.  The pad argument is the minimum amount of
//               unused memory to keep in the heap (against future
//               allocations).  Any memory above that may be released
//               to the system, reducing the memory size of this
//               process.  There is no guarantee that any memory may
//               be released.
//
//               Returns true if any memory was actually released,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PandaSystem::
heap_trim(size_t pad) {
  // This actually just vectors into _memory_hook, which isn't
  // published.  This method only exists on PandaSystem for the
  // convenience of Python programmers.
  return memory_hook->heap_trim(pad);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PandaSystem::
output(ostream &out) const {
  out << "Panda version " << get_version_string();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PandaSystem::
write(ostream &out) const {
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


////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::get_global_ptr
//       Access: Published, Static
//  Description: Returns the global PandaSystem object.
////////////////////////////////////////////////////////////////////
PandaSystem *PandaSystem::
get_global_ptr() {
  if (_global_ptr == (PandaSystem *)NULL) {
    _global_ptr = new PandaSystem;
  }

  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::reset_system_names
//       Access: Private
//  Description: Refills the _system_names vector, which is used for
//               get_system_name(), from the current set of available
//               system names.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::set_package_version_string
//       Access: Private
//  Description: Loads the value returned by
//               get_package_version_string().  This is intended to be
//               called by ConfigPageManager to preload the value from
//               the panda-package-version config variable, for
//               developer's convenience.  This has no effect if the
//               PANDA_PACKAGE_VERSION_STR configure variable is
//               defined at compilation time.  This also has no effect
//               in NDEBUG mode.
////////////////////////////////////////////////////////////////////
void PandaSystem::
set_package_version_string(const string &package_version_string) {
  _package_version_string = PANDA_PACKAGE_VERSION_STR;
  if (_package_version_string.empty()) {
    _package_version_string = package_version_string;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::set_package_host_url
//       Access: Private
//  Description: Loads the value returned by
//               get_package_host_url().  This is intended to be
//               called by ConfigPageManager to preload the value from
//               the panda-package-host-url config variable, for
//               developer's convenience.  This has no effect if the
//               PANDA_PACKAGE_HOST_URL configure variable is defined
//               at compilation time.  This also has no effect in
//               NDEBUG mode.
////////////////////////////////////////////////////////////////////
void PandaSystem::
set_package_host_url(const string &package_host_url) {
  _package_host_url = PANDA_PACKAGE_HOST_URL;
  if (_package_host_url.empty()) {
    _package_host_url = package_host_url;
  }
}
