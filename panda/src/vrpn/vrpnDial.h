// Filename: vrpnDial.h
// Created by:  drose (26Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef VRPNDIAL_H
#define VRPNDIAL_H

#include "pandabase.h"

#include "vrpn_interface.h"

#include "pvector.h"

class VrpnDialDevice;

////////////////////////////////////////////////////////////////////
//       Class : VrpnDial
// Description : This is the actual interface to a particular VRPN
//               dial device, and all of its numbered dials.  A
//               pointer to this object is stored in the VrpnClient
//               class for each differently-named VRPN dial device
//               we connect to.
//
//               The VRPN callbacks go here, which in turn get
//               vectored out to any VrpnDialDevice objects that
//               register with this.  When the last VrpnDialDevice
//               object unregisters, the VrpnDial will be deleted
//               by the VrpnClient.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnDial {
public:
  VrpnDial(const string &dial_name, vrpn_Connection *connection);
  ~VrpnDial();

  INLINE const string &get_dial_name() const;
  INLINE bool is_empty() const;

  void mark(VrpnDialDevice *device);
  void unmark(VrpnDialDevice *device);

  INLINE void poll();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  static void
  vrpn_dial_callback(void *userdata, const vrpn_DIALCB info);

private:
  string _dial_name;
  vrpn_Dial_Remote *_dial;

  typedef pvector<VrpnDialDevice *> Devices;
  Devices _devices;
};

INLINE ostream &operator << (ostream &out, const VrpnDial &dial) {
  dial.output(out);
  return out;
}

#include "vrpnDial.I"

#endif
