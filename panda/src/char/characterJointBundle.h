// Filename: characterJointBundle.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef CHARACTERJOINTBUNDLE_H
#define CHARACTERJOINTBUNDLE_H

#include <pandabase.h>

#include <partBundle.h>
#include <partGroup.h>
#include <animControl.h>

class Character;

////////////////////////////////////////////////////////////////////
//       Class : CharacterJointBundle
// Description : The collection of all the joints and sliders in the
//               character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterJointBundle : public PartBundle {
protected:
  INLINE CharacterJointBundle(const CharacterJointBundle &copy);

public:
  CharacterJointBundle(const string &name = "");

PUBLISHED:
  INLINE Character *get_node() const;

public:
  virtual PartGroup *make_copy() const;

  static void register_with_read_factory(void);

  static TypedWriteable *make_CharacterJointBundle(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PartBundle::init_type();
    register_type(_type_handle, "CharacterJointBundle",
		  PartBundle::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "characterJointBundle.I"

#endif


