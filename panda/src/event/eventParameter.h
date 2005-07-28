// Filename: eventParameter.h
// Created by:  drose (08Feb99)
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

#ifndef EVENTPARAMETER_H
#define EVENTPARAMETER_H

#include "pandabase.h"

#include "typedef.h"
#include "typedObject.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "bamReader.h"
#include "bamWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : EventParameter
// Description : An optional parameter associated with an event.  Each
//               event may have zero or more of these.  Each parameter
//               stores a pointer to a TypedWritableReferenceCount
//               object, which of course could be pretty much
//               anything.  To store a simple value like a double or a
//               string, the EventParameter constructors transparently
//               use the EventStoreValue template class, defined
//               below.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventParameter {
PUBLISHED:
  INLINE EventParameter();
  INLINE EventParameter(const TypedWritableReferenceCount *ptr);
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

  INLINE TypedWritableReferenceCount *get_ptr() const;

  void output(ostream &out) const;

private:
  PT(TypedWritableReferenceCount) _ptr;
};

INLINE ostream &operator << (ostream &out, const EventParameter &param);

////////////////////////////////////////////////////////////////////
//       Class : EventStoreValueBase
// Description : A non-template base class of EventStoreValue (below),
//               which serves mainly to define the placeholder for the
//               virtual output function.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventStoreValueBase : public TypedWritableReferenceCount {
public:
  INLINE EventStoreValueBase();
  INLINE ~EventStoreValueBase();
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
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "EventStoreValueBase",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : EventStoreValue
// Description : A handy class object for storing simple values (like
//               integers or strings) passed along with an Event.
//               This is essentially just a wrapper around whatever
//               data type you like, to make it a
//               TypedWritableReferenceCount object which can be
//               passed along inside an EventParameter.
////////////////////////////////////////////////////////////////////
template<class Type>
class EventStoreValue : public EventStoreValueBase {
private:
  INLINE EventStoreValue();
public:
  INLINE EventStoreValue(const Type &value);
  virtual ~EventStoreValue();

  INLINE void set_value(const Type &value);
  INLINE const Type &get_value() const;

  virtual void output(ostream &out) const;

  Type _value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<int>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<double>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<std::string>);

typedef EventStoreValue<int> EventStoreInt;
typedef EventStoreValue<double> EventStoreDouble;
typedef EventStoreValue<string> EventStoreString;

#include "eventParameter.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif

