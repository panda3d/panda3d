// Filename: nodePathLerps.cxx
// Created by:  frang (01Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "nodePathLerps.h"

TypeHandle PosLerpFunctor::_type_handle;
TypeHandle HprLerpFunctor::_type_handle;
TypeHandle ScaleLerpFunctor::_type_handle;
TypeHandle ColorLerpFunctor::_type_handle;
TypeHandle PosHprLerpFunctor::_type_handle;
TypeHandle PosHprScaleLerpFunctor::_type_handle;
TypeHandle ColorScaleLerpFunctor::_type_handle;


PosLerpFunctor::PosLerpFunctor(const PosLerpFunctor& c)
  : LPoint3fLerpFunctor(c), _node_path(c._node_path) {}

PosLerpFunctor::~PosLerpFunctor(void)
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

HprLerpFunctor::~HprLerpFunctor(void)
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

ScaleLerpFunctor::~ScaleLerpFunctor(void)
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

ColorLerpFunctor::~ColorLerpFunctor(void)
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

PosHprLerpFunctor::~PosHprLerpFunctor(void)
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

PosHprScaleLerpFunctor::PosHprScaleLerpFunctor(const PosHprScaleLerpFunctor& c)
  : LerpFunctor(c), _node_path(c._node_path) {}

PosHprScaleLerpFunctor::~PosHprScaleLerpFunctor(void)
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

ColorScaleLerpFunctor::~ColorScaleLerpFunctor(void)
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


