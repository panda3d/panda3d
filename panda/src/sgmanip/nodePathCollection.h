// Filename: nodePathCollection.h
// Created by:  drose (06Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODEPATHCOLLECTION_H
#define NODEPATHCOLLECTION_H

#include <pandabase.h>

// We don't include NodePath in the header file, so NodePath can
// include us.
#include "nodePathBase.h"

#include <pointerToArray.h>

class NodePath;

////////////////////////////////////////////////////////////////////
//       Class : NodePathCollection
// Description : This is a set of zero or more NodePaths.  It's handy
//               for returning from functions that need to return
//               multiple NodePaths (for instance,
//               NodePaths::get_children).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePathCollection {
PUBLISHED:
  NodePathCollection();
  NodePathCollection(const NodePathCollection &copy);
  void operator = (const NodePathCollection &copy);
  INLINE ~NodePathCollection();

  void add_path(const NodePath &node_path);
  bool remove_path(const NodePath &node_path);
  void add_paths_from(const NodePathCollection &other);
  void remove_paths_from(const NodePathCollection &other);
  void remove_duplicate_paths();
  bool has_path(const NodePath &path) const;
  void clear();

  bool is_empty() const;
  int get_num_paths() const;
  NodePath get_path(int index) const;

  // Handy operations on many NodePaths at once.
  INLINE void ls() const;
  void ls(ostream &out, int indent_level = 0) const;

  NodePathCollection find_all_matches(const string &path) const;
  void reparent_to(const NodePath &other);
  void wrt_reparent_to(const NodePath &other);
  NodePathCollection instance_to(const NodePath &other) const;

  void show();
  void hide();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(NodePathBase) NodePaths;
  NodePaths _node_paths;
};

INLINE ostream &operator << (ostream &out, const NodePathCollection &col) {
  col.output(out);
  return out;
}

#include "nodePathCollection.I"

#endif


