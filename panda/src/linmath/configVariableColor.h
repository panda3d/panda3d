// Filename: configVariableColor.h
// Created by:  rdb (02Feb14)
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

#ifndef CONFIGVARIABLECOLOR_H
#define CONFIGVARIABLECOLOR_H

#include "dtoolbase.h"
#include "config_linmath.h"
#include "config_prc.h"
#include "configVariable.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableColor
// Description : This is a convenience class to specialize
//               ConfigVariable as a set of floating-point types
//               representing a color value.
//
//               It interprets the color differently depending on
//               how many words were specified: if only one, it
//               is interpreted as a shade of gray with alpha 1.
//               If two values were specified, a grayscale and
//               alpha pair.  If three, a set of R, G, B values
//               with alpha 1, and if four, a complete RGBA color.
//
//               This isn't defined in dtool because it relies on
//               the LColor class, which is defined in linmath.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_LINMATH ConfigVariableColor : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableColor(const string &name);
  INLINE ConfigVariableColor(const string &name, const LColor &default_value,
                             const string &description = string(),
                             int flags = 0);
  INLINE ConfigVariableColor(const string &name, const string &default_value,
                             const string &description = string(),
                             int flags = 0);

  INLINE void operator = (const LColor &value);
  INLINE operator const LColor & () const;

  INLINE PN_stdfloat operator [] (int n) const;

  INLINE void set_value(const LColor &value);
  INLINE const LColor &get_value() const;
  INLINE LColor get_default_value() const;

private:
  void set_default_value(const LColor &default_value);

private:
  mutable AtomicAdjust::Integer _local_modified;
  mutable LColor _cache;
};

#include "configVariableColor.I"

#endif
