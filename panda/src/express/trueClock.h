// Filename: trueClock.h
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRUECLOCK_H
#define TRUECLOCK_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : TrueClock
// Description : An interface to whatever real-time clock we might
//               have available in the current environment.  There is
//               only one TrueClock in existence, and it constructs
//               itself.
//
//               The TrueClock returns elapsed real time in seconds
//               since some undefined epoch.  Since it is not defined
//               at what time precisely the clock indicates zero, this
//               value can only be meaningfully used to measure
//               elapsed time, by sampling it at two different times
//               and subtracting.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TrueClock {
public:
  INLINE static TrueClock *get_ptr();
  double get_real_time() const;

protected:
  TrueClock();
  INLINE ~TrueClock();

  static TrueClock *_global_ptr;
};

#include "trueClock.I"
  
#endif
