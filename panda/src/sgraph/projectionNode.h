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

  bool is_in_view(const LPoint3f &pos);

  void get_perspective_params(float &yfov, float &aspect, float &cnear, 
                              float &cfar) const;
  void get_perspective_params(float &xfov, float &yfov, float &aspect,
                              float &cnear, float &cfar) const;
  float get_hfov(void) const;
  float get_vfov(void) const;
  void set_fov(float hfov);
  void set_fov(float hfov, float vfov);
  void set_hfov(float hfov);
  void set_vfov(float vfov);
  float get_aspect(void) const;
  void set_aspect(float aspect);
  void get_near_far(float &cnear, float &cfar) const;
  void set_near_far(float cnear, float cfar);
  float get_near(void) const;
  void set_near(float cnear);
  float get_far(void) const;
  void set_far(float cfar);
  
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
