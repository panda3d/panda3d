// Filename: correction.cxx
// Created by:  cary (20Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "correction.h"
#include <clockObject.h>

Correction::Correction(LPoint3f& start, LVector3f& s_vel) : _curr_p(start),
							    _curr_v(s_vel) {
}

Correction::~Correction(void) {
}

void Correction::step(void) {
}

void Correction::new_target(LPoint3f&, LVector3f&) {
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

/////////////////////////////////////////////////////////////////////

LerpCorrection::LerpCorrection(LPoint3f& start, LVector3f& s_vel)
  : Correction(start, s_vel), prev_p(start), save_p(start), have_both(false),
    time(0.) {
}

LerpCorrection::~LerpCorrection(void) {
}

void LerpCorrection::step(void) {
  if (have_both) {
    if (time < 0.5) {
      // half second lerp
      float tmp = time * 2.;
      LVector3f vtmp = save_p - prev_p;
      _curr_p = (tmp * vtmp) + prev_p;
      time += ClockObject::get_global_clock()->get_dt();
    }
  }
}

void LerpCorrection::new_target(LPoint3f& target, LVector3f&) {
  if (have_both) {
    time = 0.;
    prev_p = _curr_p;
    save_p = target;
  } else {
    save_p = target;
    _curr_p = prev_p;
    time = 0.;
    have_both = true;
  }
}

/////////////////////////////////////////////////////////////////////

SplineCorrection::SplineCorrection(LPoint3f& start, LVector3f& s_vel)
  : Correction(start, s_vel), have_both(false), prev_p(start), save_p(start),
    prev_v(s_vel), save_v(s_vel), time(0.) {
}

SplineCorrection::~SplineCorrection(void) {
}

void SplineCorrection::step(void) {
  if (have_both) {
    if (time < 0.5) {
      // half second lerp
      float tmp = time * 2.;
      _curr_p = (tmp * tmp * tmp * A) + (tmp * tmp * B) + (tmp * C) + D;
      _curr_v = (3. * tmp * tmp * A) + (2. * tmp * B) + C;
      time += ClockObject::get_global_clock()->get_dt();
    }
  }
}

void SplineCorrection::new_target(LPoint3f& target, LVector3f& v_target) {
  if (have_both) {
    time = 0.;
    prev_p = _curr_p;
    prev_v = _curr_v;
    save_p = target;
    save_v = v_target;
    A = (2. * (prev_p - save_p)) + prev_v + save_v;
    B = (3. * (save_p - prev_p)) - (2. * prev_v) - save_v;
    C = prev_v;
    D = prev_p;
  } else {
    save_p = target;
    save_v = v_target;
    _curr_p = prev_p;
    _curr_v = prev_v;
    time = 0.;
    A = (2. * (prev_p - save_p)) + prev_v + save_v;
    B = (3. * (save_p - prev_p)) - (2. * prev_v) - save_v;
    C = prev_v;
    D = prev_p;
    have_both = true;
  }
}
