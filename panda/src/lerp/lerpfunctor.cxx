// Filename: lerpfunctor.cxx
// Created by:  frang (26May00)
// 
////////////////////////////////////////////////////////////////////

#include "lerpfunctor.h"

TypeHandle LerpFunctor::_type_handle;
TypeHandle MultiLerpFunctor::_type_handle;

LerpFunctor::LerpFunctor(const LerpFunctor&)
{
}

LerpFunctor::~LerpFunctor(void)
{
}

LerpFunctor& LerpFunctor::operator=(const LerpFunctor&) {
  return *this;
}

void LerpFunctor::operator()(float) {
  // should not be here
}

MultiLerpFunctor::MultiLerpFunctor(const MultiLerpFunctor& c)
  : LerpFunctor(c), _funcs(c._funcs) {}

MultiLerpFunctor::~MultiLerpFunctor(void) {}

MultiLerpFunctor& MultiLerpFunctor::operator=(const MultiLerpFunctor& c) {
  _funcs = c._funcs;
  LerpFunctor::operator=(c);
  return *this;
}

void MultiLerpFunctor::operator()(float f) {
  for (Functors::iterator i=_funcs.begin(); i!=_funcs.end(); ++i)
    (*(*i))(f);
}

void MultiLerpFunctor::add_functor(LerpFunctor* func) {
  _funcs.insert(func);
}

void MultiLerpFunctor::remove_functor(LerpFunctor* func) {
  _funcs.erase(func);
}
