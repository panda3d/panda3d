/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file attribNodeRegistry.h
 * @author drose
 * @date 2007-07-07
 */

#ifndef ATTRIBNODEREGISTRY_H
#define ATTRIBNODEREGISTRY_H

#include "pandabase.h"
#include "nodePath.h"
#include "ordered_vector.h"
#include "lightMutex.h"

/**
 * This global object records NodePaths that are referenced by scene graph
 * attribs, such as ClipPlaneAttribs and LightAttribs.
 *
 * Its primary purpose is to unify attribs that are loaded in from bam files.
 * Attrib nodes are identified by name and type; when a bam file that contains
 * references to some attrib nodes is loaded, those nodes are first looked up
 * here in the AttribNodeRegistry.  If there is a match (by name and node
 * type), the identified node is used instead of the node referenced within
 * the bam file itself.
 */
class EXPCL_PANDA_PGRAPH AttribNodeRegistry {
protected:
  AttribNodeRegistry();

PUBLISHED:
  void add_node(const NodePath &attrib_node);
  bool remove_node(const NodePath &attrib_node);
  NodePath lookup_node(const NodePath &orig_node) const;

  int get_num_nodes() const;
  NodePath get_node(int n) const;
  MAKE_SEQ(get_nodes, get_num_nodes, get_node);
  TypeHandle get_node_type(int n) const;
  std::string get_node_name(int n) const;

  int find_node(const NodePath &attrib_node) const;
  int find_node(TypeHandle type, const std::string &name) const;
  void remove_node(int n);
  void clear();

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

  INLINE static AttribNodeRegistry *get_global_ptr();

private:
  static void make_global_ptr();

  class Entry {
  public:
    INLINE Entry(const NodePath &node);
    INLINE Entry(TypeHandle type, const std::string &name);
    INLINE bool operator < (const Entry &other) const;

    TypeHandle _type;
    std::string _name;
    NodePath _node;
  };

  typedef ov_set<Entry> Entries;
  Entries _entries;

  LightMutex _lock;

  static AttribNodeRegistry * TVOLATILE _global_ptr;
};

#include "attribNodeRegistry.I"

#endif
