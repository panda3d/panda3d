// Filename: materialTransition.h
// Created by:  mike (19Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef MATERIALTRANSITION_H
#define MATERIALTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>
#include <material.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : MaterialTransition
// Description : Applies a material, controlling subtle lighting
//               parameters, to geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MaterialTransition : public OnOffTransition {
public:
  INLINE MaterialTransition();
  INLINE MaterialTransition(const Material *material);
  INLINE static MaterialTransition off();

  INLINE void set_on(const Material *material);
  INLINE const Material *get_material() const;

public:  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  CPT(Material) _value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWritable &plist, 
                                BamReader *manager);

protected:
  static TypedWritable *make_MaterialTransition(const FactoryParams &params);
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
    register_type(_type_handle, "MaterialTransition",
                  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class MaterialAttribute;
};

#include "materialTransition.I"

#endif
