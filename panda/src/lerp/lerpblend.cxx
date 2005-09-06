// Filename: lerpblend.cxx
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

#include "lerpblend.h"

TypeHandle LerpBlendType::_type_handle;
TypeHandle EaseInBlendType::_type_handle;
TypeHandle EaseOutBlendType::_type_handle;
TypeHandle EaseInOutBlendType::_type_handle;
TypeHandle NoBlendType::_type_handle;

LerpBlendType::LerpBlendType(const LerpBlendType&) {}

LerpBlendType::~LerpBlendType() {}

LerpBlendType& LerpBlendType::operator=(const LerpBlendType&) {
  return *this;
}

float LerpBlendType::operator()(float t) {
  return t;
}

EaseInBlendType::EaseInBlendType(const EaseInBlendType& c) : LerpBlendType(c)
{
}

EaseInBlendType::~EaseInBlendType() {}

EaseInBlendType& EaseInBlendType::operator=(const EaseInBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float EaseInBlendType::operator()(float t) {
  float x = t*t;
  return ((3.0f * x) - (t * x)) * 0.5f;
}

EaseOutBlendType::EaseOutBlendType(const EaseOutBlendType& c)
  : LerpBlendType(c) {}

EaseOutBlendType::~EaseOutBlendType() {}

EaseOutBlendType& EaseOutBlendType::operator=(const EaseOutBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float EaseOutBlendType::operator()(float t) {
  return ((3.0f * t) - (t * t * t)) * 0.5f;
}

EaseInOutBlendType::EaseInOutBlendType(const EaseInOutBlendType& c)
  : LerpBlendType(c) {}

EaseInOutBlendType::~EaseInOutBlendType() {}

EaseInOutBlendType& EaseInOutBlendType::operator=(const EaseInOutBlendType& c)
{
  LerpBlendType::operator=(c);
  return *this;
}

float EaseInOutBlendType::operator()(float t) {
  float x = t*t;
  return (3.0f * x) - (2.0f * t * x);
}

NoBlendType::NoBlendType(const NoBlendType& c) : LerpBlendType(c) {}

NoBlendType::~NoBlendType() {}

NoBlendType& NoBlendType::operator=(const NoBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float NoBlendType::operator()(float t) {
  return t;
}

