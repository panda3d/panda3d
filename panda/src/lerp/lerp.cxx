// Filename: lerp.cxx
// Created by:  frang (30May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "lerp.h"

#include "clockObject.h"
#include "throw_event.h"

TypeHandle Lerp::_type_handle;
TypeHandle AutonomousLerp::_type_handle;

static INLINE float scale_t(float t, float start, float end) {
  float ret = t;
  if (ret < start)
    ret = start;
  if (ret > end)
    ret = end;
  // Avoid a possible divide-by-zero
  if (end == 0.0f)
      return 0.0f;
  return ret / end;
}

Lerp::Lerp(LerpFunctor* func, float endt, LerpBlendType* blend)
  : _blend(blend), _func(func), _startt(0.), _endt(endt),
    _delta(1.), _t(0.) {}

Lerp::Lerp(LerpFunctor* func, float startt, float endt,
           LerpBlendType* blend) : _blend(blend), _func(func),
                                   _startt(startt),
                                   _endt(endt),
                                   _delta(1.),
                                   _t(startt) {}

Lerp::Lerp(const Lerp& c) : _blend(c._blend), _func(c._func), _event(c._event),
                            _startt(c._startt), _endt(c._endt),
                            _delta(c._delta), _t(c._t) {}

Lerp::~Lerp(void) {}

Lerp& Lerp::operator=(const Lerp& c) {
  _blend = c._blend;
  _func = c._func;
  _event = c._event;
  _startt = c._startt;
  _endt = c._endt;
  _delta = c._delta;
  _t = c._t;
  return *this;
}

void Lerp::step(void) {
  _t += _delta;
  if (is_done()) {
    (*_func)(1.0);
    if (!_event.empty()) {
      throw_event(_event);
    }
  } else {
    float t = scale_t(_t, _startt, _endt);
    t = (_blend==(LerpBlendType*)0L)?t:(*_blend)(t);
    (*_func)(t);
  }
}

void Lerp::set_step_size(float delta) {
  _delta = delta;
}

float Lerp::get_step_size(void) const {
  return _delta;
}

void Lerp::set_t(float t) {
  _t = t;
  float x = scale_t(_t, _startt, _endt);
  x = (_blend==(LerpBlendType*)0L)?x:(*_blend)(x);
  (*_func)(x);
}

float Lerp::get_t(void) const {
  return _t;
}

bool Lerp::is_done(void) const {
  return (_t >= _endt);
}

LerpFunctor* Lerp::get_functor(void) const {
  return _func;
}

void Lerp::set_end_event(const std::string& event) {
  _event = event;
}

std::string Lerp::get_end_event(void) const {
  return _event;
}

AutonomousLerp::AutonomousLerp(LerpFunctor* func, float endt,
                               LerpBlendType* blend, EventHandler* handler)
  : _blend(blend), _func(func), _handler(handler),
    _startt(0.), _endt(endt), _t(0.) {}

AutonomousLerp::AutonomousLerp(LerpFunctor* func, float startt, float endt,
                               LerpBlendType* blend, EventHandler* handler)
  : _blend(blend), _func(func), _handler(handler),
    _startt(startt), _endt(endt), _t(startt) {}

AutonomousLerp::AutonomousLerp(const AutonomousLerp& c) : _blend(c._blend),
                                                          _func(c._func),
                                                          _handler(c._handler),
                                                          _event(c._event),
                                                          _startt(c._startt),
                                                          _endt(c._endt),
                                                          _t(c._t) {}

AutonomousLerp::~AutonomousLerp(void) {}

AutonomousLerp& AutonomousLerp::operator=(const AutonomousLerp& c) {
  _blend = c._blend;
  _func = c._func;
  _handler = c._handler;
  _event = c._event;
  _startt = c._startt;
  _endt = c._endt;
  _t = c._t;
  return *this;
}

void AutonomousLerp::start(void) {
  _t = _startt;
  _handler->add_hook("NewFrame", handle_event, this);
}

void AutonomousLerp::stop(void) {
  _handler->remove_hook("NewFrame", handle_event, this);
}

void AutonomousLerp::resume(void) {
  _handler->add_hook("NewFrame", handle_event, this);
}

bool AutonomousLerp::is_done(void) const {
  return (_t >= _endt);
}

LerpFunctor* AutonomousLerp::get_functor(void) const {
  return _func;
}

void AutonomousLerp::set_t(float t) {
  _t = t;
  float x = scale_t(_t, _startt, _endt);
  x = (_blend==(LerpBlendType*)0L)?x:(*_blend)(x);
  (*_func)(x);
}

float AutonomousLerp::get_t(void) const {
  return _t;
}

void AutonomousLerp::set_end_event(const std::string& event) {
  _event = event;
}

std::string AutonomousLerp::get_end_event(void) const {
  return _event;
}

void AutonomousLerp::step(void) {
  // Probably broken because it does not set the final value when t
  // exceeds end_t. see fixed Lerp::step() above
  if (is_done()) {
    stop();
    return;
  }
  float delta = ClockObject::get_global_clock()->get_dt();
  _t += delta;
  float t = scale_t(_t, _startt, _endt);
  t = (_blend==(LerpBlendType*)0L)?t:(*_blend)(t);
  (*_func)(t);
  if (is_done() && !_event.empty())
    throw_event(_event);
}

void AutonomousLerp::handle_event(CPT(Event), void* data) {
  AutonomousLerp* l = (AutonomousLerp*)data;
  l->step();
}
