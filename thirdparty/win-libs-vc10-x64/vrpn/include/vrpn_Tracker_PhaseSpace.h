#ifndef VRPN_TRACKER_PHASESPACE_H
#define VRPN_TRACKER_PHASESPACE_H

#include "vrpn_Shared.h"
#include "vrpn_Tracker.h"

#ifdef  VRPN_INCLUDE_PHASESPACE

#include "../phasespace/owl.h"

const int vrpn_PhaseSpace_MAXMARKERS = 256;
const int vrpn_PhaseSpace_MAXRIGIDS = 32;
const int vrpn_PhaseSpace_MSGBUFSIZE = 1024;

class VRPN_API vrpn_Tracker_PhaseSpace : public vrpn_Tracker {

public:

  vrpn_Tracker_PhaseSpace(const char *name, 
                          vrpn_Connection *c,
                          const char* device,
                          float frequency,
                          int readflag,
                          int slaveflag=0);


  ~vrpn_Tracker_PhaseSpace();

  virtual void mainloop();

  bool addMarker(int sensor,int led_id);
  bool addRigidMarker(int sensor, int led_id, float x, float y, float z);
  bool startNewRigidBody(int sensor);
  bool enableTracker(bool enable);
  void setFrequency(float freq);

  static int VRPN_CALLBACK handle_update_rate_request(void *userdata, vrpn_HANDLERPARAM p);  

protected:

  vrpn_int32 r2s_map[vrpn_PhaseSpace_MAXRIGIDS]; //rigid body to sensor map
  int numRigids;
  int numMarkers;
  bool owlRunning;
  float frequency;
  bool readMostRecent;
  bool slave;

  OWLMarker markers[vrpn_PhaseSpace_MAXMARKERS];
  OWLRigid rigids[vrpn_PhaseSpace_MAXRIGIDS];

protected:
  
  virtual int get_report(void);
  virtual void send_report(void);
};
#endif

#endif
