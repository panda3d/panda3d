// Filename: cullStateLookup.cxx
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


#include "cullState.h"
#include "cullStateSubtree.h"

#include <indent.h>

#include "cullStateLookup.h"

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CullStateLookup::
~CullStateLookup() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::clear
//       Access: Public
//  Description: Removes all of the data cached in the lookup.
////////////////////////////////////////////////////////////////////
void CullStateLookup::
clear() {
  _cull_states.clear();

  // We have to explicitly delete each of the subtrees created within
  // this class.
  Subtrees::iterator si;
  for (si = _subtrees.begin(); si != _subtrees.end(); ++si) {
    delete (*si).second;
  }
  _subtrees.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::find_node
//       Access: Public
//  Description: Looks up the indicated Node in the table, and
//               determines if the associated CullState (if any) is
//               fresh.  If it is acceptable, returns its pointer; if
//               there is no associated CullState or if the CullState
//               is stale, removes the CullState and returns NULL.
////////////////////////////////////////////////////////////////////
CullState *CullStateLookup::
find_node(Node *node,
          const AllTransitionsWrapper &trans,
          UpdateSeq now) {
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Looking up " << *node << " in lookup " << (void *)this << "\n";
  }
  CullStates::iterator csi;
  csi = _cull_states.find(node);
  if (csi == _cull_states.end()) {
    // No entry for the node.
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "No entry for the node.\n";
    }
    return NULL;
  }

  CullState *cs = (*csi).second;
  if (cs->check_currency(node, trans, now)) {
    // The entry is current enough to use.
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "The entry is found and current.\n";
    }
    return cs;
  }

  // The entry is stale; remove it and return NULL.
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "The entry is stale.\n";
  }
  _cull_states.erase(csi);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::get_subtree
//       Access: Public
//  Description: A subtree is declared for each arc that begins a new
//               shared instance of a node within the scene graph;
//               i.e. each arc that leads to a node with multiple
//               parents.  Each of the descendents of this arc is
//               added only to the thus-defined subtree, and not
//               directly to the parent CullStateLookup; this allows us
//               to define a different cached value for each instance
//               of a particular node.
//
//               This function looks up the indicated NodeRelation in
//               the table, and determines if the associated
//               CullStateSubtree (if any) is fresh.  If it is
//               acceptable, returns its pointer; if there is no
//               associated CullStateSubtree or if the
//               CullStateSubtree is stale, creates and returns a new
//               CullStateSubtree.  In any case, a usable
//               CullStateSubtree is always returned.
////////////////////////////////////////////////////////////////////
CullStateSubtree *CullStateLookup::
get_subtree(const PT_NodeRelation &arc,
            const AllTransitionsWrapper &trans,
            Node *top_subtree,
            UpdateSeq now) {
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Getting subtree for " << *arc << " in lookup "
      << (void *)this << "\n";
  }
  Subtrees::iterator si;
  si = _subtrees.find(arc);
  if (si == _subtrees.end()) {
    // No entry for the arc.
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "No entry for the arc; creating new one.\n";
    }
    CullStateSubtree *subtree =
      new CullStateSubtree(this, trans, top_subtree, now);
    _subtrees.insert(Subtrees::value_type(arc, subtree));
    return subtree;
  }

  CullStateSubtree *subtree = (*si).second;
  if (subtree->check_currency(trans, top_subtree, now)) {
    // The entry is current enough to use.
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "The entry is found and current.\n";
    }
    return subtree;
  }

  // The entry is stale; update it.
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "The entry is stale.\n";
  }
  subtree->update(trans, top_subtree, now);
  return subtree;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::clean_out_top_nodes
//       Access: Public
//  Description: Walks through the list of nodes and arcs, and removes
//               any pointers to CullStates that don't seem to be
//               active any more.
////////////////////////////////////////////////////////////////////
void CullStateLookup::
clean_out_old_nodes() {
  CullStates::iterator csi, csnext;
  csi = _cull_states.begin();
  csnext = csi;
  while (csi != _cull_states.end()) {
    ++csnext;

    CullState *cs = (*csi).second;
    if (!cs->has_bin() && cs->is_empty() &&
        cs->get_empty_frames_count() > 100) {
      _cull_states.erase(csi);
    }
    csi = csnext;
  }

  Subtrees::iterator si, snext;
  si = _subtrees.begin();
  snext = si;
  while (si != _subtrees.end()) {
    ++snext;

    CullStateSubtree *subtree = (*si).second;
    subtree->clean_out_old_nodes();
    if (subtree->is_completely_empty()) {
      _subtrees.erase(si);
    }
    si = snext;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::get_top_subtree
//       Access: Public, Virtual
//  Description: Returns the node that represents the top of the
//               subtree for this particular CullStateSubtree.  In the
//               case of the root CullStateLookup, this always returns
//               NULL, indicating the root of the graph.
////////////////////////////////////////////////////////////////////
Node *CullStateLookup::
get_top_subtree() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::compose_trans
//       Access: Public, Virtual
//  Description: Composes the transitions given in "from" with the net
//               transitions that appear above the top node of this
//               subtree, and returns the result in "to".  This is a
//               pass-thru for CullStateLookup; it has effect only for
//               CullStateSubtree.
////////////////////////////////////////////////////////////////////
void CullStateLookup::
compose_trans(const AllTransitionsWrapper &from,
              AllTransitionsWrapper &to) const {
  to = from;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CullStateLookup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << "CullStateLookup(";
  bool is_first = true;

  CullStates::const_iterator csi;
  for (csi = _cull_states.begin(); csi != _cull_states.end(); ++csi) {
    if (!is_first) {
      out << ", ";
    }
    is_first = false;
    out << *(*csi).first;
  }
  out << ") {\n";

  Subtrees::const_iterator si;
  for (si = _subtrees.begin(); si != _subtrees.end(); ++si) {
    const NodeRelation *arc = (*si).first;
    const CullStateSubtree *subtree = (*si).second;

    indent(out, indent_level + 2) << *arc << " {\n";
    subtree->write(out, indent_level + 4);
    indent(out, indent_level + 2) << "}\n";
  }

  indent(out, indent_level) << "}\n";
}
