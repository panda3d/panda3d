// Filename: transparencyProperty.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRANSPARENCYPROPERTY_H
#define TRANSPARENCYPROPERTY_H

#include <pandabase.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : TransparencyProperty
// Description : This defines the types of transparency that can be
//               enabled.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransparencyProperty {
public:
  enum Mode {
    M_none,             // No transparency in effect.
    M_alpha,            // Writes to depth buffer of transp objects disabled
    M_alpha_sorted,     // Assumes transp objects are depth sorted
    M_multisample,      // Source alpha values modified to 1.0 before writing
    M_multisample_mask, // Source alpha values not modified
    M_binary,           // Only writes pixels with alpha = 1.0
  };

public:
  INLINE TransparencyProperty(Mode mode);
  INLINE TransparencyProperty(void);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const TransparencyProperty &other) const;
  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Mode _mode;
};

ostream &operator << (ostream &out, TransparencyProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const TransparencyProperty &prop) {
  prop.output(out);
  return out;
}

#include "transparencyProperty.I"

#endif
