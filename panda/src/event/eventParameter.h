// Filename: eventParameter.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EVENTPARAMETER_H
#define EVENTPARAMETER_H

#include <pandabase.h>

#include <typedef.h>
#include <typeHandle.h>
#include <typedReferenceCount.h>
#include <pointerTo.h>
//#include <luse.h>
#include <string>

////////////////////////////////////////////////////////////////////
// 	 Class : EventParameter
// Description : An optional parameter associated with an event.  Each
//               event may have zero or more of these.  Each parameter
//               stores a pointer to a TypedReferenceCount object,
//               which of course could be pretty much anything.  To
//               store a simple value like a double or a string, the
//               EventParameter constructors transparently use the
//               EventStoreValue template class, defined below.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS EventParameter {
public:
  INLINE EventParameter(const TypedReferenceCount *ptr);
  INLINE EventParameter(int value);
  INLINE EventParameter(double value);
  INLINE EventParameter(const string &value);
//  INLINE EventParameter(const LVecBase3f &value);

  INLINE EventParameter(const EventParameter &copy);
  INLINE EventParameter &operator = (const EventParameter &copy);

  // These functions are conveniences to easily determine if the
  // EventParameter is one of the predefined parameter types, and
  // retrieve the corresponding value.  Of course, it is possible that
  // the EventParameter is some user-defined type, and is none of
  // these.
  INLINE bool is_int() const;
  INLINE int get_int_value() const;
  INLINE bool is_double() const;
  INLINE double get_double_value() const;
  INLINE bool is_string() const;
  INLINE string get_string_value() const;
  INLINE bool is_vec3() const;
//  INLINE LVecBase3f get_vec3_value() const;

  INLINE const TypedReferenceCount *get_ptr() const;

private:
  CPT(TypedReferenceCount) _ptr;
};


////////////////////////////////////////////////////////////////////
// 	 Class : EventStoreValue
// Description : A handy class object for storing simple values (like
//               integers or strings) passed along with an Event.
//               This is essentially just a wrapper around whatever
//               data type you like, to make it a TypedReferenceCount
//               object which can be passed along inside an
//               EventParameter.
////////////////////////////////////////////////////////////////////
template<class Type>
class EventStoreValue : public TypedReferenceCount {
public:
  EventStoreValue(const Type &value) : _value(value) { }

  INLINE void set_value(const Type &value);
  INLINE Type get_value() const;

  Type _value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type(const string &type_name) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, type_name,
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    // In this case, we can't do anything, since we don't have the
    // class' type_name.
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, EventStoreValue<int>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, EventStoreValue<double>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, EventStoreValue<std::string>);
//EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, EventStoreValue<LVecBase3f>);

typedef EventStoreValue<int> EventStoreInt;
typedef EventStoreValue<double> EventStoreDouble;
typedef EventStoreValue<string> EventStoreString;
//typedef EventStoreValue<LVecBase3f> EventStoreVec3;

#include "eventParameter.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif

