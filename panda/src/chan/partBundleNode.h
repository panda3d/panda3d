/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundleNode.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef PARTBUNDLENODE_H
#define PARTBUNDLENODE_H

#include "pandabase.h"

#include "partBundle.h"
#include "partBundleHandle.h"

#include "pandaNode.h"
#include "dcast.h"
#include "pvector.h"

/**
 * This is a node that contains a pointer to an PartBundle.  Like
 * AnimBundleNode, it exists to make it easy to store PartBundles in the scene
 * graph.
 *
 * (Unlike AnimBundleNode, however, PartBundleNode has an additional function:
 * it is also the base class of the Character node type, which adds additional
 * functionality.)
 */
class EXPCL_PANDA_CHAN PartBundleNode : public PandaNode {
PUBLISHED:
  INLINE explicit PartBundleNode(const std::string &name, PartBundle *bundle);

protected:
  INLINE PartBundleNode();
  INLINE PartBundleNode(const PartBundleNode &copy);

public:
  virtual ~PartBundleNode();
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual void xform(const LMatrix4 &mat);

PUBLISHED:
  INLINE int get_num_bundles() const;
  INLINE PartBundle *get_bundle(int n) const;
  MAKE_SEQ(get_bundles, get_num_bundles, get_bundle);
  INLINE PartBundleHandle *get_bundle_handle(int n) const;
  MAKE_SEQ(get_bundle_handles, get_num_bundles, get_bundle_handle);

  MAKE_SEQ_PROPERTY(bundles, get_num_bundles, get_bundle);
  MAKE_SEQ_PROPERTY(bundle_handles, get_num_bundles, get_bundle_handle);

protected:
  void add_bundle(PartBundle *bundle);
  void add_bundle_handle(PartBundleHandle *handle);
  void steal_bundles(PartBundleNode *other);
  virtual void update_bundle(PartBundleHandle *old_bundle_handle,
                             PartBundle *new_bundle);

protected:
  typedef pvector< PT(PartBundleHandle) > Bundles;
  Bundles _bundles;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PartBundleNode",
                  PandaNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "partBundleNode.I"

#endif
