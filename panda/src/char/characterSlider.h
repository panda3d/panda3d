// Filename: characterSlider.h
// Created by:  drose (03Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef CHARACTERSLIDER_H
#define CHARACTERSLIDER_H

#include <pandabase.h>

#include <movingPartScalar.h>

////////////////////////////////////////////////////////////////////
//       Class : CharacterSlider
// Description : This is a morph slider within the character.  It's
//               simply a single floating-point value that animates
//               generally between 0 and 1, that controls the effects
//               of one or more morphs within the character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterSlider : public MovingPartScalar {
protected:
  CharacterSlider();
  CharacterSlider(const CharacterSlider &copy);

public:
  CharacterSlider(PartGroup *parent, const string &name);

  virtual PartGroup *make_copy() const;

  static void register_with_read_factory(void);

  static TypedWriteable *make_CharacterSlider(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovingPartScalar::init_type();
    register_type(_type_handle, "CharacterSlider",
		  MovingPartScalar::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


