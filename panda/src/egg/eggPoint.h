// Filename: eggPoint.h
// Created by:  drose (15Dec99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGPOINT_H
#define EGGPOINT_H

#include <pandabase.h>

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPoint
// Description : A single point, or a collection of points as defined
//               by a single <PointLight> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPoint : public EggPrimitive {
public:
  INLINE EggPoint(const string &name = "");
  INLINE EggPoint(const EggPoint &copy);
  INLINE EggPoint &operator = (const EggPoint &copy);

  virtual bool cleanup();

  virtual void write(ostream &out, int indent_level) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPoint",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
 
};

#include "eggPoint.I"

#endif
