// Filename: pointShapeProperty.cxx
// Created by:  charles (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pointShapeProperty.h"

ostream &
operator << (ostream &out, PointShapeProperty::Mode mode) {
  switch (mode) {
  case PointShapeProperty::M_square:
    return out << "square";

  case PointShapeProperty::M_circle:
    return out << "circle";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
// Function : output
//   Access : public
////////////////////////////////////////////////////////////////////
void PointShapeProperty::
output(ostream &out) const {
  out << _mode;
}
