// Filename: correction.cxx
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

#include "correction.h"
#include "clockObject.h"

#include "notifyCategoryProxy.h"
NotifyCategoryDeclNoExport(correction);
NotifyCategoryDef(correction, "");

Correction::Correction(LPoint3f& start, LVector3f& s_vel) : _curr_p(start),
                                                            _curr_v(s_vel) {
  if(correction_cat.is_debug())
     correction_cat->debug() << "construction with:" << endl << "start = "
                             << start << endl << "vel = " << s_vel << endl;
}

Correction::~Correction(void) {
}

void Correction::step(void) {
}

void Correction::new_target(LPoint3f&, LVector3f&) {
}

void Correction::force_target(LPoint3f&, LVector3f&) {
}

LPoint3f Correction::get_pos(void) const {
  return _curr_p;
}

LVector3f Correction::get_vel(void) const {
  return _curr_v;
}

////////////////////////////////////////////////////////////////////

PopCorrection::PopCorrection(LPoint3f& start, LVector3f& s_vel)
  : Correction(start, s_vel) {
}

PopCorrection::~PopCorrection(void) {
}

void PopCorrection::step(void) {
}

void PopCorrection::new_target(LPoint3f& target, LVector3f&) {
  _curr_p = target;
}

void PopCorrection::force_target(LPoint3f& target, LVector3f&) {
  _curr_p = target;
}

/////////////////////////////////////////////////////////////////////

LerpCorrection::LerpCorrection(LPoint3f& start, LVector3f& s_vel)
  : Correction(start, s_vel), prev_p(start), save_p(start), have_both(false),
    time(0.0f), dur(0.5f) {
}

LerpCorrection::~LerpCorrection(void) {
}

void LerpCorrection::step(void) {
  if (have_both) {
    if (time < dur) {
      float tmp = time / dur;
      LVector3f vtmp = save_p - prev_p;
      _curr_p = (tmp * vtmp) + prev_p;
      time += ClockObject::get_global_clock()->get_dt();
    }
  }
}

void LerpCorrection::new_target(LPoint3f& target, LVector3f&) {
  if (target == save_p)
    return;
  if (have_both) {
    time = 0.0f;
    prev_p = _curr_p;
    save_p = target;
  } else {
    save_p = target;
    _curr_p = prev_p;
    time = 0.0f;
    have_both = true;
  }
}

void LerpCorrection::force_target(LPoint3f& target, LVector3f&) {
  if (target == save_p)
    return;
  _curr_p = prev_p = save_p = target;
  have_both = false;
  time = 0.0f;
}

void LerpCorrection::set_duration(float d) {
  dur = d;
}

float LerpCorrection::get_duration(void) const {
  return dur;
}

/////////////////////////////////////////////////////////////////////

SplineCorrection::SplineCorrection(LPoint3f& start, LVector3f& s_vel)
  : Correction(start, s_vel), have_both(false), prev_p(start), save_p(start),
    prev_v(s_vel), save_v(s_vel), time(0.0f), dur(0.5f) {
}

SplineCorrection::~SplineCorrection(void) {
}

void SplineCorrection::step(void) {
  if (have_both) {
    if (time < dur) {
      float tmp = time / dur;
      _curr_p = (tmp * tmp * tmp * A) + (tmp * tmp * B) + (tmp * C) + D;
      _curr_v = (3.0f * tmp * tmp * A) + (2.0f * tmp * B) + C;
      time += ClockObject::get_global_clock()->get_dt();
      if(correction_cat.is_spam()) {
         correction_cat->spam() << "new position = " << _curr_p << endl;
         correction_cat->spam() << "new vel = " << _curr_v << endl;
      }
    } else
     if(correction_cat.is_spam())
      correction_cat->spam() << "time >= dur, holding at current pos" << endl;
  } else
    if(correction_cat.is_spam())
      correction_cat->spam() << "have_both is false, no point calculated" << endl;
}

void SplineCorrection::new_target(LPoint3f& target, LVector3f& v_target) {
  if (target == save_p) {
    if(correction_cat.is_spam())
        correction_cat->spam() << "new target: same point, no action" << endl;
    return;
  }
  if (have_both) {
    time = 0.0f;
    prev_p = _curr_p;
    prev_v = _curr_v;
    save_p = target;
    save_v = v_target;
    A = (2.0f * (prev_p - save_p)) + prev_v + save_v;
    B = (3.0f * (save_p - prev_p)) - (2.0f * prev_v) - save_v;
    C = prev_v;
    D = prev_p;
    if(correction_cat.is_debug()) {
        correction_cat->debug() << "new target: already had 'both'." << endl;
        correction_cat->debug() << "target = " << target << endl;
        correction_cat->debug() << "vel = " << v_target << endl;
        correction_cat->debug() << "A = " << A << endl;
        correction_cat->debug() << "B = " << B << endl;
        correction_cat->debug() << "C = " << C << endl;
        correction_cat->debug() << "D = " << D << endl;
    }
  } else {
    save_p = target;
    save_v = v_target;
    _curr_p = prev_p;
    _curr_v = prev_v;
    time = 0.0f;
    A = (2.0f * (prev_p - save_p)) + prev_v + save_v;
    B = (3.0f * (save_p - prev_p)) - (2.0f * prev_v) - save_v;
    C = prev_v;
    D = prev_p;
    have_both = true;
    if(correction_cat.is_debug()) {
        correction_cat->debug() << "new target: now have 'both'." << endl;
        correction_cat->debug() << "target = " << target << endl;
        correction_cat->debug() << "vel = " << v_target << endl;
        correction_cat->debug() << "A = " << A << endl;
        correction_cat->debug() << "B = " << B << endl;
        correction_cat->debug() << "C = " << C << endl;
        correction_cat->debug() << "D = " << D << endl;
    }
  }
}

void SplineCorrection::force_target(LPoint3f& target, LVector3f& v_target) {
  if (target == save_p) {
    if(correction_cat.is_debug())
       correction_cat->debug() << "force target: same point, no action" << endl;
    return;
  }
  _curr_p = prev_p = save_p = target;
  _curr_v = prev_v = save_v = v_target;
  have_both = false;
  time = 0.0f;
}

void SplineCorrection::set_duration(float d) {
  dur = d;
}

float SplineCorrection::get_duration(void) const {
  return dur;
}
