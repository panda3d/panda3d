// Filename: depthTestTransition.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef DEPTHTESTTRANSITION_H
#define DEPTHTESTTRANSITION_H

#include <pandabase.h>

#include "depthTestProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : DepthTestTransition
// Description : This transition controls the nature of the test
//               against the depth buffer.  It does not affect whether
//               the depth buffer will be written to or not; that is
//               handled by a separate transition,
//               DepthWriteTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DepthTestTransition : public OnTransition {
PUBLISHED:
  INLINE DepthTestTransition(DepthTestProperty::Mode mode);

  INLINE void set_mode(DepthTestProperty::Mode mode);
  INLINE DepthTestProperty::Mode get_mode() const;

public:  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  INLINE DepthTestTransition();
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  DepthTestProperty _value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);  

  static TypedWriteable *make_DepthTestTransition(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    register_type(_type_handle, "DepthTestTransition",
		  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class DepthTestAttribute;
};

#include "depthTestTransition.I"

#endif


