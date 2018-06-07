/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnButton.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef VRPNBUTTON_H
#define VRPNBUTTON_H

#include "pandabase.h"

#include "vrpn_interface.h"

#include "pvector.h"

class VrpnButtonDevice;

/**
 * This is the actual interface to a particular VRPN button device, and all of
 * its numbered buttons.  A pointer to this object is stored in the VrpnClient
 * class for each differently-named VRPN button device we connect to.
 *
 * The VRPN callbacks go here, which in turn get vectored out to any
 * VrpnButtonDevice objects that register with this.  When the last
 * VrpnButtonDevice object unregisters, the VrpnButton will be deleted by the
 * VrpnClient.
 *
 * This class does not need to be exported from the DLL.
 */
class VrpnButton {
public:
  VrpnButton(const std::string &button_name, vrpn_Connection *connection);
  ~VrpnButton();

  INLINE const std::string &get_button_name() const;
  INLINE bool is_empty() const;

  void mark(VrpnButtonDevice *device);
  void unmark(VrpnButtonDevice *device);

  INLINE void poll();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  static void VRPN_CALLBACK
  vrpn_button_callback(void *userdata, const vrpn_BUTTONCB info);

private:
  std::string _button_name;
  vrpn_Button_Remote *_button;

  typedef pvector<VrpnButtonDevice *> Devices;
  Devices _devices;
};

INLINE std::ostream &operator << (std::ostream &out, const VrpnButton &button) {
  button.output(out);
  return out;
}

#include "vrpnButton.I"

#endif
