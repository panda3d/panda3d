/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaSystem.h
 * @author drose
 * @date 2005-01-26
 */

#ifndef PANDASYSTEM_H
#define PANDASYSTEM_H

#include "dtoolbase.h"
#include "pmap.h"
#include "pvector.h"

/**
 * This class is used as a namespace to group several global properties of
 * Panda.  Application developers can use this class to query the runtime
 * version or capabilities of the current Panda environment.
 */
class EXPCL_DTOOL PandaSystem {
protected:
  PandaSystem();
  ~PandaSystem();

PUBLISHED:
  static string get_version_string();
  static string get_package_version_string();
  static string get_package_host_url();
  static string get_p3d_coreapi_version_string();

  static int get_major_version();
  static int get_minor_version();
  static int get_sequence_version();
  static bool is_official_version();

  static int get_memory_alignment();

  static string get_distributor();
  static string get_compiler();
  static string get_build_date();
  static string get_git_commit();

  static string get_platform();

  MAKE_PROPERTY(version_string, get_version_string);
  MAKE_PROPERTY(major_version, get_major_version);
  MAKE_PROPERTY(minor_version, get_minor_version);
  MAKE_PROPERTY(sequence_version, get_sequence_version);
  MAKE_PROPERTY(official_version, is_official_version);

  MAKE_PROPERTY(memory_alignment, get_memory_alignment);

  MAKE_PROPERTY(distributor, get_distributor);
  MAKE_PROPERTY(compiler, get_compiler);
  MAKE_PROPERTY(build_date, get_build_date);
  MAKE_PROPERTY(git_commit, get_git_commit);

  MAKE_PROPERTY(platform, get_platform);

  bool has_system(const string &system) const;
  size_t get_num_systems() const;
  string get_system(size_t n) const;
  MAKE_SEQ(get_systems, get_num_systems, get_system);
  MAKE_SEQ_PROPERTY(systems, get_num_systems, get_system);

  string get_system_tag(const string &system, const string &tag) const;

  void add_system(const string &system);
  void set_system_tag(const string &system, const string &tag,
                      const string &value);

  bool heap_trim(size_t pad);

  void output(ostream &out) const;
  void write(ostream &out) const;

  static PandaSystem *get_global_ptr();

private:
  void reset_system_names();

  void set_package_version_string(const string &package_version_string);
  void set_package_host_url(const string &package_host_url);

  typedef pmap<string, string> SystemTags;
  typedef pmap<string, SystemTags> Systems;
  typedef pvector<string> SystemNames;

  Systems _systems;
  SystemNames _system_names;
  bool _system_names_dirty;

  string _package_version_string;
  string _package_host_url;

  static PandaSystem *_global_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "PandaSystem");
  }

private:
  static TypeHandle _type_handle;

  friend class ConfigPageManager;
};

inline ostream &operator << (ostream &out, const PandaSystem &ps) {
  ps.output(out);
  return out;
}

#endif
