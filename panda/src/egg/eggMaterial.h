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

  INLINE void set_diff(const RGBColorf &diff);
  INLINE void clear_diff();
  INLINE bool has_diff() const;
  INLINE RGBColorf get_diff() const;

  INLINE void set_amb(const RGBColorf &amb);
  INLINE void clear_amb();
  INLINE bool has_amb() const;
  INLINE RGBColorf get_amb() const;

  INLINE void set_emit(const RGBColorf &emit);
  INLINE void clear_emit();
  INLINE bool has_emit() const;
  INLINE RGBColorf get_emit() const;

  INLINE void set_spec(const RGBColorf &spec);
  INLINE void clear_spec();
  INLINE bool has_spec() const;
  INLINE RGBColorf get_spec() const;

private:
  enum Flags {
    F_diff    = 0x001,
    F_amb     = 0x002,
    F_emit    = 0x004,
    F_spec    = 0x008
  };

  RGBColorf _diff;
  RGBColorf _amb;
  RGBColorf _emit;
  RGBColorf _spec;
  int _flags;


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
