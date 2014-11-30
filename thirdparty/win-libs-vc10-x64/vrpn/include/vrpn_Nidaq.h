/*****************************************************************************\
  vrpn_Nidaq.h
  --
  NOTICE: This class is superceded by vrpn_NationalInstruments.h and is
		  now deprecated.

  Description : This class reads from a National Instruments D/A Card
		(NIDAQ).  To compile this class, you must have the
		following directories in your include path:
		  ~tracker/hiball/nidaq/
		  ~tracker/hiball/src/libs/libgb (for uptime.h)
		  ~tracker/hiball/src/hybrid/ (for daq.h)
		And you must link in:
		  ~tracker/hiball/src/libs/libgb/uptime.cpp
		  ~tracker/hiball/src/hybrid/daq.cpp
		  ~tracker/hiball/nidaq/nidaq32.lib

  ----------------------------------------------------------------------------
  Author: weberh
  Created: Fri Jan 29 10:00:00 1999
  Revised: Fri Mar 19 14:45:55 1999 by weberh
\*****************************************************************************/

#ifndef VRPN_NIDAQ
#define VRPN_NIDAQ
#if defined(_WIN32) || defined(WIN32)
#if defined(VRPN_USE_NIDAQ)
#include "vrpn_Analog.h"
#include <daq.h>
#include <windows.h>

class VRPN_API vrpn_Nidaq : public vrpn_Analog {
public:
	// see daq.h for more info on the args
	// fNice says whether these threads should use 100% of the cpu or
	// whether they should sleep for 1 ms each time thru their loops
	// (the net effect is that they add 1 ms of uncertainty to the
	// existing 1 or 1/2 ms of uncertainty in time-stamps across a
	// synchronized vrpn connection).  If fNice is set, then 
  // the max theoretical reporting rate is 1000 hz.
	vrpn_Nidaq(char * pchName, vrpn_Connection * pConnection,
	     double dSamplingRate=100.0, double dInterChannelRate=100000.0, 
	     short sDeviceNumber=DAQ::DEF_DEVICE, int cChannels=10, 
	     short rgsChan[]=DAQ::DEF_CHANS_DIFF, 
	     short rgsGain[]=DAQ::DEF_GAINS,
	     short sInputMode=DAQ::DIFFERENTIAL, 
	     short sPolarity=DAQ::BIPOLAR,
		   int fNice=0);

  ~vrpn_Nidaq();
  void mainloop();
  int doing_okay();

protected:
  void report_changes();
  
private:
  DAQSample daqSample;
  DAQ *pDAQ;
  // value to add to UpTime calls to get into vrpn_gettimeofday timeframe
  struct timeval tvOffset;
  
  // Data, threadshell, and function used by extra daq getSample thread.
  // the crit section is used to protect the analog buffer from simultaneous
  // access by the two nidaq threads.
  CRITICAL_SECTION csAnalogBuffer;
  HANDLE hDAQThread;
  static unsigned __stdcall vrpn_Nidaq::runThread(void *pVrpnNidaq);
  void runNidaq();
  int fStop;
  int fNewData;
  double dSampleTime;

  // this controls whether we give up about 1 ms worth of timestamp accuracy
  // in exchange for cpu utilization going from 100% to 4% (or so)
  int fNice;
};

#endif // def(VRPN_USE_NIDAQ)
#endif // def(_WIN32) || def(WIN32)
#endif // ndef(VRPN_NIDAQ)
