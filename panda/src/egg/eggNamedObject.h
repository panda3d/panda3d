// Filename: eggNamedObject.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGNAMEDOBJECT_H
#define EGGNAMEDOBJECT_H

#include <pandabase.h>

#include "eggObject.h"
#include <namable.h>
#include <referenceCount.h>

#ifndef WIN32_VC
class ostream;
#endif

////////////////////////////////////////////////////////////////////
// 	 Class : EggNamedObject
// Description : This is a fairly high-level base class--any egg
//               object that has a name.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggNamedObject : public EggObject, public Namable {
public:
  INLINE EggNamedObject(const string &name = "");
  INLINE EggNamedObject(const EggNamedObject &copy);
  INLINE EggNamedObject &operator = (const EggNamedObject &copy);

  void write_header(ostream &out, int indent_level,
		    const char *egg_keyword) const;


  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    Namable::init_type();
    register_type(_type_handle, "EggNamedObject",
		  EggObject::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggNamedObject.I"

#endif
