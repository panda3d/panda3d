// Filename: textureApplyProperty.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREAPPLYPROPERTY_H
#define TEXTUREAPPLYPROPERTY_H

#include <pandabase.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : TextureApplyProperty
// Description : Defines the way texture colors modify existing
//               geometry colors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureApplyProperty {
public:
  enum Mode {
    M_modulate,
    M_decal,
    M_blend,
    M_replace,
    M_add
  };

public:
  INLINE TextureApplyProperty(Mode mode);
  INLINE TextureApplyProperty(void);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const TextureApplyProperty &other) const;

  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Mode _mode;
};

ostream &operator << (ostream &out, TextureApplyProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const TextureApplyProperty &prop) {
  prop.output(out);
  return out;
}

#include "textureApplyProperty.I"

#endif
