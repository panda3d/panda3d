// Filename: projectionNode.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef PROJECTIONNODE_H
#define PROJECTIONNODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <namedNode.h>
#include <projection.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : ProjectionNode
// Description : A node that has a frustum
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ProjectionNode : public NamedNode {
PUBLISHED:
  INLINE ProjectionNode(const string &name = "");

public:
  INLINE ProjectionNode(const ProjectionNode &copy);
  INLINE void operator = (const ProjectionNode &copy);
	
  virtual Node *make_copy() const;

PUBLISHED:  
  void set_projection(const Projection &projection);
  void share_projection(Projection *projection);
  Projection *get_projection();
  
protected:
  PT(Projection) _projection;
  
public:
  
  static TypeHandle get_class_type( void ) {
    return _type_handle;
  }
  static void init_type( void ) {
    NamedNode::init_type();
    register_type( _type_handle, "ProjectionNode",
		   NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type( void ) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  
  static TypeHandle                       _type_handle;
};

#include "projectionNode.I"

#endif
