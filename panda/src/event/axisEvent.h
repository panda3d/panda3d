/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file axisEvent.h
 * @author jb
 * @date 2023-11-24
 */

#ifndef AXISEVENT_H
#define AXISEVENT_H

#include "pandabase.h"

class Datagram;
class DatagramIterator;

/**
 * Records a axis movement event.
 */
class EXPCL_PANDA_EVENT AxisEvent {
public:
  AxisEvent() = default;
  AxisEvent(int index, double value);

  void output(std::ostream &out) const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

public:
  int index;
  double value;
};

INLINE std::ostream &operator << (std::ostream &out, const AxisEvent &ae) {
  ae.output(out);
  return out;
}

#endif
