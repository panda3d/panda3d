// Filename: renderRelation.h
// Created by:  drose (26Oct98)
//

#ifndef RENDERRELATION_H
#define RENDERRELATION_H

#include <pandabase.h>

#include <nodeRelation.h>
#include <luse.h>

///////////////////////////////////////////////////////////////////
// 	 Class : RenderRelation
// Description : The arc type specific to renderable scene graphs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderRelation : public NodeRelation {
public:
  INLINE RenderRelation(Node *from, Node *to, int sort = 0);

protected:
  // Normally, this should only be used for passing to the factory.
  // Don't attempt to create an unattached arc directly.
  INLINE RenderRelation();

public:
  virtual void changed_transition(TypeHandle transition_type);

protected:
  virtual void recompute_bound();

public:
  // This is just to be called at initialization time; don't try to
  // call this directly.
  INLINE static void register_with_factory();
  
private:
  static NodeRelation *make_arc(const FactoryParams &params);

public:
  static void register_with_read_factory(void);

  static TypedWriteable *make_RenderRelation(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    NodeRelation::init_type();
    register_type(_type_handle, "RenderRelation",
		 NodeRelation::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "renderRelation.I"

#endif
