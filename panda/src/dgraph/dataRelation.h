// Filename: dataRelation.h
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DATARELATION_H
#define DATARELATION_H

#include <pandabase.h>

#include <nodeRelation.h>

///////////////////////////////////////////////////////////////////
// 	 Class : DataRelation
// Description : The arc type specific to data graphs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DataRelation : public NodeRelation {
public:
  INLINE DataRelation(Node *from, Node *to, int sort = 0);

protected:
  // Normally, this should only be used for passing to the factory.
  // Don't attempt to create an unattached arc directly.
  INLINE DataRelation();

public:
  // This is just to be called at initialization time; don't try to
  // call this directly.
  INLINE static void register_with_factory();
  
private:
  static NodeRelation *make_arc(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    NodeRelation::init_type();
    register_type(_type_handle, "DataRelation",
		 NodeRelation::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dataRelation.I"

#endif
