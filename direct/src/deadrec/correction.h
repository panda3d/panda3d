// Filename: correction.h
// Created by:  cary (20Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __CORRECTION_H__
#define __CORRECTION_H__

#include "directbase.h"
#include "luse.h"

class Correction {
protected:
  LPoint3f _curr_p;
  LVector3f _curr_v;
PUBLISHED:
  Correction(LPoint3f&, LVector3f&);
  virtual ~Correction(void);

  virtual void step(void);
  virtual void new_target(LPoint3f&, LVector3f&);
  virtual void force_target(LPoint3f&, LVector3f&);

  LPoint3f get_pos(void) const;
  LVector3f get_vel(void) const;
};

class PopCorrection : public Correction {
PUBLISHED:
  PopCorrection(LPoint3f&, LVector3f&);
  virtual ~PopCorrection(void);

  virtual void step(void);
  virtual void new_target(LPoint3f&, LVector3f&);
  virtual void force_target(LPoint3f&, LVector3f&);
};

class LerpCorrection : public Correction {
private:
  LPoint3f prev_p, save_p;
  bool have_both;
  float time;
  float dur;
PUBLISHED:
  LerpCorrection(LPoint3f&, LVector3f&);
  virtual ~LerpCorrection(void);

  virtual void step(void);
  virtual void new_target(LPoint3f&, LVector3f&);
  virtual void force_target(LPoint3f&, LVector3f&);

  void set_duration(float);
  float get_duration(void) const;
};

class SplineCorrection : public Correction {
private:
  LPoint3f A, B, C, D;
  bool have_both;
  LPoint3f prev_p, save_p;
  LVector3f prev_v, save_v;
  float time;
  float dur;
PUBLISHED:
  SplineCorrection(LPoint3f&, LVector3f&);
  virtual ~SplineCorrection(void);

  virtual void step(void);
  virtual void new_target(LPoint3f&, LVector3f&);
  virtual void force_target(LPoint3f&, LVector3f&);

  void set_duration(float);
  float get_duration(void) const;
};

#endif /* __CORRECTION_H__ */
