// Filename: qpcollisionTraverser.h
// Created by:  drose (16Mar02)
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

#ifndef qpCOLLISIONTRAVERSER_H
#define qpCOLLISIONTRAVERSER_H

#include "pandabase.h"

#include "qpcollisionHandler.h"
#include "qpcollisionLevelState.h"

#include "traverserVisitor.h"
#include "nullTransitionWrapper.h"
#include "pointerTo.h"
#include "renderRelation.h"
#include "pointerTo.h"
#include "pStatCollector.h"

#include "pset.h"

class qpCollisionNode;
class Geom;
class qpNodePath;
class qpCollisionEntry;

////////////////////////////////////////////////////////////////////
//       Class : qpCollisionTraverser
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCollisionTraverser {
PUBLISHED:
  qpCollisionTraverser();
  ~qpCollisionTraverser();

  void add_collider(qpCollisionNode *node, qpCollisionHandler *handler);
  bool remove_collider(qpCollisionNode *node);
  bool has_collider(qpCollisionNode *node) const;
  int get_num_colliders() const;
  qpCollisionNode *get_collider(int n) const;
  qpCollisionHandler *get_handler(qpCollisionNode *node) const;
  void clear_colliders();

  void traverse(const qpNodePath &root);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

private:
  void prepare_colliders(qpCollisionLevelState &state);

  void r_traverse(qpCollisionLevelState &level_state);

  void compare_collider_to_node(qpCollisionEntry &entry,
                                const GeometricBoundingVolume *from_node_gbv,
                                const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_geom_node(qpCollisionEntry &entry,
                                     const GeometricBoundingVolume *from_node_gbv,
                                     const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_solid(qpCollisionEntry &entry,
                                 const GeometricBoundingVolume *from_node_gbv,
                                 const GeometricBoundingVolume *solid_gbv);
  void compare_collider_to_geom(qpCollisionEntry &entry, Geom *geom,
                                const GeometricBoundingVolume *from_node_gbv,
                                const GeometricBoundingVolume *solid_gbv);

private:
  PT(qpCollisionHandler) _default_handler;
  TypeHandle _graph_type;

  typedef pmap<PT(qpCollisionNode),  PT(qpCollisionHandler) > Colliders;
  Colliders _colliders;
  typedef pvector<qpCollisionNode *> OrderedColliders;
  OrderedColliders _ordered_colliders;

  typedef pmap<PT(qpCollisionHandler), int> Handlers;
  Handlers _handlers;

  // Statistics
  static PStatCollector _collisions_pcollector;
};

INLINE ostream &operator << (ostream &out, const qpCollisionTraverser &trav) {
  trav.output(out);
  return out;
}

#include "qpcollisionTraverser.I"

#endif

