// Filename: auto_bind.cxx
// Created by:  drose (23Feb99)
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


#include "auto_bind.h"
#include "qpanimBundleNode.h"
#include "qppartBundleNode.h"
#include "config_chan.h"
#include "string_utils.h"

typedef pset<qpAnimBundleNode *> qpAnimNodes;
typedef pmap<string, qpAnimNodes> qpAnims;

typedef pset<qpPartBundleNode *> qpPartNodes;
typedef pmap<string, qpPartNodes> qpParts;


////////////////////////////////////////////////////////////////////
//     Function: bind_anims
//  Description: A support function for auto_bind(), below.  Given a
//               set of AnimBundles and a set of PartBundles that all
//               share the same name, perform whatever bindings make
//               sense.
////////////////////////////////////////////////////////////////////
static void
qpbind_anims(const qpPartNodes &parts, const qpAnimNodes &anims,
           AnimControlCollection &controls,
           int hierarchy_match_flags) {

  qpPartNodes::const_iterator pni;

  for (pni = parts.begin(); pni != parts.end(); ++pni) {
    PartBundle *part = (*pni)->get_bundle();

    qpAnimNodes::const_iterator ani;
    for (ani = anims.begin(); ani != anims.end(); ++ani) {
      AnimBundle *anim = (*ani)->get_bundle();

      if (chan_cat.is_info()) {
        chan_cat.info()
          << "Attempting to bind " << *part << " to " << *anim << "\n";
      }

      PT(AnimControl) control =
        part->bind_anim(anim, hierarchy_match_flags);
      string name = anim->get_name();
      if (control != (AnimControl *)NULL) {
        if (controls.find_anim(name) != (AnimControl *)NULL) {
          // That name's already used; synthesize another one.
          int index = 0;
          string new_name;
          do {
            index++;
            new_name = name + '.' + format_string(index);
          } while (controls.find_anim(new_name) != (AnimControl *)NULL);
          name = new_name;
        }

        controls.store_anim(control, name);
      }

      if (chan_cat.is_info()) {
        if (control == NULL) {
          chan_cat.info()
            << "Bind failed.\n";
        } else {
          chan_cat.info()
            << "Bind succeeded, index "
            << control->get_channel_index() << "; accessible as "
            << name << "\n";
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: r_find_bundles
//  Description: A support function for auto_bind(), below.  Walks
//               through the hierarchy and finds all of the
//               PartBundles and AnimBundles.
////////////////////////////////////////////////////////////////////
static void 
r_find_bundles(PandaNode *node, qpAnims &anims, qpParts &parts) {
  if (node->is_of_type(qpAnimBundleNode::get_class_type())) {
    qpAnimBundleNode *bn = DCAST(qpAnimBundleNode, node);
    anims[bn->get_bundle()->get_name()].insert(bn);
    
  } else if (node->is_of_type(qpPartBundleNode::get_class_type())) {
    qpPartBundleNode *bn = DCAST(qpPartBundleNode, node);
    parts[bn->get_bundle()->get_name()].insert(bn);
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_find_bundles(cr.get_child(i), anims, parts);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: auto_bind
//  Description: Walks the scene graph or subgraph beginning at the
//               indicated node, and attempts to bind any AnimBundles
//               found to their matching PartBundles, when possible.
//
//               The list of all resulting AnimControls created is
//               filled into controls.
////////////////////////////////////////////////////////////////////
void
auto_bind(PandaNode *root_node, AnimControlCollection &controls,
          int hierarchy_match_flags) {
  // First, locate all the bundles in the subgraph.
  qpAnims anims;
  qpParts parts;
  r_find_bundles(root_node, anims, parts);

  // Now, match up the bundles by name.

  qpAnims::const_iterator ai = anims.begin();
  qpParts::const_iterator pi = parts.begin();

  while (ai != anims.end() && pi != parts.end()) {
    if ((*ai).first < (*pi).first) {
      // Here's an anim with no matching parts.
      ++ai;

    } else if ((*pi).first < (*ai).first) {
      // And here's a part with no matching anims.
      ++pi;

    } else {
      // But here we have (at least one) match!
      qpbind_anims((*pi).second, (*ai).second, controls,
                   hierarchy_match_flags);
      ++pi;

      // We don't increment the anim counter yet.  That way, the same
      // anim may bind to multiple parts, if they all share the same
      // name.
    }
  }
}


