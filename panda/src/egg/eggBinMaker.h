/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBinMaker.h
 * @author drose
 * @date 1999-01-21
 */

#ifndef EGGBINMAKER_H
#define EGGBINMAKER_H

/*
 * EggBinMaker This is a handy class for collecting related nodes together.
 * Its purpose is to make it easier to process egg files for converting to
 * another scene graph format.  Egg is very general and allows nodes to be
 * parented willy-nilly anywhere you like, while many other scene graph
 * formats have requirements that certain kinds of nodes be grouped together.
 * Although EggBinMaker can be used to group any kinds of nodes together, one
 * of the most common examples is grouping polygons into polysets.  Egg allows
 * individual polygons to be parented directly to any group node, while most
 * scene graph formats prefer to have polygons with similar attributes grouped
 * into some kind of a polyset node.  Therefore, the following usage
 * discussion will use grouping polygons into polysets as an example.
 * EggBinMaker is actually an abstract class; it cannot be used directly.  To
 * use it, you must create a subclass and redefine some or all of its virtual
 * functions to specify the precise behavior you require.  You must define at
 * least the following function: virtual int get_bin_number(const EggNode
 * *node); This function identifies the kinds of nodes in the graph, for
 * instance EggPolygons, that are to be put into bins.  It will be called once
 * for each node encountered, and it should return nonzero if the node is to
 * be binned, and zero otherwise.  To group polygons into polysets, this
 * function might look like: virtual int get_bin_number(const EggNode *node) {
 * if (node->is_of_type(EggPolygon::get_class_type())) { return 1; } else {
 * return 0; } } This function may also return the bin number that a given
 * node should be dropped into.  The bin number is completely arbitrary, and
 * it just serves to differentiate different bins.  By default, all sibling
 * nodes will be dropped into the same bin; you can redefine this to sort
 * nodes further into categories.  For instance, if you wanted to put textured
 * polygons into a different polyset than untextured polygons, you might
 * define this function as follows: virtual int get_bin_number(const EggNode
 * *node) { if (node->is_of_type(EggPolygon::get_class_type())) { EggPolygon
 * *poly = DCAST(EggPolygon, node); return (poly->has_texture()) ? 1 : 2; }
 * else { return 0; } } Of course, unrelated nodes--nodes that belong to
 * different parents--will never be placed into the same bin together,
 * regardless of the bin number.  It is important to note that it is not
 * necessarily true that there is only one bin for each bin number.  If you
 * redefine sorts_less(), below, you provide a finer-grained control that may
 * create multiple bins for a given bin number.  This function may be called
 * several times for a given node, and it should return the same number each
 * time.  You may also redefine any or all of the following functions: virtual
 * void prepare_node(EggNode *node); This method is called, once, on each node
 * in the egg hierarchy as it is visited the first time.  It allows the
 * subclass a chance to analyze the node or do any other initial processing.
 * This is a fine opportunity to tag an EggUserData onto the node, for
 * instance.  virtual bool sorts_less(int bin_number, const EggNode *a, const
 * EggNode *b); Sometimes a simple bin number alone is not enough.  For
 * instance, suppose you needed to group together not just all textured
 * polygons, but all polygons that shared a particular texture map.  Two
 * polygons that are each textured with a different texture map should go into
 * different polysets.  To do this with bin numbers, you'd have to know ahead
 * of time all the texture maps that are in use, and assign a unique number to
 * each one.  sorts_less() can make this unnecessary.  It's a finer-grained
 * sorting than by bin numbers.  Once two nodes have been grouped together
 * into the same bin number, sorts_less is called on them.  If it returns
 * true, then node a should be placed into an earlier bin than node b, even
 * though they share the same bin number.  If sorts_less(a, b) and
 * sorts_less(b, a) both return false, then nodes a and b are placed into the
 * same bin.  To continue the example, and sort polygons into different bins
 * based on the texture map: virtual bool sorts_less(int bin_number, const
 * EggNode *a, const EggNode *b) { if (bin_number == 2) { bin 2, textured
 * geometry return (a->get_texture() < b->get_texture()); } else { bin 1,
 * untextured geometry return false; } } The actual comparison can be
 * arbitrary, as long as it is consistent.  Its only purpose is to assign some
 * ordering among bins.  In the example, for instance, the comparison is based
 * on the pointer to the texture maps--it doesn't matter which comes before
 * the other, as long as it's consistent.  In particular, it should never be
 * true that sorts_less(a, b) and sorts_less(b, a) both return true--that is a
 * clear contradiction.  Of course, if you're using sorts_less() anyway, you
 * could put *all* of the logic for binning into this function; there's no
 * need to use both get_bin_number() and sorts_less(), necessarily.  In the
 * current example, here's another version of sorts_less() that accomplishes
 * the same thing as the combined effects of the above get_bin_number() and
 * sorts_less() working together: virtual bool sorts_less(int bin_number,
 * const EggNode *a, const EggNode *b) { if (a->has_texture() !=
 * b->has_texture()) { return ((int)a->has_texture() < (int)b->has_texture());
 * } if (a->has_texture()) { return (a->get_texture() < b->get_texture()); }
 * return false; } virtual bool collapse_group(const EggGroup *group, int
 * bin_number); After all the nodes have been assigned to bins and the
 * individual bins (polysets) have been created, it might turn out that some
 * groups have had all their children placed into the same bin.  In this case,
 * the group node is now redundant, since it contains just the one child, the
 * new EggBin (polyset) node.  It might be advantageous to remove the group
 * and collapse its properties into the new node.  In this case (and this case
 * only), collapse_group() will be called, given the node and the bin number.
 * If it returns true, the node will indeed be collapsed into its bin;
 * otherwise, they will be left separate.  The point is that there might be
 * some attributes in the group node (for instance, a matrix transform) that
 * cannot be represented in a polyset node in the new scene graph format, so
 * there may be some cases in which the group cannot be safely collapsed.
 * Since the egg library cannot know about which such cases cause problems, it
 * leaves it up to you.  The default behavior is never to collapse nodes.
 * virtual string get_bin_name(int bin_number, EggNode *child); This function
 * is called as each new bin is created, to optionally define a name for the
 * new node.  If it returns the empty string, the node name will be empty,
 * unless it was collapsed with its parent group, in which case it will
 * inherit its former parent's name.  Once you have subclassed EggBinMaker and
 * defined the functions as you require, you use it by simply calling
 * make_bins() one or more times, passing it the pointer to the root of the
 * scene graph or of some subgraph.  It will traverse the subgraph and create
 * a series of EggBin objects, as required, moving all the binned geometry
 * under the EggBin objects.  The return value is the number of EggBins
 * created.  Each EggBin stores its bin number, which may be retrieved via
 * get_bin_number().
 */


