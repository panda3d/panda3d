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

#ifdef WIN32_VC
#include "chan_headers.h"
#endif

#pragma hdrstop

#ifndef WIN32_VC
#include "animBundleNode.h"
#include "partBundleNode.h"
#include "config_chan.h"
#endif

#include <renderRelation.h>
#include <traverserVisitor.h>
#include <dftraverser.h>
#include <string_utils.h>
#include <nullAttributeWrapper.h>
#include <nullLevelState.h>
#include <nullTransitionWrapper.h>
#include "auto_bind.h"

typedef set<AnimBundleNode *> AnimNodes;
typedef map<string, AnimNodes> Anims;

typedef set<PartBundleNode *> PartNodes;
typedef map<string, PartNodes> Parts;



////////////////////////////////////////////////////////////////////
//       Class : CollectNodes
// Description : This is a traverser visitor that locates bundle nodes
//               and adds them to their respective maps.
////////////////////////////////////////////////////////////////////
class CollectNodes :
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  bool reached_node(Node *node, const NullAttributeWrapper &,
                    NullLevelState &) {
    if (node->is_of_type(AnimBundleNode::get_class_type())) {
      AnimBundleNode *bn = DCAST(AnimBundleNode, node);
      _anims[bn->get_bundle()->get_name()].insert(bn);

    } else if (node->is_of_type(PartBundleNode::get_class_type())) {
      PartBundleNode *bn = DCAST(PartBundleNode, node);
      _parts[bn->get_bundle()->get_name()].insert(bn);
    }

    return true;
  }

  Anims _anims;
  Parts _parts;
};


////////////////////////////////////////////////////////////////////
//     Function: bind_anims
//  Description: A support function for auto_bind(), below.  Given a
//               set of AnimBundles and a set of PartBundles that all
//               share the same name, perform whatever bindings make
//               sense.
////////////////////////////////////////////////////////////////////
static void
bind_anims(const PartNodes &parts, const AnimNodes &anims,
           AnimControlCollection &controls,
           int hierarchy_match_flags) {

  PartNodes::const_iterator pni;

  for (pni = parts.begin(); pni != parts.end(); ++pni) {
    PartBundle *part = (*pni)->get_bundle();

    AnimNodes::const_iterator ani;
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
//     Function: auto_bind
//  Description: Walks the scene graph or subgraph beginning at the
//               indicated node, and attempts to bind any AnimBundles
//               found to their matching PartBundles, when possible.
//
//               The list of all resulting AnimControls created is
//               filled into controls.
////////////////////////////////////////////////////////////////////
void auto_bind(Node *root_node, AnimControlCollection &controls,
               int hierarchy_match_flags) {

  // First, locate all the bundles in the subgraph.
  CollectNodes cn;
  df_traverse(root_node, cn, NullAttributeWrapper(), NullLevelState(),
              RenderRelation::get_class_type());

  // Now, match up the bundles by name.

  Anims::const_iterator ai = cn._anims.begin();
  Parts::const_iterator pi = cn._parts.begin();

  while (ai != cn._anims.end() && pi != cn._parts.end()) {
    if ((*ai).first < (*pi).first) {
      // Here's an anim with no matching parts.
      ++ai;

    } else if ((*pi).first < (*ai).first) {
      // And here's a part with no matching anims.
      ++pi;

    } else {
      // But here we have (at least one) match!
      bind_anims((*pi).second, (*ai).second, controls,
                 hierarchy_match_flags);
      ++pi;

      // We don't increment the anim counter yet.  That way, the same
      // anim may bind to multiple parts, if they all share the same
      // name.
    }
  }
}


