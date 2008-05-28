// Filename: lerpfunctor.cxx
// Created by:  frang (26May00)
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

#include "lerpfunctor.h"

TypeHandle LerpFunctor::_type_handle;
TypeHandle MultiLerpFunctor::_type_handle;

LerpFunctor::LerpFunctor(const LerpFunctor&)
{
}

LerpFunctor::~LerpFunctor()
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

MultiLerpFunctor::~MultiLerpFunctor() {}

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
