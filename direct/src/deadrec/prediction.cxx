// Filename: prediction.cxx
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

#include "prediction.h"

Prediction::Prediction(LPoint3f& start) : _curr_p(start), _curr_v(0.0f, 0.0f, 0.0f) {
}

Prediction::~Prediction(void) {
}

void Prediction::step(void) {
}

void Prediction::new_telemetry(LPoint3f&) {
}

void Prediction::force_telemetry(LPoint3f&) {
}

LPoint3f Prediction::get_pos(void) const {
  return _curr_p;
}

LVector3f Prediction::get_vel(void) const {
  return _curr_v;
}

//////////////////////////////////////////////////////////////////////

NullPrediction::NullPrediction(LPoint3f& start) : Prediction(start) {
}

NullPrediction::~NullPrediction(void) {
}

void NullPrediction::step(void) {
}

void NullPrediction::new_telemetry(LPoint3f& t_pos) {
  _curr_v = t_pos - _curr_p;
  _curr_p = t_pos;
}

void NullPrediction::force_telemetry(LPoint3f& t_pos) {
  _curr_v = t_pos - _curr_p;
  _curr_p = t_pos;
}

//////////////////////////////////////////////////////////////////////

LinearPrediction::LinearPrediction(LPoint3f& start) : Prediction(start) {
}

LinearPrediction::~LinearPrediction(void) {
}

void LinearPrediction::step(void) {
}

void LinearPrediction::new_telemetry(LPoint3f&) {
}

void LinearPrediction::force_telemetry(LPoint3f&) {
}
