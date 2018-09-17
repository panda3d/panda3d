/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file auto_bind.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "auto_bind.h"
#include "animBundleNode.h"
#include "partBundleNode.h"
#include "config_chan.h"
#include "string_utils.h"
#include "partGroup.h"

using std::string;

typedef pset<AnimBundle *> AnimBundles;
typedef pmap<string, AnimBundles> Anims;

typedef pset<PartBundle *> PartBundles;
typedef pmap<string, PartBundles> Parts;


/**
 * A support function for auto_bind(), below.  Given a set of AnimBundles and
 * a set of PartBundles that all share the same name, perform whatever
 * bindings make sense.
 */
static void
bind_anims(const PartBundles &parts, const AnimBundles &anims,
           AnimControlCollection &controls,
           int hierarchy_match_flags) {
  PartBundles::const_iterator pbi;

  for (pbi = parts.begin(); pbi != parts.end(); ++pbi) {
    PartBundle *part = (*pbi);
    AnimBundles::const_iterator abi;
    for (abi = anims.begin(); abi != anims.end(); ++abi) {
      AnimBundle *anim = (*abi);
      if (chan_cat.is_info()) {
        chan_cat.info()
          << "Attempting to bind " << *part << " to " << *anim << "\n";
      }

      PT(AnimControl) control =
        part->bind_anim(anim, hierarchy_match_flags);
      string name = (*abi)->get_name();
      if (name.empty()) {
        name = anim->get_name();
      }
      if (control != nullptr) {
        if (controls.find_anim(name) != nullptr) {
          // That name's already used; synthesize another one.
          int index = 0;
          string new_name;
          do {
            index++;
            new_name = name + '.' + format_string(index);
          } while (controls.find_anim(new_name) != nullptr);
          name = new_name;
        }

        controls.store_anim(control, name);
      }

      if (chan_cat.is_info()) {
        if (control == nullptr) {
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

/**
 * A support function for auto_bind(), below.  Walks through the hierarchy and
 * finds all of the PartBundles and AnimBundles.
 */
static void
r_find_bundles(PandaNode *node, Anims &anims, Parts &parts) {
  if (node->is_of_type(AnimBundleNode::get_class_type())) {
    AnimBundleNode *bn = DCAST(AnimBundleNode, node);
    AnimBundle *bundle = bn->get_bundle();
    anims[bundle->get_name()].insert(bundle);

  } else if (node->is_of_type(PartBundleNode::get_class_type())) {
    PartBundleNode *bn = DCAST(PartBundleNode, node);
    int num_bundles = bn->get_num_bundles();
    for (int i = 0; i < num_bundles; ++i) {
      PartBundle *bundle = bn->get_bundle(i);
      parts[bundle->get_name()].insert(bundle);
    }
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_find_bundles(cr.get_child(i), anims, parts);
  }
}


/**
 * Walks the scene graph or subgraph beginning at the indicated node, and
 * attempts to bind any AnimBundles found to their matching PartBundles, when
 * possible.
 *
 * The list of all resulting AnimControls created is filled into controls.
 */
void
auto_bind(PandaNode *root_node, AnimControlCollection &controls,
          int hierarchy_match_flags) {
  // First, locate all the bundles in the subgraph.
  Anims anims;
  AnimBundles extra_anims;
  Parts parts;
  PartBundles extra_parts;
  r_find_bundles(root_node, anims, parts);

  if (chan_cat.is_debug()) {
    int anim_count = 0;
    Anims::const_iterator ai;
    for (ai = anims.begin(); ai != anims.end(); ++ai) {
      anim_count += (int)(*ai).second.size();
    }
    chan_cat.debug()
      << "Found " << anim_count << " anims:\n";
    for (ai = anims.begin(); ai != anims.end(); ++ai) {
      chan_cat.debug(false)
        << " " << (*ai).first;
      if ((*ai).second.size() != 1) {
        chan_cat.debug(false)
          << "*" << ((*ai).second.size());
      }
    }
    chan_cat.debug(false)
      << "\n";

    int part_count = 0;
    Parts::const_iterator pi;
    for (pi = parts.begin(); pi != parts.end(); ++pi) {
      part_count += (int)(*pi).second.size();
    }
    chan_cat.debug()
      << "Found " << part_count << " parts:\n";
    for (pi = parts.begin(); pi != parts.end(); ++pi) {
      chan_cat.debug(false)
        << " " << (*pi).first;
      if ((*pi).second.size() != 1) {
        chan_cat.debug(false)
          << "*" << ((*pi).second.size());
      }
    }
    chan_cat.debug(false)
      << "\n";
  }

  // Now, match up the bundles by name.

  Anims::const_iterator ai = anims.begin();
  Parts::const_iterator pi = parts.begin();

  while (ai != anims.end() && pi != parts.end()) {
    if ((*ai).first < (*pi).first) {
      // Here's an anim with no matching parts.
      if (hierarchy_match_flags & PartGroup::HMF_ok_wrong_root_name) {
        AnimBundles::const_iterator abi;
        for (abi = (*ai).second.begin(); abi != (*ai).second.end(); ++abi) {
          extra_anims.insert(*abi);
        }
      }
      ++ai;

    } else if ((*pi).first < (*ai).first) {
      // And here's a part with no matching anims.
      if (hierarchy_match_flags & PartGroup::HMF_ok_wrong_root_name) {
        PartBundles::const_iterator pbi;
        for (pbi = (*pi).second.begin(); pbi != (*pi).second.end(); ++pbi) {
          extra_parts.insert(*pbi);
        }
      }
      ++pi;

    } else {
      // But here we have (at least one) match!
      bind_anims((*pi).second, (*ai).second, controls,
                   hierarchy_match_flags);
      ++pi;

      // We don't increment the anim counter yet.  That way, the same anim may
      // bind to multiple parts, if they all share the same name.
    }
  }

  if (hierarchy_match_flags & PartGroup::HMF_ok_wrong_root_name) {
    // Continue searching through the remaining anims and parts.

    while (ai != anims.end()) {
      // Here's an anim with no matching parts.
      if (hierarchy_match_flags & PartGroup::HMF_ok_wrong_root_name) {
        AnimBundles::const_iterator abi;
        for (abi = (*ai).second.begin(); abi != (*ai).second.end(); ++abi) {
          extra_anims.insert(*abi);
        }
      }
      ++ai;
    }

    while (pi != parts.end()) {
      // And here's a part with no matching anims.
      if (hierarchy_match_flags & PartGroup::HMF_ok_wrong_root_name) {
        PartBundles::const_iterator pbi;
        for (pbi = (*pi).second.begin(); pbi != (*pi).second.end(); ++pbi) {
          extra_parts.insert(*pbi);
        }
      }
      ++pi;
    }

    bind_anims(extra_parts, extra_anims, controls,
               hierarchy_match_flags);
  }
}
