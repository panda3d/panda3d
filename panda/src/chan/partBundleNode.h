// Filename: partBundleNode.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef PARTBUNDLENODE_H
#define PARTBUNDLENODE_H

#include <pandabase.h>

#include "partBundle.h"

#include <namedNode.h>

////////////////////////////////////////////////////////////////////
//       Class : PartBundleNode
// Description : This is a node that contains a pointer to an
//               PartBundle.  Like AnimBundleNode, it exists solely to
//               make it easy to store PartBundles in the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PartBundleNode : public NamedNode {
public:
  INLINE PartBundleNode(const string &name, PartBundle *bundle);

protected:
  INLINE PartBundleNode();
  INLINE PartBundleNode(const PartBundleNode &copy);

public:
  virtual bool safe_to_flatten() const;

PUBLISHED:
  INLINE PartBundle *get_bundle() const;

private:
  PT(PartBundle) _bundle;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWritable &plist, 
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
    NamedNode::init_type();
    register_type(_type_handle, "PartBundleNode",
                  NamedNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "partBundleNode.I"

#endif
