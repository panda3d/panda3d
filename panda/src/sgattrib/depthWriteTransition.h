// Filename: depthWriteTransition.h
// Created by:  drose (31Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DEPTHWRITETRANSITION_H
#define DEPTHWRITETRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : DepthWriteTransition
// Description : This enables or disables the writing to the depth
//               buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DepthWriteTransition : public OnOffTransition {
public:
  INLINE DepthWriteTransition();
  INLINE static DepthWriteTransition off();
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_DepthWriteTransition(const FactoryParams &params);

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
    OnOffTransition::init_type();
    register_type(_type_handle, "DepthWriteTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class DepthWriteAttribute;
};

#include "depthWriteTransition.I"

#endif
