// Filename: cullFaceProperty.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CULLFACEPROPERTY_H
#define CULLFACEPROPERTY_H

#include <pandabase.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : CullFaceProperty
// Description : This defines the ways we can cull faces according to
//               their vertex ordering (and hence orientation relative
//               to the camera).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullFaceProperty {
public:
  enum Mode {
    M_cull_none,                // Cull no polygons
    M_cull_clockwise, 		// Cull clockwise-oriented polygons
    M_cull_counter_clockwise,  	// Cull counter-clockwise-oriented polygons
    M_cull_all, 	// Cull all polygons (other primitives are still drawn)
  };

public:
  INLINE CullFaceProperty(Mode mode);
  INLINE CullFaceProperty(void);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const CullFaceProperty &other) const;
  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Mode _mode;
};

ostream &operator << (ostream &out, CullFaceProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const CullFaceProperty &prop) {
  prop.output(out);
  return out;
}

#include "cullFaceProperty.I"

#endif
