// Filename: node.h
// Created by:  drose (26Oct98)
//

#ifndef NODE_H
#define NODE_H

#include <pandabase.h>

#include "nodeRelation.h"
#include "boundedObject.h"

#include <typedWriteable.h>
#include <referenceCount.h>
#include <luse.h>

class NodeAttributes;
class GraphicsStateGuardianBase;
class AllAttributesWrapper;
class AllTransitionsWrapper;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : Node
// Description : The base class for all scene graph nodes.  A Node may
//               be joined to any number of other nodes, as a parent
//               or as a child, with any kind of relation arcs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Node : public TypedWriteable, public BoundedObject,
	     virtual public ReferenceCount {
  // We can't simply inherit from TypedWriteableReferenceCount, because we need
  // to inherit virtually from ReferenceCount.
public:
  static Node* const Null;

  Node();
  Node(const Node &copy);
  void operator = (const Node &copy);
  virtual ~Node();

  virtual Node *make_copy() const;
  Node *copy_subgraph(TypeHandle graph_type) const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void transform_changed(NodeRelation *arc);

  int get_num_parents(TypeHandle type) const;
  NodeRelation *get_parent(TypeHandle type, int index) const;
  int get_num_children(TypeHandle type) const;
  NodeRelation *get_child(TypeHandle type, int index) const;

  // These functions will be called when the node is visited during
  // the indicated traversal.
  virtual void app_traverse() { }
  virtual void draw_traverse() { }
  virtual void dgraph_traverse() { }

  // This function is similar to another function in NodeTransition.
  // It may or may not intercept the render traversal.
  virtual bool sub_render(const AllAttributesWrapper &attrib,
			  AllTransitionsWrapper &trans,
			  GraphicsStateGuardianBase *gsgbase);
  virtual bool has_sub_render() const;

  virtual void output(ostream &out) const;

  // We reference-count the child pointer, but not the parent pointer,
  // to avoid circular reference counting.

  // The following members are public to allow low-overhead traversal
  // through the scene graph when necessary, but beware!  It is not
  // safe to access these members directly outside of PANDA.DLL.
  // Instead, use the get_parent()/get_child() interface, above.

  UpRelations _parents;
  DownRelations _children;

protected:
  virtual void propagate_stale_bound();

  typedef map<Node *, Node *> InstanceMap;
  virtual Node *r_copy_subgraph(TypeHandle graph_type, 
				InstanceMap &inst_map) const;
  virtual void r_copy_children(const Node *from, TypeHandle graph_type,
			       InstanceMap &inst_map);

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_Node(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  //This variable is set inside fillin to the number of pointers
  //that are "read" by Node.  This needs to be set, even though
  //node does nothing in complete_pointers, because it needs to
  //return the number of pointer requests that it made.  This 
  //is necessary because the vector of pointers to TypedWriteables
  //that is given to it by BamReader, will be filled with those requests
  //and any children of Node, need to be able to ignore those as well.
  //So complete_pointers must return the correct number read
  int _num_pointers;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteable::init_type();
    BoundedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "Node",
		  TypedWriteable::get_class_type(),
		  BoundedObject::get_class_type(),
		  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &
operator << (ostream &out, const Node &node) {
  node.output(out);
  return out;
}

EXPCL_PANDA NodeRelation *
find_arc(Node *parent, Node *child, TypeHandle graph_type);

INLINE bool
remove_child(Node *parent, Node *child, TypeHandle graph_type);


#include "node.I"

#endif
 























