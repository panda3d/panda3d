// Filename: colorBlendProperty.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLORBLENDPROPERTY_H
#define COLORBLENDPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ColorBlendProperty
// Description : This defines the types of color blending we can
//               perform.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorBlendProperty {
public:
  enum Mode {
    M_none,		// Blending is disabled
    M_multiply, 	// color already in fbuffer * incoming color
    M_add, 	 	// color already in fbuffer + incoming color
    M_multiply_add, 	// color already in fbuffer * incoming color +
		     	//   color already in fbuffer
    M_alpha,		// ????
  };

public:
  INLINE ColorBlendProperty(Mode mode);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const ColorBlendProperty &other) const;
  void output(ostream &out) const;

private:
  Mode _mode;
};

ostream &operator << (ostream &out, ColorBlendProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const ColorBlendProperty &prop) {
  prop.output(out);
  return out;
}

#include "colorBlendProperty.I"

#endif
