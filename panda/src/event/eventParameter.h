// Filename: eventParameter.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EVENTPARAMETER_H
#define EVENTPARAMETER_H

#include "pandabase.h"

#include "typedef.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : EventParameter
// Description : An optional parameter associated with an event.  Each
//               event may have zero or more of these.  Each parameter
//               stores a pointer to a TypedReferenceCount object,
//               which of course could be pretty much anything.  To
//               store a simple value like a double or a string, the
//               EventParameter constructors transparently use the
//               EventStoreValue template class, defined below.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS EventParameter {
PUBLISHED:
  INLINE EventParameter();
  INLINE EventParameter(const TypedReferenceCount *ptr);
  INLINE EventParameter(int value);
  INLINE EventParameter(double value);
  INLINE EventParameter(const string &value);

  INLINE EventParameter(const EventParameter &copy);
  INLINE EventParameter &operator = (const EventParameter &copy);

  // These functions are conveniences to easily determine if the
  // EventParameter is one of the predefined parameter types, and
  // retrieve the corresponding value.  Of course, it is possible that
  // the EventParameter is some user-defined type, and is none of
  // these.
  INLINE bool is_empty() const;
  INLINE bool is_int() const;
  INLINE int get_int_value() const;
  INLINE bool is_double() const;
  INLINE double get_double_value() const;
  INLINE bool is_string() const;
  INLINE string get_string_value() const;

  INLINE TypedReferenceCount *get_ptr() const;

  void output(ostream &out) const;

private:
  PT(TypedReferenceCount) _ptr;
};

INLINE ostream &operator << (ostream &out, const EventParameter &param);

////////////////////////////////////////////////////////////////////
//       Class : EventStoreValueBase
// Description : A non-template base class of EventStoreValue (below),
//               which serves mainly to define the placeholder for the
//               virtual output function.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS EventStoreValueBase : public TypedReferenceCount {
public:
  virtual void output(ostream &out) const=0;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EventStoreValueBase",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : EventStoreValue
// Description : A handy class object for storing simple values (like
//               integers or strings) passed along with an Event.
//               This is essentially just a wrapper around whatever
//               data type you like, to make it a TypedReferenceCount
//               object which can be passed along inside an
//               EventParameter.
////////////////////////////////////////////////////////////////////
template<class Type>
class EventStoreValue : public EventStoreValueBase {
public:
  EventStoreValue(const Type &value) : _value(value) { }

  INLINE void set_value(const Type &value);
  INLINE const Type &get_value() const;

  virtual void output(ostream &out) const;

  Type _value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type(const string &type_name = "UndefinedEventStoreValue") {
    EventStoreValueBase::init_type();
    _type_handle = register_dynamic_type
      (type_name, EventStoreValueBase::get_class_type());
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

typedef EventStoreValue<int> EventStoreInt;
typedef EventStoreValue<double> EventStoreDouble;
typedef EventStoreValue<string> EventStoreString;

#include "eventParameter.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif

