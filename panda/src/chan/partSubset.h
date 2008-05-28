// Filename: partSubset.h
// Created by:  drose (19Jan06)
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

#ifndef PARTSUBSET_H
#define PARTSUBSET_H

#include "pandabase.h"
#include "globPattern.h"

////////////////////////////////////////////////////////////////////
//       Class : PartSubset
// Description : This class is used to define a subset of part names
//               to apply to the PartBundle::bind_anim() operation.
//               Only those part names within the subset will be
//               included in the bind.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAN PartSubset {
PUBLISHED:
  PartSubset();
  PartSubset(const PartSubset &copy);
  void operator = (const PartSubset &copy);

  void add_include_joint(const GlobPattern &name);
  void add_exclude_joint(const GlobPattern &name);

  void append(const PartSubset &other);

  void output(ostream &out) const;

  bool is_include_empty() const;
  bool matches_include(const string &joint_name) const;
  bool matches_exclude(const string &joint_name) const;

private:
  typedef pvector<GlobPattern> Joints;
  Joints _include_joints;
  Joints _exclude_joints;
};

INLINE ostream &operator << (ostream &out, const PartSubset &subset) {
  subset.output(out);
  return out;
}

#include "partSubset.I"

#endif

