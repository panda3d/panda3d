/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventParameter.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef EVENTPARAMETER_H
#define EVENTPARAMETER_H

#include "pandabase.h"

#include "typedef.h"
#include "typedObject.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "paramValue.h"

/**
 * An optional parameter associated with an event.  Each event may have zero
 * or more of these.  Each parameter stores a pointer to a
 * TypedWritableReferenceCount object, which of course could be pretty much
 * anything.  To store a simple value like a double or a string, the
 * EventParameter constructors transparently use the ParamValue template class
 * from paramValue.h.
 */
class EXPCL_PANDA_EVENT EventParameter {
PUBLISHED:
  INLINE EventParameter() = default;
  INLINE EventParameter(std::nullptr_t) {};
  INLINE EventParameter(const TypedWritableReferenceCount *ptr);
  INLINE EventParameter(const TypedReferenceCount *ptr);
  INLINE EventParameter(int value);
  INLINE EventParameter(double value);
  INLINE EventParameter(const std::string &value);
  INLINE EventParameter(const std::wstring &value);

  INLINE EventParameter(const EventParameter &copy);
  INLINE EventParameter &operator = (const EventParameter &copy);
  INLINE ~EventParameter();

  // These functions are conveniences to easily determine if the
  // EventParameter is one of the predefined parameter types, and retrieve the
  // corresponding value.  Of course, it is possible that the EventParameter
  // is some user-defined type, and is none of these.
  INLINE bool is_empty() const;
  INLINE bool is_int() const;
  INLINE int get_int_value() const;
  INLINE bool is_double() const;
  INLINE double get_double_value() const;
  INLINE bool is_string() const;
  INLINE std::string get_string_value() const;
  INLINE bool is_wstring() const;
  INLINE std::wstring get_wstring_value() const;

  INLINE bool is_typed_ref_count() const;
  INLINE TypedReferenceCount *get_typed_ref_count_value() const;

  INLINE TypedWritableReferenceCount *get_ptr() const;

  void output(std::ostream &out) const;

private:
  PT(TypedWritableReferenceCount) _ptr;
};

INLINE std::ostream &operator << (std::ostream &out, const EventParameter &param);

typedef ParamTypedRefCount EventStoreTypedRefCount;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, ParamValue<int>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, ParamValue<double>);

typedef ParamValue<int> EventStoreInt;
typedef ParamValue<double> EventStoreDouble;
typedef ParamString EventStoreString;
typedef ParamWstring EventStoreWstring;

#include "eventParameter.I"

#endif
