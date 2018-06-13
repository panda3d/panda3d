/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBinMaker.cxx
 * @author drose
 * @date 1999-01-21
 */

#include "eggBinMaker.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggBin.h"

#include "dcast.h"
#include "pnotify.h"

TypeHandle EggBinMaker::_type_handle;


/**
 * Called by the SortedNodes set to put nodes into bin order.  Returns true if
 * the first node falls into an earlier bin than the second node, false
 * otherwise.
 */
bool EggBinMakerCompareNodes::
operator ()(const EggNode *a, const EggNode *b) const {
  int bin_number_a = _ebm->get_bin_number(a);
  int bin_number_b = _ebm->get_bin_number(b);

  if (bin_number_a != bin_number_b) {
    // If the two nodes return different bin numbers, then they sort based on
    // those numbers.
    return bin_number_a < bin_number_b;
  }

  // The two nodes fell into the same bin number, so fall back on the
  // comparison function to see if they should be differentiated.
  return _ebm->sorts_less(bin_number_a, a, b);
}

/**
 *
 */
EggBinMaker::
EggBinMaker() {
}

/**
 *
 */
EggBinMaker::
~EggBinMaker() {
}


/**
 * The main entry point to EggBinMaker.  Walks the egg scene graph beginning
 * at the indicated root node, and moves all binnable nodes into EggBin
 * objects.  Returns the number of EggBins created.
 */
int EggBinMaker::
make_bins(EggGroupNode *root_group) {
  _group_nodes.clear();

  collect_nodes(root_group);

  int num_bins = 0;
  GroupNodes::const_iterator gi;
  for (gi = _group_nodes.begin(); gi != _group_nodes.end(); ++gi) {
    num_bins += get_bins_for_group(gi);
  }

  return num_bins;
}

/**
 * May be overridden in derived classes to perform some setup work as each
 * node is encountered.  This will be called once for each node in the egg
 * hierarchy.
 */
void EggBinMaker::
prepare_node(EggNode *) {
}

/**
 * May be overridden in derived classes to create additional bins within a
 * particular bin number, based on some arbitrary property of nodes.  This
 * function establishes an arbitrary but fixed ordering between nodes; if two
 * nodes do not sort to the same position, different bins are created for each
 * one (with the same bin number on each bin).
 */
bool EggBinMaker::
sorts_less(int, const EggNode *, const EggNode *) {
  return false;
}

/**
 * May be overridden in derived classes to specify whether a particular group
 * node, apparently redundant, may be safely collapsed out.
 */
bool EggBinMaker::
collapse_group(const EggGroup *, int) {
  return false;
}

/**
 * May be overridden in derived classes to define a name for each new bin,
 * based on its bin number, and a sample child.
 */
std::string EggBinMaker::
get_bin_name(int, const EggNode *) {
  return std::string();
}

/**
 * May be overridden in derived classes to construct a new EggBin object (or
 * some derived class, if needed), and preload some initial data into as
 * required.
 *
 * child is an arbitrary child of the bin, and collapse_from is the group the
 * bin is being collapsed with, if any (implying collapse_group() returned
 * true), or NULL if not.
 */
PT(EggBin) EggBinMaker::
make_bin(int, const EggNode *, EggGroup *collapse_from) {
  if (collapse_from == nullptr) {
    return new EggBin;
  } else {
    return new EggBin(*collapse_from);
  }
}

/**
 * Walks the egg scene graph, identifying nodes to be binned and moving them
 * from the scene graph into the internal bin structure.
 */
