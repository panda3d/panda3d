// Filename: lerpblend.cxx
// Created by:  frang (30May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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

PN_stdfloat LerpBlendType::operator()(PN_stdfloat t) {
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

PN_stdfloat EaseInBlendType::operator()(PN_stdfloat t) {
  PN_stdfloat x = t*t;
  return ((3.0f * x) - (t * x)) * 0.5f;
}

EaseOutBlendType::EaseOutBlendType(const EaseOutBlendType& c)
  : LerpBlendType(c) {}

EaseOutBlendType::~EaseOutBlendType() {}

EaseOutBlendType& EaseOutBlendType::operator=(const EaseOutBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat EaseOutBlendType::operator()(PN_stdfloat t) {
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

PN_stdfloat EaseInOutBlendType::operator()(PN_stdfloat t) {
  PN_stdfloat x = t*t;
  return (3.0f * x) - (2.0f * t * x);
}

NoBlendType::NoBlendType(const NoBlendType& c) : LerpBlendType(c) {}

NoBlendType::~NoBlendType() {}

NoBlendType& NoBlendType::operator=(const NoBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat NoBlendType::operator()(PN_stdfloat t) {
  return t;
}

