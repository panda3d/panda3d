/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxPath.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef FINDAPPROXPATH_H
#define FINDAPPROXPATH_H

#include "pandabase.h"

#include "globPattern.h"
#include "typeHandle.h"
#include "pvector.h"
#include "pnotify.h"

class PandaNode;

/**
 * This class is local to this package only; it doesn't get exported.  It
 * chops a string path, as supplied to find_up() or find_down(), and breaks it
 * up into its component pieces.
 */
class FindApproxPath {
public:
  INLINE FindApproxPath();

  bool add_string(const std::string &str_path);
  bool add_flags(const std::string &str_flags);
  bool add_component(std::string str_component);

  void add_match_name(const std::string &name, int flags);
  void add_match_name_glob(const std::string &glob, int flags);
  void add_match_exact_type(TypeHandle type, int flags);
  void add_match_inexact_type(TypeHandle type, int flags);
  void add_match_tag(const std::string &key, int flags);
  void add_match_tag_value(const std::string &key, const std::string &value, int flags);

  void add_match_one(int flags);
  void add_match_many(int flags);
  void add_match_pointer(PandaNode *pointer, int flags);

  INLINE int get_num_components() const;
  INLINE bool is_component_match_many(int index) const;
  INLINE bool matches_component(int index, PandaNode *node) const;
  INLINE bool matches_stashed(int index) const;

  INLINE bool return_hidden() const;
  INLINE bool return_stashed() const;
  INLINE bool case_insensitive() const;

  void output(std::ostream &out) const;
  INLINE void output_component(std::ostream &out, int index) const;

#if !defined(WIN32_VC) && !defined(WIN64_VC)
// Visual C++ won't let us define the ostream operator functions for these
// guys if they're private--even though we declare them friends.
private:
#endif
  enum ComponentType {
    CT_match_name,
    CT_match_name_insensitive,
    CT_match_name_glob,
    CT_match_exact_type,
    CT_match_inexact_type,
    CT_match_tag,
    CT_match_tag_value,
    CT_match_one,
    CT_match_many,
    CT_match_pointer
  };
  enum ComponentFlags {
    CF_stashed        = 0x001,
  };

  class Component {
  public:
    bool matches(PandaNode *node) const;
    void output(std::ostream &out) const;

    ComponentType _type;
    std::string _name;
    GlobPattern _glob;
    TypeHandle _type_handle;
    PandaNode *_pointer;
    int _flags;
  };

  typedef pvector<Component> Path;
  Path _path;

  bool _return_hidden;
  bool _return_stashed;
  bool _case_insensitive;

friend std::ostream &operator << (std::ostream &, FindApproxPath::ComponentType);
friend INLINE std::ostream &operator << (std::ostream &, const FindApproxPath::Component &);
};

std::ostream &
operator << (std::ostream &out, FindApproxPath::ComponentType type);

INLINE std::ostream &
operator << (std::ostream &out, const FindApproxPath::Component &component) {
  component.output(out);
  return out;
}

INLINE std::ostream &
operator << (std::ostream &out, const FindApproxPath &path) {
  path.output(out);
  return out;
}

#include "findApproxPath.I"

#endif
