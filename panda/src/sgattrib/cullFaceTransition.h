// Filename: cullFaceTransition.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CULLFACETRANSITION_H
#define CULLFACETRANSITION_H

#include <pandabase.h>

#include "cullFaceProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : CullFaceTransition
// Description : This controls how polygons are culled according to
//               the ordering of their vertices after projection (and,
//               hence, according to their facing relative to the
//               camera).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullFaceTransition : public OnTransition {
public:
  INLINE CullFaceTransition(CullFaceProperty::Mode mode);

  INLINE void set_mode(CullFaceProperty::Mode mode);
  INLINE CullFaceProperty::Mode get_mode() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  INLINE CullFaceTransition(void);
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  CullFaceProperty _value;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWritable *make_CullFaceTransition(const FactoryParams &params);

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
    OnTransition::init_type();
    register_type(_type_handle, "CullFaceTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class CullFaceAttribute;
};

#include "cullFaceTransition.I"

#endif