#include "pandabase.h"

#include "eggObject.h"

#include "pointerTo.h"
#include "pnotify.h"

#include "pset.h"
#include "pmap.h"

class EggNode;
class EggGroup;
class EggGroupNode;
class EggBin;
class EggBinMaker;

/**
 * This is just an STL function object, used to sort nodes within EggBinMaker.
 * It's part of the private interface; ignore it.
 */
class EXPCL_PANDA_EGG EggBinMakerCompareNodes {
public:
  EggBinMakerCompareNodes() {
    // We need to have a default constructor to compile, but it should never
    // be called.
    nassertv(false);
  }
  EggBinMakerCompareNodes(EggBinMaker *ebm) : _ebm(ebm) { }
  bool operator ()(const EggNode *a, const EggNode *b) const;

  EggBinMaker *_ebm;
};


/**
 * This is a handy class for collecting related nodes together.  It is an
 * abstract class; to use it you must subclass off of it.  See the somewhat
 * lengthy comment above.
 */
class EXPCL_PANDA_EGG EggBinMaker : public EggObject {
PUBLISHED:
  EggBinMaker();
  ~EggBinMaker();

  int make_bins(EggGroupNode *root_group);

  virtual void
  prepare_node(EggNode *node);

  virtual int
  get_bin_number(const EggNode *node)=0;

  virtual bool
  sorts_less(int bin_number, const EggNode *a, const EggNode *b);

  virtual bool
  collapse_group(const EggGroup *group, int bin_number);

  virtual std::string
  get_bin_name(int bin_number, const EggNode *child);

  virtual PT(EggBin)
  make_bin(int bin_number, const EggNode *child, EggGroup *collapse_from);

private:
  // The logic is two-pass.  First, we make a scene graph traversal and store
  // all the pointers into the GroupNodesSortedNodes structure, which groups
  // nodes by their parent group, and then sorted into bin order.
  typedef pmultiset<PT(EggNode), EggBinMakerCompareNodes> SortedNodes;
  typedef pmap<EggGroupNode *, SortedNodes> GroupNodes;

  // Then we walk through that list and create a BinsNodes structure for each
  // group, which separates out the nodes into the individual bins.
  typedef pvector< PT(EggNode) > Nodes;
  typedef pvector<Nodes> Bins;

  void collect_nodes(EggGroupNode *group);
  int get_bins_for_group(GroupNodes::const_iterator gi);
  void make_bins_for_group(EggGroupNode *group, const Bins &bins);
  void setup_bin(EggBin *bin, const Nodes &nodes);

  GroupNodes _group_nodes;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    register_type(_type_handle, "EggBinMaker",
                  EggObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif
