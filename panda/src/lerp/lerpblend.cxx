// Filename: lerpblend.cxx
// Created by:  frang (30May00)
// 
////////////////////////////////////////////////////////////////////

#include "lerpblend.h"

TypeHandle LerpBlendType::_type_handle;
TypeHandle EaseInBlendType::_type_handle;
TypeHandle EaseOutBlendType::_type_handle;
TypeHandle EaseInOutBlendType::_type_handle;
TypeHandle NoBlendType::_type_handle;

LerpBlendType::LerpBlendType(const LerpBlendType&) {}

LerpBlendType::~LerpBlendType(void) {}

LerpBlendType& LerpBlendType::operator=(const LerpBlendType&) {
  return *this;
}

float LerpBlendType::operator()(float t) {
  return t;
}

EaseInBlendType::EaseInBlendType(const EaseInBlendType& c) : LerpBlendType(c)
{
}

EaseInBlendType::~EaseInBlendType(void) {}

EaseInBlendType& EaseInBlendType::operator=(const EaseInBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float EaseInBlendType::operator()(float t) {
  float x = t*t;
  return ((3. * x) - (t * x)) * 0.5;
}

EaseOutBlendType::EaseOutBlendType(const EaseOutBlendType& c)
  : LerpBlendType(c) {}

EaseOutBlendType::~EaseOutBlendType(void) {}

EaseOutBlendType& EaseOutBlendType::operator=(const EaseOutBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float EaseOutBlendType::operator()(float t) {
  return ((3. * t) - (t * t * t)) * 0.5;
}

EaseInOutBlendType::EaseInOutBlendType(const EaseInOutBlendType& c)
  : LerpBlendType(c) {}

EaseInOutBlendType::~EaseInOutBlendType(void) {}

EaseInOutBlendType& EaseInOutBlendType::operator=(const EaseInOutBlendType& c)
{
  LerpBlendType::operator=(c);
  return *this;
}

float EaseInOutBlendType::operator()(float t) {
  float x = t*t;
  return (3. * x) - (2. * t * x);
}

NoBlendType::NoBlendType(const NoBlendType& c) : LerpBlendType(c) {}

NoBlendType::~NoBlendType(void) {}

NoBlendType& NoBlendType::operator=(const NoBlendType& c) {
  LerpBlendType::operator=(c);
  return *this;
}

float NoBlendType::operator()(float t) {
  return t;
}

