// Filename: vrpnTracker.h
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPNTRACKER_H
#define VRPNTRACKER_H

#include <pandabase.h>

#include "vrpn_interface.h"

#include <vector>

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
  static void
  vrpn_position_callback(void *userdata, const vrpn_TRACKERCB info);
  static void
  vrpn_velocity_callback(void *userdata, const vrpn_TRACKERVELCB info);
  static void
  vrpn_acceleration_callback(void *userdata, const vrpn_TRACKERACCCB info);

private:
  string _tracker_name;
  vrpn_Tracker_Remote *_tracker;

  typedef vector<VrpnTrackerDevice *> Devices;
  Devices _devices;
};

INLINE ostream &operator << (ostream &out, const VrpnTracker &tracker) {
  tracker.output(out);
  return out;
}

#include "vrpnTracker.I"

#endif

