// Filename: trackerData.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRACKER_DATA
#define TRACKER_DATA

#include <pandabase.h>
#include <luse.h>

class EXPCL_PANDA TrackerData {
public:
  INLINE TrackerData();
  INLINE TrackerData(const TrackerData &copy);
  INLINE TrackerData &operator = (const TrackerData &copy);

  INLINE static const TrackerData &none();

  double ptime;
  LPoint3f position;
  LVector4f pquat;
  
  double vtime;
  LVector3f velocity;
  LVector4f vquat;
  float vquat_dt;

  double atime;
  LVector3f acceleration;
  LVector4f aquat;
  float aquat_dt;

private:
  static TrackerData _none;
};

#include "trackerData.I"

#endif
