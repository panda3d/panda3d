// Filename: pandaSystem.h
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

#ifndef PANDASYSTEM_H
#define PANDASYSTEM_H

#include "dtoolbase.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : PandaSystem
// Description : This class is used as a namespace to group several
//               global properties of Panda.  Application developers
//               can use this class to query the runtime version or
//               capabilities of the current Panda environment.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL PandaSystem {
protected:
  PandaSystem();
  ~PandaSystem();

PUBLISHED:
  static string get_version_string();

  static int get_major_version();
  static int get_minor_version();
  static int get_sequence_version();
  static bool is_official_version();
  
  static string get_distributor();

  bool has_system(const string &system) const;
  int get_num_systems() const;
  string get_system(int n) const;

  string get_system_tag(const string &system, const string &tag) const;

  void add_system(const string &system);
  void set_system_tag(const string &system, const string &tag,
                      const string &value);

  void output(ostream &out) const;
  void write(ostream &out) const;

  static PandaSystem *get_global_ptr();

private:
  void reset_system_names();

  typedef pmap<string, string> SystemTags;
  typedef pmap<string, SystemTags> Systems;
  typedef pvector<string> SystemNames;

  Systems _systems;
  SystemNames _system_names;
  bool _system_names_dirty;

  static PandaSystem *_global_ptr;
};

inline ostream &operator << (ostream &out, const PandaSystem &ps) {
  ps.output(out);
  return out;
}

#endif

  
