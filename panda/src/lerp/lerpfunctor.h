// Filename: lerpfunctor.h
// Created by:  frang (26May00)
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

#ifndef __LERPFUNCTOR_H__
#define __LERPFUNCTOR_H__

#include "pandabase.h"
#include "typedReferenceCount.h"

class EXPCL_PANDA LerpFunctor : public TypedReferenceCount {
public:
  LerpFunctor(void) {}
  LerpFunctor(const LerpFunctor&);
  virtual ~LerpFunctor(void);
  LerpFunctor& operator=(const LerpFunctor&);
  virtual void operator()(float) = 0;

PUBLISHED:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }

public:
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LerpFunctor",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
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
  virtual ~SimpleLerpFunctor(void);
  SimpleLerpFunctor<value>& operator=(const SimpleLerpFunctor<value>&);
  virtual void operator()(float);

PUBLISHED:
  value interpolate(float);
  INLINE const value &get_start() const { return _start; }
  INLINE const value &get_end() const { return _end; }

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LerpFunctor::init_type();
    do_init_type(value);
    ostringstream os;
    os << "SimpleLerpFunctor<" << get_type_handle(value).get_name() << ">";
    register_type(_type_handle, os.str(), LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
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
  virtual ~SimpleQueryLerpFunctor(void);
  SimpleQueryLerpFunctor<value>& operator=(const SimpleQueryLerpFunctor<value>&);
  virtual void operator()(float);
PUBLISHED:
  value get_value(void);

  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
public:
  static void init_type(void) {
    SimpleLerpFunctor<value>::init_type();
    ostringstream os;
    os << "SimpleQueryLerpFunctor<" << get_type_handle(value).get_name()
       << ">";
    register_type(_type_handle, os.str(),
                  SimpleLerpFunctor<value>::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

template <class value>
TypeHandle SimpleQueryLerpFunctor<value>::_type_handle;

#include "luse.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<int>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LPoint2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LPoint3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LPoint4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVector2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVector3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleLerpFunctor<LVector4f>)

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<int>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LPoint2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LPoint3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LPoint4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVector2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVector3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, SimpleQueryLerpFunctor<LVector4f>)

typedef SimpleLerpFunctor<int> IntLerpFunctor;
typedef SimpleLerpFunctor<float> FloatLerpFunctor;
typedef SimpleLerpFunctor<LPoint2f> LPoint2fLerpFunctor;
typedef SimpleLerpFunctor<LPoint3f> LPoint3fLerpFunctor;
typedef SimpleLerpFunctor<LPoint4f> LPoint4fLerpFunctor;
typedef SimpleLerpFunctor<LVecBase2f> LVecBase2fLerpFunctor;
typedef SimpleLerpFunctor<LVecBase3f> LVecBase3fLerpFunctor;
typedef SimpleLerpFunctor<LVecBase4f> LVecBase4fLerpFunctor;
typedef SimpleLerpFunctor<LVector2f> LVector2fLerpFunctor;
typedef SimpleLerpFunctor<LVector3f> LVector3fLerpFunctor;
typedef SimpleLerpFunctor<LVector4f> LVector4fLerpFunctor;

typedef SimpleQueryLerpFunctor<int> IntQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<float> FloatQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint2f> LPoint2fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint3f> LPoint3fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LPoint4f> LPoint4fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase2f> LVecBase2fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase3f> LVecBase3fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVecBase4f> LVecBase4fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector2f> LVector2fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector3f> LVector3fQueryLerpFunctor;
typedef SimpleQueryLerpFunctor<LVector4f> LVector4fQueryLerpFunctor;

#include "pset.h"
#include "pointerTo.h"

class EXPCL_PANDA MultiLerpFunctor : public LerpFunctor {
private:
  typedef pset< PT(LerpFunctor) > Functors;
  Functors _funcs;
public:
  MultiLerpFunctor(void) {}
  MultiLerpFunctor(const MultiLerpFunctor&);
  virtual ~MultiLerpFunctor(void);
  MultiLerpFunctor& operator=(const MultiLerpFunctor&);
  virtual void operator()(float);
  void add_functor(LerpFunctor*);
  void remove_functor(LerpFunctor*);

PUBLISHED:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }

public:
  static void init_type(void) {
    LerpFunctor::init_type();
    register_type(_type_handle, "MultiLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
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
SimpleLerpFunctor<value>::~SimpleLerpFunctor(void)
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
void SimpleLerpFunctor<value>::operator()(float) {
  // should not be here
}

template <class value>
value SimpleLerpFunctor<value>::interpolate(float t) {
  return ((t * _diff_cache) + _start);
}

/*
template <class value>
SimpleQueryLerpFunctor<value>::SimpleQueryLerpFunctor(const SimpleQueryLerpFunctor& c) : SimpleLerpFunctor<value>(c), _save(c._save) {}
*/

template <class value>
SimpleQueryLerpFunctor<value>::~SimpleQueryLerpFunctor(void)
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
void SimpleQueryLerpFunctor<value>::operator()(float t) {
  _save = interpolate(t);
}

template <class value>
value SimpleQueryLerpFunctor<value>::get_value(void) {
  return _save;
}

#endif /* __LERPFUNCTOR_H__ */
