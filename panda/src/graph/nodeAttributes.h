// Filename: nodeAttributes.h
// Created by:  drose (20Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef NODEATTRIBUTES_H
#define NODEATTRIBUTES_H

#include <pandabase.h>

#include "nodeAttribute.h"

#include <pointerTo.h>

#include <map>

class NodeTransitionCache;

////////////////////////////////////////////////////////////////////
//       Class : NodeAttributes
// Description : This represents a set of zero or more NodeAttribute
//               pointers, organized by the attributes' get_handle()
//               value.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeAttributes {
PUBLISHED:
  NodeAttributes();

public:
  NodeAttributes(const NodeAttributes &copy);
  void operator = (const NodeAttributes &copy);
  ~NodeAttributes();

PUBLISHED:
  bool is_empty() const;
  PT(NodeAttribute) set_attribute(TypeHandle handle, NodeAttribute *attrib);
  PT(NodeAttribute) clear_attribute(TypeHandle handle);
  bool has_attribute(TypeHandle handle) const;
  NodeAttribute *get_attribute(TypeHandle handle) const;

  void clear();

  bool is_initial() const;
  int compare_to(const NodeAttributes &other) const;

private:
  typedef map< TypeHandle, PT(NodeAttribute) > Attributes;
public:
  // STL-like definitions to expose the map within NodeAttributes to
  // external adjustment.  Beware!  These are not safe to use outside
  // of PANDA.DLL.
  typedef Attributes::iterator iterator;
  typedef Attributes::const_iterator const_iterator;
  typedef Attributes::key_type key_type;
  typedef Attributes::value_type value_type;
  typedef Attributes::size_type size_type;

  INLINE_GRAPH size_type size() const;
  INLINE_GRAPH iterator begin();
  INLINE_GRAPH iterator end();
  INLINE_GRAPH const_iterator begin() const;
  INLINE_GRAPH const_iterator end() const;
  INLINE_GRAPH iterator insert(iterator position, const value_type &x);
  INLINE_GRAPH iterator find(const key_type &k);
  INLINE_GRAPH void erase(iterator position);

public:
  INLINE_GRAPH void apply_in_place(const NodeTransitionCache &trans);
  INLINE_GRAPH NodeAttributes *apply(const NodeTransitionCache &trans) const;
  void apply_from(const NodeAttributes &other,
                  const NodeTransitionCache &trans);

  void merge_from(const NodeAttributes &a, const NodeAttributes &b);

PUBLISHED:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  Attributes _attributes;
  friend class NodeAttributeCache;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttributes &nas);

#include "nodeAttributes.T"

#ifndef DONT_INLINE_GRAPH
#include "nodeAttributes.I"
#endif

#endif

