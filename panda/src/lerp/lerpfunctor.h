// Filename: lerpfunctor.h
// Created by:  frang (26May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LERPFUNCTOR_H__
#define __LERPFUNCTOR_H__

#include <pandabase.h>
#include <typedReferenceCount.h>

class EXPCL_PANDA LerpFunctor : public TypedReferenceCount {
public:
  LerpFunctor(void) {}
  LerpFunctor(const LerpFunctor&);
  virtual ~LerpFunctor(void);
  LerpFunctor& operator=(const LerpFunctor&);
  virtual void operator()(float) = 0;
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
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
private:
  value _start;
  value _end;
  value _diff_cache;

protected:
  value interpolate(float);
  SimpleLerpFunctor(value start, value end) : LerpFunctor(), _start(start),
					      _end(end), _diff_cache(end-start)
    {}
  SimpleLerpFunctor(const SimpleLerpFunctor<value>&);
public:
  virtual ~SimpleLerpFunctor(void);
  SimpleLerpFunctor<value>& operator=(const SimpleLerpFunctor<value>&);
  virtual void operator()(float) = 0;
public:
  // now for typehandle stuff
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

#include <luse.h>

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

#include <set>
#include <pointerTo.h>

class EXPCL_PANDA MultiLerpFunctor : public LerpFunctor {
private:
  typedef set<PT(LerpFunctor)> Functors;
  Functors _funcs;
public:
  MultiLerpFunctor(void) {}
  MultiLerpFunctor(const MultiLerpFunctor&);
  virtual ~MultiLerpFunctor(void);
  MultiLerpFunctor& operator=(const MultiLerpFunctor&);
  virtual void operator()(float);
  void add_functor(LerpFunctor*);
  void remove_functor(LerpFunctor*);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
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

#endif /* __LERPFUNCTOR_H__ */