void EggBinMaker::
collect_nodes(EggGroupNode *group) {
  // We have to play games with this next iterator, because we might be
  // destructively operating on the child list as we traverse it.
  EggGroupNode::iterator i, next;

  bool first_in_group = true;
  GroupNodes::iterator gni = _group_nodes.end();

  i = group->begin();
  next = i;
  while (i != group->end()) {
    EggNode *node = (*i);
    ++next;

    prepare_node(node);

    if (get_bin_number(node) != 0) {
      // Ok, here's a node to be binned.  Add it to the appropriate bin.
      if (first_in_group) {
        // If this is the first time this group has been encountered, we need
        // to create a new entry in _group_nodes for it.

        std::pair<GroupNodes::iterator, bool> result;
        result = _group_nodes.insert
          (GroupNodes::value_type
           (group, SortedNodes(EggBinMakerCompareNodes(this))));

        nassertv(result.second);
        gni = result.first;
        first_in_group = false;
      }

      // Add this node to the set of all nodes being binned for the group.
      // This also puts the nodes into bin order.
      nassertv(gni != _group_nodes.end());
      (*gni).second.insert(node);

      // And remove it from the scene graph.
      group->erase(i);
    }

    if (node->is_of_type(EggGroupNode::get_class_type())) {
      // Recursively traverse.
      collect_nodes(DCAST(EggGroupNode, node));
    }

    i = next;
  }
}


/**
 * Breaks the set of nodes for a given group up into individual bins.
 */
int EggBinMaker::
get_bins_for_group(GroupNodes::const_iterator gi) {
  EggGroupNode *group = (*gi).first;
  const SortedNodes &nodes = (*gi).second;

  // It shouldn't be possible for this to be empty.
  nassertr(!nodes.empty(), 0);

  Bins bins;
  EggBinMakerCompareNodes cn(this);
  SortedNodes::const_iterator sni, last;
  sni = nodes.begin();
  last = sni;

  bins.push_back(Nodes());
  bins.back().push_back(*sni);
  ++sni;
  while (sni != nodes.end()) {
    if (cn(*last, *sni)) {
      // Begin a new bin.
      bins.push_back(Nodes());
    }
    bins.back().push_back(*sni);

    last = sni;
    ++sni;
  }

  make_bins_for_group(group, bins);
  return bins.size();
}

/**
 * Creates the EggBin nodes indicated by the internal bin structure for each
 * group.
 */
void EggBinMaker::
make_bins_for_group(EggGroupNode *group, const Bins &bins) {
  // We shouldn't be able to get here if we have no bins!
  nassertv(!bins.empty());

  // If the group will have only one bin, and no other children, and the group
  // is not the root node (and it is not some funny group-like node like a
  // <Table>), maybe we should collapse the group and its bin together.

  bool collapse = false;

  if (group->empty() &&
      bins.size() == 1 &&
      group->get_parent() != nullptr &&
      group->is_of_type(EggGroup::get_class_type())) {
    const Nodes &nodes = bins.front();
    nassertv(!nodes.empty());
    int bin_number = get_bin_number(nodes.front());
    collapse = collapse_group(DCAST(EggGroup, group), bin_number);
  }

  if (collapse) {
    const Nodes &nodes = bins.front();
    nassertv(!nodes.empty());
    int bin_number = get_bin_number(nodes.front());
    PT(EggBin) bin = make_bin(bin_number, nodes.front(), DCAST(EggGroup, group));
    setup_bin(bin, nodes);

    EggGroupNode *parent = group->get_parent();
    parent->remove_child(group);
    parent->add_child(bin);

  } else {
    Bins::const_iterator bi;
    for (bi = bins.begin(); bi != bins.end(); ++bi) {
      const Nodes &nodes = (*bi);
      nassertv(!nodes.empty());
      int bin_number = get_bin_number(nodes.front());
      PT(EggBin) bin = make_bin(bin_number, nodes.front(), nullptr);
      setup_bin(bin, nodes);

      group->add_child(bin);
    }
  }
}


/**
 * Sets up a recently-created EggBin structure with all of its children.
 */
void EggBinMaker::
setup_bin(EggBin *bin, const Nodes &nodes) {
  nassertv(!nodes.empty());
  int bin_number = get_bin_number(nodes.front());
  bin->set_bin_number(bin_number);

  std::string bin_name = get_bin_name(bin_number, nodes.front());
  if (!bin_name.empty()) {
    bin->set_name(bin_name);
  }

  Nodes::const_iterator ni;
  for (ni = nodes.begin(); ni != nodes.end(); ++ni) {
    bin->add_child(*ni);
  }
}
