// Filename: depthTestProperty.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DEPTHTESTPROPERTY_H
#define DEPTHTESTPROPERTY_H

#include <pandabase.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : DepthTestProperty
// Description : This defines the types of depth testing we can
//               enable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DepthTestProperty {
PUBLISHED:
  enum Mode {
    M_none,             // No depth test; may still write to depth buffer.
    M_never,            // Never draw.
    M_less,		// incoming < stored
    M_equal,		// incoming == stored
    M_less_equal, 	// incoming <= stored
    M_greater, 		// incoming > stored
    M_not_equal, 	// incoming != stored
    M_greater_equal,	// incoming >= stored
    M_always            // Always draw.  Same effect as none, more expensive.
  };

public:
  INLINE DepthTestProperty(Mode mode = M_none);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE int compare_to(const DepthTestProperty &other) const;

  void output(ostream &out) const;

public:
  void write_datagram(Datagram &destination);
  void read_datagram(DatagramIterator &source);

private:
  Mode _mode;
};

ostream &operator << (ostream &out, DepthTestProperty::Mode mode);

INLINE ostream &operator << (ostream &out, const DepthTestProperty &prop) {
  prop.output(out);
  return out;
}

#include "depthTestProperty.I"

#endif
