// Filename: lerpfunctor.h
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

#ifndef __LERPFUNCTOR_H__
#define __LERPFUNCTOR_H__

#include "pandabase.h"
#include "typedReferenceCount.h"

class EXPCL_PANDA_LERP LerpFunctor : public TypedReferenceCount {
public:
  LerpFunctor() {}
  LerpFunctor(const LerpFunctor&);
  virtual ~LerpFunctor();
  LerpFunctor& operator=(const LerpFunctor&);
  virtual void operator()(PN_stdfloat) = 0;

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LerpFunctor",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

template <class value>
class SimpleLerpFunctor : public LerpFunctor {
protected:
  value _start;
  value _end;
  value _diff_cache;

  SimpleLerpFunctor(value start, value end) : LerpFunctor(), _start(start),
                                              _end(end), _diff_cache(end-start)
    {}
  SimpleLerpFunctor(const SimpleLerpFunctor<value>&);
public:
  virtual ~SimpleLerpFunctor();
  SimpleLerpFunctor<value>& operator=(const SimpleLerpFunctor<value>&);
  virtual void operator()(PN_stdfloat);

PUBLISHED:
  value interpolate(PN_stdfloat);
  INLINE const value &get_start() const { return _start; }
  INLINE const value &get_end() const { return _end; }

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LerpFunctor::init_type();
    do_init_type(value);
    ostringstream os;
    os << "SimpleLerpFunctor<" << get_type_handle(value).get_name() << ">";
    register_type(_type_handle, os.str(), LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

template <class value>
TypeHandle SimpleLerpFunctor<value>::_type_handle;

template <class value>
class SimpleQueryLerpFunctor : public SimpleLerpFunctor<value> {
private:
  value _save;
protected:
  /*
  SimpleQueryLerpFunctor(const SimpleQueryLerpFucntor<value>& c);
  */
public:
  SimpleQueryLerpFunctor(value start, value end)
    : SimpleLerpFunctor<value>(start, end) {}
  virtual ~SimpleQueryLerpFunctor();
  SimpleQueryLerpFunctor<value>& operator=(const SimpleQueryLerpFunctor<value>&);
  virtual void operator()(PN_stdfloat);
PUBLISHED:
  value get_value();

  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    SimpleLerpFunctor<value>::init_type();
    ostringstream os;
    os << "SimpleQueryLerpFunctor<" << get_type_handle(value).get_name()
       << ">";
    register_type(_type_handle, os.str(),
                  SimpleLerpFunctor<value>::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

template <class value>
TypeHandle SimpleQueryLerpFunctor<value>::_type_handle;

#include "luse.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<int>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<PN_stdfloat>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LPoint2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LPoint3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LPoint4>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVecBase2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVecBase3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVecBase4>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVector2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVector3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleLerpFunctor<LVector4>)

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<int>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<PN_stdfloat>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LPoint2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LPoint3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LPoint4>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVecBase2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVecBase3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVecBase4>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVector2>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVector3>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_LERP, EXPTP_PANDA_LERP, SimpleQueryLerpFunctor<LVector4>)

typedef SimpleLerpFunctor<int> IntLerpFunctor;
typedef SimpleLerpFunctor<PN_stdfloat> StdfloatLerpFunctor;
typedef SimpleLerpFunctor<LPoint2> LPoint2LerpFunctor;
typedef SimpleLerpFunctor<LPoint3> LPoint3LerpFunctor;
typedef SimpleLerpFunctor<LPoint4> LPoint4LerpFunctor;
typedef SimpleLerpFunctor<LVecBase2> LVecBase2LerpFunctor;
typedef SimpleLerpFunctor<LVecBase3> LVecBase3LerpFunctor;
typedef SimpleLerpFunctor<LVecBase4> LVecBase4LerpFunctor;
typedef SimpleLerpFunctor<LVector2> LVector2LerpFunctor;
typedef SimpleLerpFunctor<LVector3> LVector3LerpFunctor;
typedef SimpleLerpFunctor<LVector4> LVector4LerpFunctor;

typedef SimpleQueryLerpFunctor<int> IntQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<PN_stdfloat> StdfloatQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint2> LPoint2QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint3> LPoint3QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint4> LPoint4QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase2> LVecBase2QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase3> LVecBase3QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase4> LVecBase4QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector2> LVector2QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector3> LVector3QueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector4> LVector4QueryLerpFunctor;

#include "pset.h"
#include "pointerTo.h"

class EXPCL_PANDA_LERP MultiLerpFunctor : public LerpFunctor {
private:
  typedef phash_set< PT(LerpFunctor) > Functors;
  Functors _funcs;
public:
  MultiLerpFunctor() {}
  MultiLerpFunctor(const MultiLerpFunctor&);
  virtual ~MultiLerpFunctor();
  MultiLerpFunctor& operator=(const MultiLerpFunctor&);
  virtual void operator()(PN_stdfloat);
  void add_functor(LerpFunctor*);
  void remove_functor(LerpFunctor*);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    LerpFunctor::init_type();
    register_type(_type_handle, "MultiLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

//
// template implementation
//

template <class value>
SimpleLerpFunctor<value>::SimpleLerpFunctor(const SimpleLerpFunctor<value>& c)
  : LerpFunctor(c), _start(c._start), _end(c._end), _diff_cache(c._diff_cache)
  {}

template <class value>
SimpleLerpFunctor<value>::~SimpleLerpFunctor()
{
}

template <class value>
SimpleLerpFunctor<value>&
SimpleLerpFunctor<value>::operator=(const SimpleLerpFunctor& c) {
  _start = c._start;
  _end = c._end;
  _diff_cache = c._diff_cache;
  LerpFunctor::operator=(c);
  return *this;
}

template <class value>
void SimpleLerpFunctor<value>::operator()(PN_stdfloat) {
  // should not be here
}

template <class value>
value SimpleLerpFunctor<value>::interpolate(PN_stdfloat t) {
  return ((t * _diff_cache) + _start);
}

/*
template <class value>
SimpleQueryLerpFunctor<value>::SimpleQueryLerpFunctor(const SimpleQueryLerpFunctor& c) : SimpleLerpFunctor<value>(c), _save(c._save) {}
*/

template <class value>
SimpleQueryLerpFunctor<value>::~SimpleQueryLerpFunctor()
{
}

template <class value>
SimpleQueryLerpFunctor<value>&
SimpleQueryLerpFunctor<value>::operator=(const SimpleQueryLerpFunctor& c) {
  _save = c._save;
  SimpleLerpFunctor<value>::operator=(c);
  return *this;
}

template <class value>
void SimpleQueryLerpFunctor<value>::operator()(PN_stdfloat t) {
  _save = this->interpolate(t);
}

template <class value>
value SimpleQueryLerpFunctor<value>::get_value() {
  return _save;
}

#endif /* __LERPFUNCTOR_H__ */
