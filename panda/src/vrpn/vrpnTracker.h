// Filename: vrpnTracker.h
// Created by:  drose (25Jan01)
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

#ifndef VRPNTRACKER_H
#define VRPNTRACKER_H

#include "pandabase.h"

#include "vrpn_interface.h"

#include "pvector.h"

class VrpnTrackerDevice;

////////////////////////////////////////////////////////////////////
//       Class : VrpnTracker
// Description : This is the actual interface to a particular VRPN
//               tracker object, and all of its sensors.  A pointer to
//               this object is stored in the VrpnClient class for
//               each differently-named VRPN tracker we connect to.
//
//               The VRPN callbacks go here, which in turn get
//               vectored out to any VrpnTrackerDevice objects that
//               register with this.  When the last VrpnTrackerDevice
//               object unregisters, the VrpnTracker will be deleted
//               by the VrpnClient.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnTracker {
public:
  VrpnTracker(const string &tracker_name, vrpn_Connection *connection);
  ~VrpnTracker();

  INLINE const string &get_tracker_name() const;
  INLINE bool is_empty() const;

  void mark(VrpnTrackerDevice *device);
  void unmark(VrpnTrackerDevice *device);

  INLINE void poll();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  static void VRPN_CALLBACK
  vrpn_position_callback(void *userdata, const vrpn_TRACKERCB info);
  static void VRPN_CALLBACK
  vrpn_velocity_callback(void *userdata, const vrpn_TRACKERVELCB info);
  static void VRPN_CALLBACK
  vrpn_acceleration_callback(void *userdata, const vrpn_TRACKERACCCB info);

private:
  string _tracker_name;
  vrpn_Tracker_Remote *_tracker;

  typedef pvector<VrpnTrackerDevice *> Devices;
  Devices _devices;
};

INLINE ostream &operator << (ostream &out, const VrpnTracker &tracker) {
  tracker.output(out);
  return out;
}

#include "vrpnTracker.I"

#endif

