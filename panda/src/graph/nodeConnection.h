// Filename: nodeConnection.h
// Created by:  drose (07May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODECONNECTION_H
#define NODECONNECTION_H

#include <pandabase.h>

#include "nodeRelation.h"
#include "config_graph.h"
#include "vector_PT_NodeRelation.h"
#include "vector_NodeRelation_star.h"

typedef vector_PT_NodeRelation DownRelationPointers;
typedef vector_NodeRelation_star UpRelationPointers;


///////////////////////////////////////////////////////////////////
// 	 Class : NodeConnection
// Description : This class represents the table, stored within each
//               Node, of all the connected NodeRelations (arcs) of a
//               particular graph type.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeConnection {
public:
  INLINE_GRAPH NodeConnection(TypeHandle graph_type = TypeHandle::none());
  INLINE_GRAPH ~NodeConnection();

private:
  INLINE_GRAPH NodeConnection(const NodeConnection &copy);
  INLINE_GRAPH void operator = (const NodeConnection &copy);

public:
  INLINE_GRAPH bool is_empty() const;

  INLINE_GRAPH TypeHandle get_graph_type() const;
  INLINE_GRAPH void set_graph_type(TypeHandle graph_type);

  INLINE_GRAPH UpRelationPointers &get_up();
  INLINE_GRAPH const UpRelationPointers &get_up() const;

  INLINE_GRAPH DownRelationPointers &get_down();
  INLINE_GRAPH const DownRelationPointers &get_down() const;

private:
  TypeHandle _graph_type;
  UpRelationPointers _up;
  DownRelationPointers _down;
};

#ifndef DONT_INLINE_GRAPH
#include "nodeConnection.I"
#endif

#endif

