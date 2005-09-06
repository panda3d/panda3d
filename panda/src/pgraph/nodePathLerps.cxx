// Filename: nodePathLerps.cxx
// Created by:  frang (01Jun00)
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

#include "nodePathLerps.h"

TypeHandle PosLerpFunctor::_type_handle;
TypeHandle HprLerpFunctor::_type_handle;
TypeHandle ScaleLerpFunctor::_type_handle;
TypeHandle ColorLerpFunctor::_type_handle;
TypeHandle PosHprLerpFunctor::_type_handle;
TypeHandle HprScaleLerpFunctor::_type_handle;
TypeHandle PosHprScaleLerpFunctor::_type_handle;
TypeHandle ColorScaleLerpFunctor::_type_handle;


PosLerpFunctor::PosLerpFunctor(const PosLerpFunctor& c)
  : LPoint3fLerpFunctor(c), _node_path(c._node_path) {}

PosLerpFunctor::~PosLerpFunctor()
{
}

PosLerpFunctor& PosLerpFunctor::operator=(const PosLerpFunctor& c) {
  _node_path = c._node_path;
  LPoint3fLerpFunctor::operator=(c);
  return *this;
}

void PosLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_pos(_wrt_path, interpolate(t));
  else
    _node_path.set_pos(interpolate(t));
}

HprLerpFunctor::HprLerpFunctor(const HprLerpFunctor& c)
  : LVecBase3fLerpFunctor(c), _node_path(c._node_path) {}

void HprLerpFunctor::take_shortest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_diff_cache[i] < -180.)
      _start[i] -= 360.;
    else if (this->_diff_cache[i] > 180.)
      _start[i] += 360.;
  this->_diff_cache = this->_end - this->_start;
}

void HprLerpFunctor::take_longest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_diff_cache[i] < 0.) && (this->_diff_cache[i] > -180.))
      _start[i] -= 360.;
    else if ((this->_diff_cache[i] >= 0.) && (this->_diff_cache[i] < 180))
      _start[i] += 360.;
  this->_diff_cache = this->_end - this->_start;
}

HprLerpFunctor::~HprLerpFunctor()
{
}

HprLerpFunctor& HprLerpFunctor::operator=(const HprLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase3fLerpFunctor::operator=(c);
  return *this;
}

void HprLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_hpr(_wrt_path, interpolate(t));
  else
    _node_path.set_hpr(interpolate(t));
}

ScaleLerpFunctor::ScaleLerpFunctor(const ScaleLerpFunctor& c)
  : LVecBase3fLerpFunctor(c), _node_path(c._node_path) {}

ScaleLerpFunctor::~ScaleLerpFunctor()
{
}

ScaleLerpFunctor& ScaleLerpFunctor::operator=(const ScaleLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase3fLerpFunctor::operator=(c);
  return *this;
}

void ScaleLerpFunctor::operator()(float t) {
  if (_is_wrt)
    _node_path.set_scale(_wrt_path, interpolate(t));
  else
    _node_path.set_scale(interpolate(t));
}

ColorLerpFunctor::ColorLerpFunctor(const ColorLerpFunctor& c)
  : LVecBase4fLerpFunctor(c), _node_path(c._node_path) {}

ColorLerpFunctor::~ColorLerpFunctor()
{
}

ColorLerpFunctor& ColorLerpFunctor::operator=(const ColorLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase4fLerpFunctor::operator=(c);
  return *this;
}

void ColorLerpFunctor::operator()(float t) {
        _node_path.set_color(interpolate(t));
}


PosHprLerpFunctor::PosHprLerpFunctor(const PosHprLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void PosHprLerpFunctor::take_shortest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void PosHprLerpFunctor::take_longest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

PosHprLerpFunctor::~PosHprLerpFunctor()
{
}

PosHprLerpFunctor& PosHprLerpFunctor::operator=(const PosHprLerpFunctor& c) {
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

void PosHprLerpFunctor::operator()(float t) {
  LPoint3f p = ((t * _pdiff_cache) + _pstart);
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  if (_is_wrt)
    _node_path.set_pos_hpr(_wrt_path, p, h);
  else
    _node_path.set_pos_hpr(p, h);
}

HprScaleLerpFunctor::HprScaleLerpFunctor(const HprScaleLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void HprScaleLerpFunctor::take_shortest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void HprScaleLerpFunctor::take_longest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

HprScaleLerpFunctor::~HprScaleLerpFunctor()
{
}

HprScaleLerpFunctor&
HprScaleLerpFunctor::operator=(const HprScaleLerpFunctor& c) {
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

void HprScaleLerpFunctor::operator()(float t) {
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  LVecBase3f s = ((t * _sdiff_cache) + _sstart);
  if (_is_wrt)
    _node_path.set_hpr_scale(_wrt_path, h, s);
  else
    _node_path.set_hpr_scale(h, s);
}

PosHprScaleLerpFunctor::PosHprScaleLerpFunctor(const PosHprScaleLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

void PosHprScaleLerpFunctor::take_shortest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if (this->_hdiff_cache[i] < -180.)
      _hstart[i] -= 360.;
    else if (this->_hdiff_cache[i] > 180.)
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

void PosHprScaleLerpFunctor::take_longest() {
  // so long as these are actually degrees
  for (int i=0; i!=3; ++i)
    if ((this->_hdiff_cache[i] < 0.) && (this->_hdiff_cache[i] > -180.))
      _hstart[i] -= 360.;
    else if ((this->_hdiff_cache[i] >= 0.) && (this->_hdiff_cache[i] < 180))
      _hstart[i] += 360.;
  this->_hdiff_cache = this->_hend - this->_hstart;
}

PosHprScaleLerpFunctor::~PosHprScaleLerpFunctor()
{
}

PosHprScaleLerpFunctor&
PosHprScaleLerpFunctor::operator=(const PosHprScaleLerpFunctor& c) {
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

void PosHprScaleLerpFunctor::operator()(float t) {
  LPoint3f p = ((t * _pdiff_cache) + _pstart);
  LVecBase3f h = ((t * _hdiff_cache) + _hstart);
  LVecBase3f s = ((t * _sdiff_cache) + _sstart);
  if (_is_wrt)
    _node_path.set_pos_hpr_scale(_wrt_path, p, h, s);
  else
    _node_path.set_pos_hpr_scale(p, h, s);
}

ColorScaleLerpFunctor::ColorScaleLerpFunctor(const ColorScaleLerpFunctor& c)
  : LVecBase4fLerpFunctor(c), _node_path(c._node_path) {}

ColorScaleLerpFunctor::~ColorScaleLerpFunctor()
{
}

ColorScaleLerpFunctor& ColorScaleLerpFunctor::operator=(const ColorScaleLerpFunctor& c) {
  _node_path = c._node_path;
  LVecBase4fLerpFunctor::operator=(c);
  return *this;
}

void ColorScaleLerpFunctor::operator()(float t) {
  _node_path.set_color_scale(interpolate(t));
}


