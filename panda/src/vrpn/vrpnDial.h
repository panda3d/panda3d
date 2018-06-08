/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnDial.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef VRPNDIAL_H
#define VRPNDIAL_H

#include "pandabase.h"

#include "vrpn_interface.h"

#include "pvector.h"

class VrpnDialDevice;

/**
 * This is the actual interface to a particular VRPN dial device, and all of
 * its numbered dials.  A pointer to this object is stored in the VrpnClient
 * class for each differently-named VRPN dial device we connect to.
 *
 * The VRPN callbacks go here, which in turn get vectored out to any
 * VrpnDialDevice objects that register with this.  When the last
 * VrpnDialDevice object unregisters, the VrpnDial will be deleted by the
 * VrpnClient.
 *
 * This class does not need to be exported from the DLL.
 */
class VrpnDial {
public:
  VrpnDial(const std::string &dial_name, vrpn_Connection *connection);
  ~VrpnDial();

  INLINE const std::string &get_dial_name() const;
  INLINE bool is_empty() const;

  void mark(VrpnDialDevice *device);
  void unmark(VrpnDialDevice *device);

  INLINE void poll();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  static void VRPN_CALLBACK
  vrpn_dial_callback(void *userdata, const vrpn_DIALCB info);

private:
  std::string _dial_name;
  vrpn_Dial_Remote *_dial;

  typedef pvector<VrpnDialDevice *> Devices;
  Devices _devices;
};

INLINE std::ostream &operator << (std::ostream &out, const VrpnDial &dial) {
  dial.output(out);
  return out;
}

#include "vrpnDial.I"

#endif
