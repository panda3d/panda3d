// Filename: polygonOffsetProperty.h
// Created by:  jason (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POLYGONOFFSETPROPERTY_H
#define POLYGONOFFSETPROPERTY_H

#include <pandabase.h>

#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : PolygonOffsetProperty
// Description : This class defines the set state for polygon offseting
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolygonOffsetProperty {
public:
  INLINE PolygonOffsetProperty();
  INLINE PolygonOffsetProperty(int units, int factor);

  INLINE void set_units(int units);
  INLINE int get_units() const;
  INLINE void set_factor(int factor);
  INLINE int get_factor() const;

  INLINE int compare_to(const PolygonOffsetProperty &other) const;
  void output(ostream &out) const;

private:
  int _units;
  int _factor;
};

INLINE ostream &operator << (ostream &out, const PolygonOffsetProperty &prop) {
  prop.output(out);
  return out;
}

#include "polygonOffsetProperty.I"

#endif
