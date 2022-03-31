/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lerpblend.h
 * @author frang
 * @date 2000-05-30
 */

#ifndef __LERPBLEND_H__
#define __LERPBLEND_H__

#include "directbase.h"
#include "typedReferenceCount.h"

class EXPCL_DIRECT_INTERVAL LerpBlendType : public TypedReferenceCount {
PUBLISHED:
  LerpBlendType() {}
  virtual ~LerpBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat) = 0;

public:
  LerpBlendType(const LerpBlendType&);
  LerpBlendType& operator=(const LerpBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LerpBlendType",
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

class EXPCL_DIRECT_INTERVAL EaseInBlendType : public LerpBlendType {
PUBLISHED:
  EaseInBlendType() {}
  virtual ~EaseInBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);

public:
  EaseInBlendType(const EaseInBlendType&);
  EaseInBlendType& operator=(const EaseInBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseInBlendType",
                  LerpBlendType::get_class_type());
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

class EXPCL_DIRECT_INTERVAL EaseOutBlendType : public LerpBlendType {
PUBLISHED:
  EaseOutBlendType() {}
  virtual ~EaseOutBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);

public:
  EaseOutBlendType(const EaseOutBlendType&);
  EaseOutBlendType& operator=(const EaseOutBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseOutBlendType",
                  LerpBlendType::get_class_type());
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

class EXPCL_DIRECT_INTERVAL EaseInOutBlendType : public LerpBlendType {
PUBLISHED:
  EaseInOutBlendType() {}
  virtual ~EaseInOutBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);
public:
  EaseInOutBlendType(const EaseInOutBlendType&);
  EaseInOutBlendType& operator=(const EaseInOutBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EaseInOutBlendType",
                  LerpBlendType::get_class_type());
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

class EXPCL_DIRECT_INTERVAL NoBlendType : public LerpBlendType {
PUBLISHED:
  NoBlendType() {}
  virtual ~NoBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);
public:
  NoBlendType(const NoBlendType&);
  NoBlendType& operator=(const NoBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NoBlendType",
                  LerpBlendType::get_class_type());
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

#endif /* __LERPBLEND_H__ */
