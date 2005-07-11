// Filename: pandaSystem.cxx
// Created by:  drose (26Jan05)
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

#include "pandaSystem.h"
#include "pandaVersion.h"

PandaSystem *PandaSystem::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: PandaSystem::Constructor
//       Access: Protected
//  Description: Don't try to construct a PandaSystem object; there is
//               only one of these, and it constructs itself.  Use
//               get_global_ptr() to get a pointer to the one
//               PandaSystem.
////////////////////////////////////////////////////////////////////
PandaSystem::
PandaSystem() {
  _system_names_dirty = false;
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
#ifdef COMPILER
  // MSVC defines this macro.
  return COMPILER;

#elif defined(__GNUC__)
  // GCC defines this one.
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
  bool inserted = _systems.insert(Systems::value_type(system, SystemTags())).second;
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
  result = _systems.insert(Systems::value_type(system, SystemTags()));
  if (result.second) {
    _system_names_dirty = true;
  }

  SystemTags &tags = (*result.first).second;
  tags[tag] = value;
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
  out << *this << ":\n";

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
