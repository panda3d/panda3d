// Filename: prediction.cxx
// Created by:  cary (20Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "prediction.h"

Prediction::Prediction(LPoint3f& start) : _curr_p(start), _curr_v(0., 0., 0.) {
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
