// Filename: cullStateSubtree.h
// Created by:  drose (07Apr00)
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

#ifndef CULLSTATESUBTREE_H
#define CULLSTATESUBTREE_H

#include <pandabase.h>

#include "cullStateLookup.h"

#include <allTransitionsWrapper.h>
#include <updateSeq.h>
#include <referenceCount.h>

class CullStateLookup;

////////////////////////////////////////////////////////////////////
//       Class : CullStateSubtree
// Description : A specialization of CullStateLookup for tracking the
//               state at and below an instanced node in the scene
//               graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullStateSubtree : public CullStateLookup {
public:
  INLINE CullStateSubtree(CullStateLookup *parent,
                          const AllTransitionsWrapper &trans,
                          Node *top_subtree,
                          UpdateSeq now);
  virtual ~CullStateSubtree();

  bool check_currency(const AllTransitionsWrapper &trans,
                      Node *top_subtree,
                      UpdateSeq as_of);
  INLINE void update(const AllTransitionsWrapper &trans,
                     Node *top_subtree,
                     UpdateSeq now);

  virtual Node *get_top_subtree() const;

  virtual void compose_trans(const AllTransitionsWrapper &from,
                             AllTransitionsWrapper &to) const;

private:
  CullStateLookup *_parent;
  AllTransitionsWrapper _trans;
  AllTransitionsWrapper _trans_from_root;
  Node *_top_subtree;
  UpdateSeq _verified;
};

#include "cullStateSubtree.I"

#endif
