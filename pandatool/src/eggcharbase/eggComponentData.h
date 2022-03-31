/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComponentData.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGCOMPONENTDATA_H
#define EGGCOMPONENTDATA_H

#include "pandatoolbase.h"

#include "eggObject.h"
#include "namable.h"
#include "pset.h"

class EggCharacterCollection;
class EggCharacterData;
class EggBackPointer;
class NameUniquifier;

/**
 * This is the base class of both EggJointData and EggSliderData.  It
 * represents a single component of a character, either a joint or a slider,
 * along with back pointers to the references to this component in all model
 * and animation egg files read.
 */
class EggComponentData : public EggObject, public Namable {
public:
  EggComponentData(EggCharacterCollection *collection,
                   EggCharacterData *char_data);
  virtual ~EggComponentData();

  void add_name(const std::string &name, NameUniquifier &uniquifier);
  bool matches_name(const std::string &name) const;

  int get_num_frames(int model_index) const;
  void extend_to(int model_index, int num_frames) const;
  double get_frame_rate(int model_index) const;

  virtual void add_back_pointer(int model_index, EggObject *egg_object)=0;
  virtual void write(std::ostream &out, int indent_level = 0) const=0;

  INLINE int get_num_models() const;
  INLINE bool has_model(int model_index) const;
  INLINE EggBackPointer *get_model(int model_index) const;
  void set_model(int model_index, EggBackPointer *back);

protected:

  // This points back to all the egg structures that reference this particular
  // table or slider.
  typedef pvector<EggBackPointer *> BackPointers;
  BackPointers _back_pointers;

  typedef pset<std::string> Names;
  Names _names;

  EggCharacterCollection *_collection;
  EggCharacterData *_char_data;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    register_type(_type_handle, "EggComponentData",
                  EggObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggComponentData.I"

#endif
