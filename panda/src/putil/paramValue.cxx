// Filename: paramValue.cxx
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

#include "paramValue.h"
#include "dcast.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle ParamValueBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParamValueBase::
~ParamValueBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of integers matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ParamValueBase::
extract_data(int *data, int width, size_t count) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of floats matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ParamValueBase::
extract_data(float *data, int width, size_t count) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of doubles matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ParamValueBase::
extract_data(double *data, int width, size_t count) const {
  return false;
}

// Now follow specializations of extract_data for the various types.
#ifndef CPPPARSER
template<>
bool ParamValue<string>::
extract_data(int *data, int width, size_t count) const {
  if (width != 1) {
    return false;
  }

  size_t maxnum = min(_value.size(), count);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = _value[i];
  }
  return (count <= _value.size());
}

template<>
bool ParamValue<wstring>::
extract_data(int *data, int width, size_t count) const {
  if (width != 1) {
    return false;
  }

  size_t maxnum = min(_value.size(), count);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = _value[i];
  }
  return (count <= _value.size());
}

template<>
bool ParamValue<LVecBase2i>::
extract_data(int *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 2) * sizeof(int));
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase3i>::
extract_data(int *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 3) * sizeof(int));
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase4i>::
extract_data(int *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 4) * sizeof(int));
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase2d>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 2);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase2f>::
extract_data(float *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 2) * sizeof(float));
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase2i>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 2);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase3d>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 3);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase3f>::
extract_data(float *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 3) * sizeof(float));
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase3i>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 3);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase4d>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 4);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase4f>::
extract_data(float *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 4) * sizeof(float));
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase4i>::
extract_data(float *data, int width, size_t count) const {
  size_t maxnum = min(width, 4);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (float)_value[i];
  }
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase2d>::
extract_data(double *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 2) * sizeof(double));
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase2f>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 2);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase2i>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 2);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 2);
}

template<>
bool ParamValue<LVecBase3d>::
extract_data(double *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 3) * sizeof(double));
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase3f>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 3);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase3i>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 3);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 3);
}

template<>
bool ParamValue<LVecBase4d>::
extract_data(double *data, int width, size_t count) const {
  memcpy((void *)data, (const void *)_value.get_data(), min(width, 4) * sizeof(double));
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase4f>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 4);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 4);
}

template<>
bool ParamValue<LVecBase4i>::
extract_data(double *data, int width, size_t count) const {
  size_t maxnum = min(width, 4);
  for (int i = 0; i < maxnum; ++i) {
    data[i] = (double)_value[i];
  }
  return (count == 1) && (width <= 4);
}
#endif  // CPPPARSER
