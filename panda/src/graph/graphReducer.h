// Filename: graphReducer.h
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GRAPHREDUCER_H
#define GRAPHREDUCER_H

#include <pandabase.h>

#include "nodeRelation.h"

#include <typedObject.h>

///////////////////////////////////////////////////////////////////
// 	 Class : GraphReducer
// Description : A generic interface to simplify a graph in various
//               ways, generally as a performance optimization.  This
//               class is designed for generic graphs; also see the
//               SceneGraphReducer (which inherits from this class)
//               for a specific class to optimize renderable scene
//               graphs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphReducer {
public:
  GraphReducer(TypeHandle graph_type);
  virtual ~GraphReducer();

  int flatten(Node *root, bool combine_siblings);

protected:
  int r_flatten(Node *root, bool combine_siblings);
  int flatten_siblings(Node *root);

  virtual bool consider_arc(NodeRelation *arc);
  virtual bool consider_siblings(Node *parent,
				 NodeRelation *arc1, NodeRelation *arc2);

  virtual bool flatten_arc(NodeRelation *arc);
  virtual NodeRelation *collapse_siblings(Node *parent, NodeRelation *arc1,
					  NodeRelation *arc2);

  virtual Node *collapse_nodes(Node *node1, Node *node2, bool siblings);
  virtual void choose_name(Node *preserve, Node *source1, Node *source2);

protected:
  void move_children(Node *to, Node *from);
  void copy_children(Node *to, Node *from);

protected:
  TypeHandle _graph_type;
};

#endif

  
  
