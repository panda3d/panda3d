// Filename: parameter.cxx
// Created by:  drose (08Feb99)
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

#include "parameter.h"
#include "dcast.h"

TypeHandle Parameter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Parameter::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Parameter::
output(ostream &out) const {
  switch (do_get_type()) {
  case T_pointer:
    {
      TypedObject *obj = get_typed_object();

      if (obj == (TypedObject *)NULL) {
        out << "(empty)";

      } else if (obj->is_of_type(ParamValueBase::get_class_type())) {
        ((const ParamValueBase *)obj)->output(out);

      } else {
        out << obj->get_type();
      }
    }
    break;

  case T_int:
    out << _v._packed._int;
    break;

  case T_bool:
    out << (_v._packed._bool ? "true" : "false");
    break;

  case T_float:
    out << _v._packed._float << "f";
    break;

  default:
    out << do_get_double();
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of ints matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
extract_data(int *data, int width, size_t count) const {
  nassertr(count > 0 && width > 0, false);

  switch (do_get_type()) {
  case T_pointer:
    {
      TypedObject *obj = get_typed_object();
      if (obj != (TypedObject *)NULL && obj->is_of_type(ParamValueBase::get_class_type())) {
        return ((const ParamValueBase *)obj)->extract_data(data, width, count);
      }
    }
    return false;

  case T_int:
    data[0] = _v._packed._int;
    return (width == 1 && count == 1);

  case T_bool:
    data[0] = (int) _v._packed._bool;
    return (width == 1 && count == 1);

  default:
    return false;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of floats matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
extract_data(float *data, int width, size_t count) const {
  nassertr(count > 0 && width > 0, false);

  switch (do_get_type()) {
  case T_pointer:
    {
      TypedObject *obj = get_typed_object();
      if (obj != (TypedObject *)NULL && obj->is_of_type(ParamValueBase::get_class_type())) {
        return ((const ParamValueBase *)obj)->extract_data(data, width, count);
      }
    }
    return false;

  case T_int:
    data[0] = (float) _v._packed._int;
    return (width == 1 && count == 1);

  case T_bool:
    data[0] = (float) (int) _v._packed._bool;
    return (width == 1 && count == 1);

  case T_float:
    data[0] = _v._packed._float;
    return (width == 1 && count == 1);

  default:
    data[0] = (float) do_get_double();
    return (width == 1 && count == 1);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of doubles matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
extract_data(double *data, int width, size_t count) const {
  nassertr(count > 0 && width > 0, false);

  switch (do_get_type()) {
  case T_pointer:
    {
      TypedObject *obj = get_typed_object();
      if (obj != (TypedObject *)NULL && obj->is_of_type(ParamValueBase::get_class_type())) {
        return ((const ParamValueBase *)obj)->extract_data(data, width, count);
      }
    }
    return false;

  case T_int:
    data[0] = (double) _v._packed._int;
    return (width == 1 && count == 1);

  case T_bool:
    data[0] = (double) (int) _v._packed._bool;
    return (width == 1 && count == 1);

  case T_float:
    data[0] = (double) _v._packed._float;
    return (width == 1 && count == 1);

  default:
    data[0] = do_get_double();
    return (width == 1 && count == 1);
  }

  return false;
}
