// Filename: colorMaskProperty.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLORMASKPROPERTY_H
#define COLORMASKPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorMaskProperty
// Description : This defines the set of color planes that may be
//               active for writing to the color buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMaskProperty {
public:
  enum Mask {
    M_r = 0x001,
    M_g = 0x002,
    M_b = 0x004,
    M_a = 0x008
  };

  INLINE ColorMaskProperty(int mask);
  INLINE static ColorMaskProperty all_on();

  INLINE void set_mask(int mask);
  INLINE int get_mask() const;

  INLINE int compare_to(const ColorMaskProperty &other) const;
  void output(ostream &out) const;

private:
  int _mask;
};

INLINE ostream &operator << (ostream &out, const ColorMaskProperty &prop) {
  prop.output(out);
  return out;
}

#include "colorMaskProperty.I"

#endif
