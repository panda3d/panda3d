// Filename: prediction.h
// Created by:  cary (20Dec00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __PREDICTION_H__
#define __PREDICTION_H__

#include <directbase.h>
#include <luse.h>

class Prediction {
protected:
  LPoint3f _curr_p;
  LVector3f _curr_v;
PUBLISHED:
  Prediction(LPoint3f&);
  virtual ~Prediction(void);

  virtual void step(void);
  virtual void new_telemetry(LPoint3f&);
  virtual void force_telemetry(LPoint3f&);

  LPoint3f get_pos(void) const;
  LVector3f get_vel(void) const;
};

class NullPrediction : public Prediction {
PUBLISHED:
  NullPrediction(LPoint3f&);
  virtual ~NullPrediction(void);

  virtual void step(void);
  virtual void new_telemetry(LPoint3f&);
  virtual void force_telemetry(LPoint3f&);
};

class LinearPrediction : public Prediction {
PUBLISHED:
  LinearPrediction(LPoint3f&);
  virtual ~LinearPrediction(void);

  virtual void step(void);
  virtual void new_telemetry(LPoint3f&);
  virtual void force_telemetry(LPoint3f&);
};

#endif /* __PREDICTION_H__ */
