// Filename: pointShapeProperty.h
// Created by:  charles (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTSHAPEPROPERTY_H
#define POINTSHAPEPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : PointShapeProperty
// Description : Affects the shapes of point primitives
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PointShapeProperty {
public:

  enum Mode {
    M_square,
    M_circle
  };

public:

  INLINE PointShapeProperty(Mode mode);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode(void) const;

  INLINE int compare_to(const PointShapeProperty& other) const;
  void output(ostream &out) const;

private:

  Mode _mode;
};

ostream &operator << (ostream &out, PointShapeProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const PointShapeProperty &prop) {
  prop.output(out);
  return out;
}

#include "pointShapeProperty.I"

#endif // POINTSHAPEPROPERTY_H
