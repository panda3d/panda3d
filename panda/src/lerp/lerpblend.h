// Filename: lerpblend.h
// Created by:  frang (30May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LERPBLEND_H__
#define __LERPBLEND_H__

#include <pandabase.h>
#include <typedReferenceCount.h>

class LerpBlendType : public TypedReferenceCount {
public:
  LerpBlendType(void) {}
  LerpBlendType(const LerpBlendType&);
  virtual ~LerpBlendType(void);
  LerpBlendType& operator=(const LerpBlendType&);
  virtual float operator()(float) = 0;
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LerpBlendType",
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

class EaseInBlendType : public LerpBlendType {
public:
  EaseInBlendType(void) {}
  EaseInBlendType(const EaseInBlendType&);
  virtual ~EaseInBlendType(void);
  EaseInBlendType& operator=(const EaseInBlendType&);
  virtual float operator()(float);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseInBlendType",
		  LerpBlendType::get_class_type());
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

class EaseOutBlendType : public LerpBlendType {
public:
  EaseOutBlendType(void) {}
  EaseOutBlendType(const EaseOutBlendType&);
  virtual ~EaseOutBlendType(void);
  EaseOutBlendType& operator=(const EaseOutBlendType&);
  virtual float operator()(float);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseOutBlendType",
		  LerpBlendType::get_class_type());
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

class EaseInOutBlendType : public LerpBlendType {
public:
  EaseInOutBlendType(void) {}
  EaseInOutBlendType(const EaseInOutBlendType&);
  virtual ~EaseInOutBlendType(void);
  EaseInOutBlendType& operator=(const EaseInOutBlendType&);
  virtual float operator()(float);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseInOutBlendType",
		  LerpBlendType::get_class_type());
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

class NoBlendType : public LerpBlendType {
public:
  NoBlendType(void) {}
  NoBlendType(const NoBlendType&);
  virtual ~NoBlendType(void);
  NoBlendType& operator=(const NoBlendType&);
  virtual float operator()(float);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NoBlendType",
		  LerpBlendType::get_class_type());
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

#endif /* __LERPBLEND_H__ */
