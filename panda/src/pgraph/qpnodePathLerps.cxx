// Filename: qpnodePathLerps.cxx
// Created by:  frang (01Jun00)
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

#include "qpnodePathLerps.h"

TypeHandle qpPosLerpFunctor::_type_handle;
TypeHandle qpHprLerpFunctor::_type_handle;
TypeHandle qpScaleLerpFunctor::_type_handle;
TypeHandle qpColorLerpFunctor::_type_handle;
TypeHandle qpPosHprLerpFunctor::_type_handle;
TypeHandle qpHprScaleLerpFunctor::_type_handle;
TypeHandle qpPosHprScaleLerpFunctor::_type_handle;
TypeHandle qpColorScaleLerpFunctor::_type_handle;


qpPosLerpFunctor::qpPosLerpFunctor(const qpPosLerpFunctor& c)
  : LPoint3fLerpFunctor(c), _node_path(c._node_path) {}

qpPosLerpFunctor::~qpPosLerpFunctor(void)
{
}

qpPosLerpFunctor& qpPosLerpFunctor::operator=(const qpPosLerpFunctor& c) {
  _node_path = c._node_path;
  LPoint3fLerpFunctor::operator=(c);
  return *this;
}

void qpPosLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_pos(_wrt_path, interpolate(t));
  else
    _node_path.set_pos(interpolate(t));
}

qpHprLerpFunctor::qpHprLerpFunctor(const qpHprLerpFunctor& c)
  : LVecBase3fLerpFunctor(c), _node_path(c._node_path) {}

void qpHprLerpFunctor::take_shortest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_diff_cache[i] < -180.)
      _start[i] -= 360.;
    else if (this->_diff_cache[i] > 180.)
      _start[i] += 360.;
  this->_diff_cache = this->_end - this->_start;
}

void qpHprLerpFunctor::take_longest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_diff_cache[i] < 0.) && (this->_diff_cache[i] > -180.))
      _start[i] -= 360.;
    else if ((this->_diff_cache[i] >= 0.) && (this->_diff_cache[i] < 180))
      _start[i] += 360.;
  this->_diff_cache = this->_end - this->_start;
}

qpHprLerpFunctor::~qpHprLerpFunctor(void)
{
}

qpHprLerpFunctor& qpHprLerpFunctor::operator=(const qpHprLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase3fLerpFunctor::operator=(c);
  return *this;
}

void qpHprLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_hpr(_wrt_path, interpolate(t));
  else
    _node_path.set_hpr(interpolate(t));
}

qpScaleLerpFunctor::qpScaleLerpFunctor(const qpScaleLerpFunctor& c)
  : LVecBase3fLerpFunctor(c), _node_path(c._node_path) {}

qpScaleLerpFunctor::~qpScaleLerpFunctor(void)
{
}

qpScaleLerpFunctor& qpScaleLerpFunctor::operator=(const qpScaleLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase3fLerpFunctor::operator=(c);
  return *this;
}

void qpScaleLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_scale(_wrt_path, interpolate(t));
  else
    _node_path.set_scale(interpolate(t));
}

qpColorLerpFunctor::qpColorLerpFunctor(const qpColorLerpFunctor& c)
  : LVecBase4fLerpFunctor(c), _node_path(c._node_path) {}

qpColorLerpFunctor::~qpColorLerpFunctor(void)
{
}

qpColorLerpFunctor& qpColorLerpFunctor::operator=(const qpColorLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase4fLerpFunctor::operator=(c);
  return *this;
}

void qpColorLerpFunctor::operator()(float t) {
        _node_path.set_color(interpolate(t));
}


qpPosHprLerpFunctor::qpPosHprLerpFunctor(const qpPosHprLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void qpPosHprLerpFunctor::take_shortest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void qpPosHprLerpFunctor::take_longest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

qpPosHprLerpFunctor::~qpPosHprLerpFunctor(void)
{
}

qpPosHprLerpFunctor& qpPosHprLerpFunctor::operator=(const qpPosHprLerpFunctor& c) {
  _node_path = c._node_path;
  _pstart = c._pstart;
  _pend = c._pend;
  _pdiff_cache = c._pdiff_cache;
  _hstart = c._hstart;
  _hend = c._hend;
  _hdiff_cache = c._hdiff_cache;
  LerpFunctor::operator=(c);
  return *this;
}

void qpPosHprLerpFunctor::operator()(float t) {
  LPoint3f p = ((t * _pdiff_cache) + _pstart);
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  if (_is_wrt)
    _node_path.set_pos_hpr(_wrt_path, p, h);
  else
    _node_path.set_pos_hpr(p, h);
}

qpHprScaleLerpFunctor::qpHprScaleLerpFunctor(const qpHprScaleLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void qpHprScaleLerpFunctor::take_shortest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void qpHprScaleLerpFunctor::take_longest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

qpHprScaleLerpFunctor::~qpHprScaleLerpFunctor(void)
{
}

qpHprScaleLerpFunctor&
qpHprScaleLerpFunctor::operator=(const qpHprScaleLerpFunctor& c) {
  _node_path = c._node_path;
  _hstart = c._hstart;
  _hend = c._hend;
  _hdiff_cache = c._hdiff_cache;
  _sstart = c._sstart;
  _send = c._send;
  _sdiff_cache = c._sdiff_cache;
  LerpFunctor::operator=(c);
  return *this;
}

void qpHprScaleLerpFunctor::operator()(float t) {
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  LVecBase3f s = ((t * _sdiff_cache) + _sstart);
  if (_is_wrt)
    _node_path.set_hpr_scale(_wrt_path, h, s);
  else
    _node_path.set_hpr_scale(h, s);
}

qpPosHprScaleLerpFunctor::qpPosHprScaleLerpFunctor(const qpPosHprScaleLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void qpPosHprScaleLerpFunctor::take_shortest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void qpPosHprScaleLerpFunctor::take_longest(void) {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

qpPosHprScaleLerpFunctor::~qpPosHprScaleLerpFunctor(void)
{
}

qpPosHprScaleLerpFunctor&
qpPosHprScaleLerpFunctor::operator=(const qpPosHprScaleLerpFunctor& c) {
  _node_path = c._node_path;
  _pstart = c._pstart;
  _pend = c._pend;
  _pdiff_cache = c._pdiff_cache;
  _hstart = c._hstart;
  _hend = c._hend;
  _hdiff_cache = c._hdiff_cache;
  _sstart = c._sstart;
  _send = c._send;
  _sdiff_cache = c._sdiff_cache;
  LerpFunctor::operator=(c);
  return *this;
}

void qpPosHprScaleLerpFunctor::operator()(float t) {
  LPoint3f p = ((t * _pdiff_cache) + _pstart);
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  LVecBase3f s = ((t * _sdiff_cache) + _sstart);
  if (_is_wrt)
    _node_path.set_pos_hpr_scale(_wrt_path, p, h, s);
  else
    _node_path.set_pos_hpr_scale(p, h, s);
}

qpColorScaleLerpFunctor::qpColorScaleLerpFunctor(const qpColorScaleLerpFunctor& c)
  : LVecBase4fLerpFunctor(c), _node_path(c._node_path) {}

qpColorScaleLerpFunctor::~qpColorScaleLerpFunctor(void)
{
}

qpColorScaleLerpFunctor& qpColorScaleLerpFunctor::operator=(const qpColorScaleLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase4fLerpFunctor::operator=(c);
  return *this;
}

void qpColorScaleLerpFunctor::operator()(float t) {
  _node_path.set_color_scale(interpolate(t));
}


