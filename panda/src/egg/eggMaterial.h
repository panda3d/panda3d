// Filename: eggMaterial.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMATERIAL_H
#define EGGMATERIAL_H

#include <pandabase.h>

#include "eggNode.h"

#include <luse.h>

///////////////////////////////////////////////////////////////////
// 	 Class : EggMaterial
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggMaterial : public EggNode {
public:
  EggMaterial(const string &mref_name);
  EggMaterial(const EggMaterial &copy);

  virtual void write(ostream &out, int indent_level) const;

  INLINE void set_diff(const Colorf &diff);
  INLINE void clear_diff();
  INLINE bool has_diff() const;
  INLINE Colorf get_diff() const;

private:
  bool _has_diff;
  Colorf _diff;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggMaterial",
		  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggMaterial.I"

#endif
