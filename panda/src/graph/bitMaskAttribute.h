// Filename: bitMaskAttribute.h
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BITMASKATTRIBUTE_H
#define BITMASKATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

template<class MaskType>
class BitMaskTransition;

////////////////////////////////////////////////////////////////////
// 	 Class : BitMaskAttribute
// Description : 
////////////////////////////////////////////////////////////////////
template<class MaskType>
class BitMaskAttribute : public NodeAttribute {
protected:
  INLINE BitMaskAttribute();
  INLINE BitMaskAttribute(const MaskType &mask);
  INLINE BitMaskAttribute(const BitMaskAttribute &copy);
  INLINE void operator = (const BitMaskAttribute &copy);

public:
  INLINE void set_mask(const MaskType &mask);
  INLINE const MaskType &get_mask() const;

  virtual void output(ostream &out) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

private:
  MaskType _mask;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    MaskType::init_type();
    register_type(_type_handle, 
		  string("BitMaskAttribute<") +
		  MaskType::get_class_type().get_name() + ">",
		  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class BitMaskTransition<MaskType>;
};

#include "bitMaskAttribute.I"

#endif
