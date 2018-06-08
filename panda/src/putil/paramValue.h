/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramValue.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef PARAMVALUE_H
#define PARAMVALUE_H

#include "pandabase.h"

#include "typedef.h"
#include "typedObject.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "luse.h"

/**
 * A non-template base class of ParamValue (below), which serves mainly to
 * define the placeholder for the virtual output function.
 */
class EXPCL_PANDA_PUTIL ParamValueBase : public TypedWritableReferenceCount {
public:
  INLINE ParamValueBase();

PUBLISHED:
  virtual ~ParamValueBase();
  INLINE virtual TypeHandle get_value_type() const;
  virtual void output(std::ostream &out) const=0;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ParamValueBase",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

/**
 * A class object for storing specifically objects of type
 * TypedReferenceCount, which is different than TypedWritableReferenceCount.
 */
class EXPCL_PANDA_PUTIL ParamTypedRefCount : public ParamValueBase {
PUBLISHED:
  INLINE ParamTypedRefCount(const TypedReferenceCount *value);
  virtual ~ParamTypedRefCount();

  INLINE virtual TypeHandle get_value_type() const;
  INLINE TypedReferenceCount *get_value() const;

  MAKE_PROPERTY(value, get_value);

  virtual void output(std::ostream &out) const;

private:
  PT(TypedReferenceCount) _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "ParamTypedRefCount",
                  ParamValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

/**
 * A handy class object for storing simple values (like integers or strings)
 * passed along with an Event or to be used as a shader input.  This is
 * essentially just a wrapper around whatever data type you like, to make it a
 * TypedWritableReferenceCount object which can be passed along inside an
 * EventParameter or ShaderInput.
 */
template<class Type>
class ParamValue : public ParamValueBase {
protected:
  INLINE ParamValue();

PUBLISHED:
  INLINE ParamValue(const Type &value);
  INLINE virtual ~ParamValue();

  INLINE virtual TypeHandle get_value_type() const;
  INLINE void set_value(const Type &value);
  INLINE const Type &get_value() const;

  MAKE_PROPERTY(value, get_value, set_value);

  INLINE virtual void output(std::ostream &out) const;

private:
  Type _value;

public:
  INLINE static void register_with_read_factory();
  INLINE virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  INLINE static TypedWritable *make_from_bam(const FactoryParams &params);
  INLINE void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type(const std::string &type_name = "UndefinedParamValue") {
    ParamValueBase::init_type();
    _type_handle = register_dynamic_type
      (type_name, ParamValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    // In this case, we can't do anything, since we don't have the class'
    // type_name.
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "paramValue.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<std::string>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<std::wstring>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase2d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase2f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase2i>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase3d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase3f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase3i>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase4d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase4f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LVecBase4i>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LMatrix3d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LMatrix3f>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LMatrix4d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ParamValue<LMatrix4f>);

typedef ParamValue<std::string> ParamString;
typedef ParamValue<std::wstring> ParamWstring;

typedef ParamValue<LVecBase2d> ParamVecBase2d;
typedef ParamValue<LVecBase2f> ParamVecBase2f;
typedef ParamValue<LVecBase2i> ParamVecBase2i;

typedef ParamValue<LVecBase3d> ParamVecBase3d;
typedef ParamValue<LVecBase3f> ParamVecBase3f;
typedef ParamValue<LVecBase3i> ParamVecBase3i;

typedef ParamValue<LVecBase4d> ParamVecBase4d;
typedef ParamValue<LVecBase4f> ParamVecBase4f;
typedef ParamValue<LVecBase4i> ParamVecBase4i;

typedef ParamValue<LMatrix3d> ParamMatrix3d;
typedef ParamValue<LMatrix3f> ParamMatrix3f;

typedef ParamValue<LMatrix4d> ParamMatrix4d;
typedef ParamValue<LMatrix4f> ParamMatrix4f;

#ifdef STDFLOAT_DOUBLE
typedef ParamVecBase2d ParamVecBase2;
typedef ParamVecBase3d ParamVecBase3;
typedef ParamVecBase4d ParamVecBase4;

typedef ParamMatrix3d ParamMatrix3;
typedef ParamMatrix4d ParamMatrix4;
#else
typedef ParamVecBase2f ParamVecBase2;
typedef ParamVecBase3f ParamVecBase3;
typedef ParamVecBase4f ParamVecBase4;

typedef ParamMatrix3f ParamMatrix3;
typedef ParamMatrix4f ParamMatrix4;
#endif

#endif
